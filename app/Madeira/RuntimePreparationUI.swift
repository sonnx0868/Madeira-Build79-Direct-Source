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
    private var tasks: [UUID: Task<RuntimeReport, Error>] = [:]

    func enqueue(_ entry: LibraryEntry) {
        Task { _ = try? await prepare(entry) }
    }

    func prepare(_ entry: LibraryEntry, force: Bool = false) async throws -> RuntimeReport {
        if let task = tasks[entry.id] { return try await task.value }
        guard entry.desktop != true, entry.temporarySession != true else { return RuntimeReport() }
        let executable = try LibraryModel.executable(entry.launchRelativePath)
        guard let bundle = Bundle.main.resourceURL else { throw RuntimePreparationError.message("The application resources are unavailable.") }
        let drive = LibraryModel.drive
        let cache = LibraryModel.documents.appendingPathComponent("RuntimeCache", isDirectory: true)
        status[entry.id] = Status(busy: true, text: "Checking required components…")
        let task = Task {
            try await RuntimePreparer.shared.prepare(executable: executable, bits: entry.bits, drive: drive,
                bundle: bundle, cache: cache, force: force, progress: { [weak self] text in
                    await self?.update(entry.id, text: text)
                })
        }
        tasks[entry.id] = task
        defer { tasks[entry.id] = nil }
        do {
            let report = try await task.value
            status[entry.id] = Status(busy: false, text: report.summary,
                                     warning: !report.missing.isEmpty || report.modernDotNet || report.truncated)
            LogStore.shared.log("[runtime-prepare] cache=\(report.cached ? 1 : 0) scanned=\(report.scanned) installed=\(report.installed.count) unresolved=\(report.missing.count)")
            return report
        } catch {
            status[entry.id] = Status(busy: false, text: error.localizedDescription, warning: true)
            LogStore.shared.log("[runtime-prepare] preparation failed; retry available", level: .error)
            throw error
        }
    }
    private func update(_ id: UUID, text: String) { status[id] = Status(busy: true, text: text) }
}

struct GameComponentPreparationSection: View {
    let entry: LibraryEntry
    @ObservedObject private var preparation = GameComponentPreparation.shared
    var body: some View {
        Section("Automatic components") {
            if let status = preparation.status[entry.id] {
                HStack(alignment: .top, spacing: 10) {
                    if status.busy { ProgressView() }
                    else { Image(systemName: status.warning ? "exclamationmark.circle" : "checkmark.circle").foregroundStyle(status.warning ? .orange : .green) }
                    Text(status.text).font(.subheadline)
                }
            } else {
                Text("Required components are checked automatically before starting.").font(.subheadline).foregroundStyle(.secondary)
            }
            Button("Check components again", systemImage: "arrow.clockwise") {
                Task { _ = try? await preparation.prepare(entry, force: true) }
            }.disabled(preparation.status[entry.id]?.busy == true || LibraryModel.shared.current != nil)
            Text("Missing supported runtimes come from Microsoft. Downloads and installed files are verified, then cached for the next launch. Game files must come from your game installation.")
                .font(.caption).foregroundStyle(.secondary)
        }
    }
}
