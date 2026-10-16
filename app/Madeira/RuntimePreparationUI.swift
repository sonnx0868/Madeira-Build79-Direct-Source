import Foundation
import Combine
import SwiftUI

@MainActor
final class GameComponentPreparation: ObservableObject {
    static let shared = GameComponentPreparation()
    struct Status {
        var busy: Bool
        var text: String
        var warning: Bool = false
    }
    @Published private(set) var status: [UUID: Status] = [:]
    @Published private(set) var reports: [UUID: RuntimeReport] = [:]
    private var tasks: [UUID: Task<RuntimeReport, Error>] = [:]

    func enqueue(_ entry: LibraryEntry) {
        Task { _ = try? await prepare(entry) }
    }

    func prepare(_ entry: LibraryEntry, force: Bool = false, repairContent: Bool = false) async throws -> RuntimeReport {
        if let task = tasks[entry.id] { return try await task.value }
        guard entry.desktop != true, entry.temporarySession != true else { return RuntimeReport() }
        guard let bundle = Bundle.main.resourceURL else { throw RuntimePreparationError.message("The application resources are unavailable.") }
        let drive = LibraryModel.drive
        let cache = LibraryModel.documents.appendingPathComponent("RuntimeCache", isDirectory: true)
        status[entry.id] = Status(busy: true, text: "Checking required components…")
        let task = Task { @MainActor [weak self] in
            if !repairContent, let appID = entry.steamAppID,
               SteamOwnedLibrary.shared.updateAvailable(appID: appID, installedBuild: SteamGamesModel.shared.builds[appID]) {
                throw RuntimePreparationError.message("Steam has a newer build of this game. Open Game details › Check & repair to update and verify its files before playing.")
            }
            if repairContent, let appID = entry.steamAppID {
                try await SteamOwnedLibrary.shared.verifyInstalledGame(appID) { [weak self] message in
                    self?.update(entry.id, text: message)
                }
            }
            self?.update(entry.id, text: "Finding the game's program…")
            guard let executable = try await Self.executable(entry, drive: drive) else {
                if repairContent { throw RuntimePreparationError.message("Files were checked, but Steam has multiple launch programs. Choose one in Game details › Steam › Program, then check again.") }
                var report = RuntimeReport(); report.selectionPending = true
                return report
            }
            let inspected = try LibraryModel.inspect(executable)
            try Task.checkCancellation()
            return try await RuntimePreparer.shared.prepare(executable: executable, bits: inspected.bits, drive: drive,
                bundle: bundle, cache: cache, force: force, progress: { [weak self] text in
                    await self?.update(entry.id, text: text)
                })
        }
        tasks[entry.id] = task
        defer { tasks[entry.id] = nil }
        do {
            let report = try await task.value
            reports[entry.id] = report
            let summary = repairContent && entry.steamAppID != nil && report.missing.isEmpty && !report.modernDotNet
                ? "Steam files and supported runtimes verified. " + report.summary : report.summary
            status[entry.id] = Status(busy: false, text: summary,
                                     warning: !report.missing.isEmpty || report.modernDotNet || report.truncated || report.selectionPending == true)
            LogStore.shared.log("[runtime-prepare] cache=\(report.cached ? 1 : 0) scanned=\(report.scanned) installed=\(report.installed.count) unresolved=\(report.missing.count)")
            return report
        } catch {
            status[entry.id] = Status(busy: false, text: error.localizedDescription, warning: true)
            LogStore.shared.log("[runtime-prepare] preparation failed; retry available", level: .error)
            throw error
        }
    }

    /// A store profile names its install folder, not necessarily an EXE. Use
    /// the same launch metadata as the existing Steam program picker.
    private static func executable(_ entry: LibraryEntry, drive: URL) async throws -> URL? {
        guard let appID = entry.steamAppID else { return try LibraryModel.executable(entry.launchRelativePath) }
        let root = drive.appendingPathComponent(entry.relativePath).resolvingSymlinksInPath()
        guard RuntimeFiles.inside(root, drive) else { throw RuntimePreparationError.message("The game folder is outside drive_c.") }
        let options = await SteamOwnedLibrary.shared.launchOptions(appID: appID)
        let picked = entry.steamProgram
        let program = await Task.detached(priority: .utility) {
            SteamDirectStart.program(picked: picked, options: options, installFolder: root)
        }.value
        guard let program else { return nil }
        let file = root.appendingPathComponent(program).resolvingSymlinksInPath()
        guard RuntimeFiles.inside(file, root) else { throw RuntimePreparationError.message("The game program is outside its installation folder.") }
        return file
    }
    private func update(_ id: UUID, text: String) {
        guard status[id]?.text != text || status[id]?.busy != true else { return }
        status[id] = Status(busy: true, text: text)
    }
}

struct GameComponentPreparationSection: View {
    let entry: LibraryEntry
    @ObservedObject private var preparation = GameComponentPreparation.shared
    var body: some View {
        Section("Game check & repair") {
            if let status = preparation.status[entry.id] {
                HStack(alignment: .top, spacing: 10) {
                    if status.busy { ProgressView() }
                    else { Image(systemName: status.warning ? "exclamationmark.circle" : "checkmark.circle").foregroundStyle(status.warning ? .orange : .green) }
                    Text(status.text).font(.subheadline)
                }
            } else {
                Text("Required components are checked automatically before starting.").font(.subheadline).foregroundStyle(.secondary)
            }
            Button("Check & repair", systemImage: "wrench.and.screwdriver") {
                Task { _ = try? await preparation.prepare(entry, force: true, repairContent: true) }
            }.disabled(preparation.status[entry.id]?.busy == true || LibraryModel.shared.current != nil || wine_process_is_running() != 0)
            if let report = preparation.reports[entry.id] {
                LabeledContent("Programs and libraries checked", value: String(report.scanned) + (report.truncated ? "+" : ""))
                if report.missing.contains(where: \.wrongArchitecture) {
                    Label("A DLL in the game folder has the wrong architecture. Use the matching game files; existing files were preserved.", systemImage: "exclamationmark.triangle")
                        .font(.caption).foregroundStyle(.orange)
                }
            }
            Text(entry.steamAppID != nil
                ? "Verifies this installation against Steam's current files, downloads missing content, then checks the program and supported runtimes. Close the game before repairing."
                : "Checks this installed program and installs missing supported runtimes from Microsoft. Game-owned files and unsupported components are reported separately.")
                .font(.caption).foregroundStyle(.secondary)
        }
    }
}
