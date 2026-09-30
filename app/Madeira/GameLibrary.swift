import Foundation
import Combine
import UniformTypeIdentifiers

/// The PE machine values that this build can actually execute.
///
/// Madeira is a 64-bit Wine/FEX port.  A file having an `.exe` suffix is not
/// enough to make it runnable: 32-bit x86 PE files still need a WoW64/x86
/// execution path which is not shipped in this target.  Keeping this fact in
/// the model lets the UI explain the failure before a long JIT boot begins.
enum GameArchitecture: String, Codable, CaseIterable, Hashable, Sendable {
    case x86_64
    case arm64ec
    case arm64
    case x86
    case arm32
    case unknown

    var title: String {
        switch self {
        case .x86_64: return "Windows 64-bit"
        case .arm64ec: return "Windows ARM64EC"
        case .arm64: return "Windows ARM64"
        case .x86: return "Windows 32-bit"
        case .arm32: return "Windows ARM32"
        case .unknown: return "Không xác định"
        }
    }

    var shortTitle: String {
        switch self {
        case .x86_64: return "x64"
        case .arm64ec: return "ARM64EC"
        case .arm64: return "ARM64"
        case .x86: return "x86"
        case .arm32: return "ARM32"
        case .unknown: return "?"
        }
    }

    var isSupported: Bool { self == .x86_64 || self == .arm64ec || self == .arm64 }
}

enum GameKind: String, Codable, Hashable, Sendable {
    case renpy
    case generic

    var title: String {
        switch self {
        case .renpy: return "Ren’Py visual novel"
        case .generic: return "Windows game / app"
        }
    }

    var iconName: String {
        switch self {
        case .renpy: return "book.closed.fill"
        case .generic: return "gamecontroller.fill"
        }
    }
}

struct GameExecutableOption: Identifiable, Codable, Hashable, Sendable {
    var id: String { path }
    let path: String
    let architecture: GameArchitecture
}

/// Immutable launch data passed from the library to the Wine bootstrap.
/// Keeping this separate from SwiftUI state prevents a second tap from
/// changing process-global environment variables while the first game starts.
struct GameLaunchConfiguration: Hashable, Sendable {
    let profileID: UUID
    let displayName: String
    let executablePath: String
    let arguments: String
    let useARM64EC: Bool
    let desktopMode: Bool
    let architecture: GameArchitecture
    let kind: GameKind
}

struct GameProfile: Identifiable, Codable, Hashable, Sendable {
    let id: UUID
    var name: String
    /// Relative to `Documents/wine/drive_c`, never an absolute user path.
    var installDirectory: String
    /// Relative to `installDirectory`, using `/` separators.
    var executable: String
    var kind: GameKind
    var architecture: GameArchitecture
    var arguments: String
    var desktopMode: Bool
    var createdAt: Date
    var lastPlayedAt: Date?
    var executableOptions: [GameExecutableOption]?
    /// Non-nil when the user imported only an executable. Optional keeps older
    /// manifests backwards-compatible.
    var importWarning: String?

    var executableWindowsPath: String {
        let rel = sanitizedRelativePath(executable)
            .split(separator: "/", omittingEmptySubsequences: true)
            .map(String.init)
            .joined(separator: "\\")
        return "C:\\Games\\\(id.uuidString)\\\(rel)"
    }

    var isSupported: Bool { architecture.isSupported }

    var compatibilityText: String {
        if architecture == .x86 {
            return "32-bit x86 chưa được hỗ trợ trong bản iOS này"
        }
        if architecture == .arm32 {
            return "ARM32 chưa được hỗ trợ trong bản iOS này"
        }
        if architecture == .unknown {
            return "Không đọc được kiến trúc PE — có thể là launcher đặc biệt"
        }
        if kind == .renpy {
            return arguments.contains("angle2")
                ? "Ren’Py 64-bit · ANGLE2 → D3D11/Metal (thử nghiệm)"
                : "Ren’Py 64-bit · software renderer"
        }
        if let importWarning { return "\(architecture.title) · \(importWarning)" }
        return architecture.title
    }

    private func sanitizedRelativePath(_ value: String) -> String {
        value.split { $0 == "/" || $0 == "\\" }
            .filter { !$0.isEmpty && $0 != "." && $0 != ".." }
            .map(String.init)
            .joined(separator: "/")
    }

    func launchConfiguration() -> GameLaunchConfiguration {
        GameLaunchConfiguration(
            profileID: id,
            displayName: name,
            executablePath: executableWindowsPath,
            arguments: arguments,
            useARM64EC: architecture == .x86_64 || architecture == .arm64ec,
            desktopMode: desktopMode,
            architecture: architecture,
            kind: kind
        )
    }
}

enum GameImportState: Equatable {
    case idle
    case importing
    case imported(String)
    case failed(String)
}

enum GameImportError: LocalizedError {
    case unsupportedItem
    case noExecutable
    case invalidExecutable
    case sourceOverlapsLibrary
    case libraryUnavailable(String)
    case insufficientSpace(required: Int64, available: Int64)
    case copyFailed(String)

    var errorDescription: String? {
        switch self {
        case .unsupportedItem:
            return "Hãy chọn một thư mục game hoặc file .exe."
        case .noExecutable:
            return "Thư mục không chứa file .exe để khởi chạy."
        case .invalidExecutable:
            return "File được chọn không phải PE executable hợp lệ."
        case .sourceOverlapsLibrary:
            return "Không thể nhập thư mục Documents/Wine của chính Madeira. Hãy chọn thư mục game gốc ở vị trí khác."
        case .libraryUnavailable(let message):
            return "Manifest thư viện đang lỗi nên Madeira sẽ không ghi đè dữ liệu cũ: \(message)"
        case .insufficientSpace(let required, let available):
            let formatter = ByteCountFormatter()
            formatter.countStyle = .file
            return "Không đủ dung lượng: game cần khoảng \(formatter.string(fromByteCount: required)), nhưng thiết bị chỉ còn \(formatter.string(fromByteCount: available))."
        case .copyFailed(let message):
            return "Không thể chép game vào bộ nhớ Madeira: \(message)"
        }
    }
}

/// Imports game folders/files into a private, stable Wine drive and persists a
/// small manifest.  The importer deliberately copies instead of retaining a
/// security-scoped URL: Files providers can revoke that URL after the picker
/// closes, while a copied game continues to work after reinstall/relocation.
@MainActor
final class GameLibraryStore: ObservableObject {
    static let shared = GameLibraryStore()

    static let windowsExecutableType =
        UTType(filenameExtension: "exe") ?? UTType.data

    @Published private(set) var games: [GameProfile] = []
    @Published private(set) var importState: GameImportState = .idle

    private let fileManager = FileManager.default
    private let metadataURL: URL
    private let gamesURL: URL
    private var persistenceFailureMessage: String?

    init(fileManager: FileManager = .default) {
        self.fileManager = fileManager
        let documents = fileManager.urls(for: .documentDirectory,
                                        in: .userDomainMask)[0]
        metadataURL = documents.appendingPathComponent("madeira-games.json")
        gamesURL = documents
            .appendingPathComponent("wine", isDirectory: true)
            .appendingPathComponent("drive_c", isDirectory: true)
            .appendingPathComponent("Games", isDirectory: true)
        load()
    }

    func importItem(from source: URL) async {
        guard importState != .importing else { return }
        if let persistenceFailureMessage {
            importState = .failed(GameImportError.libraryUnavailable(
                persistenceFailureMessage
            ).localizedDescription)
            return
        }
        importState = .importing

        let hasAccess = source.startAccessingSecurityScopedResource()
        defer {
            if hasAccess { source.stopAccessingSecurityScopedResource() }
        }

        do {
            let destination = gamesURL
            let result = try await Task.detached(priority: .userInitiated) {
                try Self.copyAndInspect(source: source, into: destination)
            }.value
            games.removeAll { $0.id == result.profile.id }
            games.insert(result.profile, at: 0)
            do {
                try save()
            } catch {
                games.removeAll { $0.id == result.profile.id }
                let orphan = gamesURL.appendingPathComponent(result.profile.id.uuidString,
                                                              isDirectory: true)
                try? await Task.detached(priority: .utility) {
                    try FileManager.default.removeItem(at: orphan)
                }.value
                throw error
            }
            importState = .imported(result.profile.name)
        } catch {
            importState = .failed((error as? LocalizedError)?.errorDescription
                                  ?? error.localizedDescription)
        }
    }

    func resetImportState() {
        importState = .idle
    }

    func markLaunched(_ profile: GameProfile) {
        guard let index = games.firstIndex(where: { $0.id == profile.id }) else { return }
        let previous = games[index]
        games[index].lastPlayedAt = Date()
        do { try save() }
        catch {
            games[index] = previous
            LogStore.shared.log("Không lưu được thời gian chạy game: \(error.localizedDescription)",
                                level: .error)
        }
    }

    func update(_ profile: GameProfile) {
        guard let index = games.firstIndex(where: { $0.id == profile.id }) else { return }
        let previous = games[index]
        games[index] = profile
        do { try save() }
        catch {
            games[index] = previous
            importState = .failed("Không lưu được cấu hình game: \(error.localizedDescription)")
        }
    }

    /// Removes only the per-game directory below the managed Games root.
    func remove(_ profile: GameProfile) async throws {
        let target = gamesURL.appendingPathComponent(profile.id.uuidString,
                                                     isDirectory: true)
        let root = gamesURL.standardizedFileURL.path
        let targetPath = target.standardizedFileURL.path
        guard targetPath == root || targetPath.hasPrefix(root + "/") else {
            throw GameImportError.copyFailed("đường dẫn game không hợp lệ")
        }
        guard let removedIndex = games.firstIndex(where: { $0.id == profile.id }) else { return }
        games.removeAll { $0.id == profile.id }
        do { try save() }
        catch {
            games.insert(profile, at: min(removedIndex, games.count))
            throw error
        }

        do {
            if fileManager.fileExists(atPath: targetPath) {
                try await Task.detached(priority: .utility) {
                    try FileManager.default.removeItem(at: target)
                }.value
            }
        } catch {
            if !games.contains(where: { $0.id == profile.id }) {
                games.insert(profile, at: min(removedIndex, games.count))
            }
            try? save()
            throw error
        }
    }

    func isInstalled(_ profile: GameProfile) -> Bool {
        let root = gamesURL.appendingPathComponent(profile.id.uuidString,
                                                    isDirectory: true).standardizedFileURL
        let target = root.appendingPathComponent(profile.executable).standardizedFileURL
        guard target.path.hasPrefix(root.path + "/") else { return false }
        return fileManager.fileExists(atPath: target.path)
    }

    private func load() {
        guard fileManager.fileExists(atPath: metadataURL.path) else { return }
        do {
            let data = try Data(contentsOf: metadataURL)
            games = try JSONDecoder().decode([GameProfile].self, from: data)
        } catch {
            persistenceFailureMessage = error.localizedDescription
            LogStore.shared.log("Không đọc được thư viện game; giữ nguyên manifest để phục hồi: \(error.localizedDescription)",
                                level: .error)
        }
    }

    private func save() throws {
        if let persistenceFailureMessage {
            throw GameImportError.libraryUnavailable(persistenceFailureMessage)
        }
        let data = try JSONEncoder.pretty.encode(games)
        try fileManager.createDirectory(at: metadataURL.deletingLastPathComponent(),
                                        withIntermediateDirectories: true)
        try data.write(to: metadataURL, options: .atomic)
    }

    private struct ImportResult: Sendable {
        let profile: GameProfile
    }

    private nonisolated static func copyAndInspect(source: URL, into gamesRoot: URL) throws -> ImportResult {
        let fm = FileManager.default
        var destinationForCleanup: URL?
        defer {
            if let destinationForCleanup {
                try? fm.removeItem(at: destinationForCleanup)
            }
        }
        do {
            let sourcePath = source.standardizedFileURL.resolvingSymlinksInPath().path
            let libraryPath = gamesRoot.standardizedFileURL.resolvingSymlinksInPath().path
            let sourcePrefix = sourcePath.hasSuffix("/") ? sourcePath : sourcePath + "/"
            if libraryPath == sourcePath || libraryPath.hasPrefix(sourcePrefix) {
                throw GameImportError.sourceOverlapsLibrary
            }
            try fm.createDirectory(at: gamesRoot, withIntermediateDirectories: true)
            let values = try source.resourceValues(forKeys: [.isDirectoryKey])
            guard values.isDirectory != nil else { throw GameImportError.unsupportedItem }
            try rejectSymlinks(in: source, fileManager: fm)
            let requiredBytes = try estimatedSize(of: source, fileManager: fm)
            if let available = try gamesRoot.resourceValues(
                forKeys: [.volumeAvailableCapacityForImportantUsageKey]
            ).volumeAvailableCapacityForImportantUsage {
                let reserve: Int64 = 256 * 1024 * 1024
                if available < requiredBytes + reserve {
                    throw GameImportError.insufficientSpace(required: requiredBytes + reserve,
                                                            available: available)
                }
            }

            let id = UUID()
            let destination = gamesRoot.appendingPathComponent(id.uuidString,
                                                               isDirectory: true)
            destinationForCleanup = destination
            try fm.createDirectory(at: destination, withIntermediateDirectories: true)

            if values.isDirectory == true {
                let children = try fm.contentsOfDirectory(at: source,
                                                           includingPropertiesForKeys: nil,
                                                           options: [])
                for child in children {
                    let target = destination.appendingPathComponent(child.lastPathComponent)
                    try fm.copyItem(at: child, to: target)
                }
            } else if source.pathExtension.lowercased() == "exe" {
                try fm.copyItem(at: source,
                                to: destination.appendingPathComponent(source.lastPathComponent))
            } else {
                throw GameImportError.unsupportedItem
            }

            let executables = try findExecutables(in: destination)
            let preferredName = values.isDirectory == true
                ? source.lastPathComponent
                : source.deletingPathExtension().lastPathComponent
            guard let executable = chooseExecutable(executables,
                                                    preferredName: preferredName)
            else {
                try? fm.removeItem(at: destination)
                throw GameImportError.noExecutable
            }

            let relative = relativePath(of: executable, to: destination)
            let architecture = inspectArchitecture(executable)
            if values.isDirectory == false && architecture == .unknown {
                try? fm.removeItem(at: destination)
                throw GameImportError.invalidExecutable
            }
            let kind = detectKind(in: destination, executable: executable)
            let rendererArgs = recommendedArguments(kind: kind, root: destination)
            let displayName = values.isDirectory == true
                ? source.lastPathComponent
                : source.deletingPathExtension().lastPathComponent
            let profile = GameProfile(
                id: id,
                name: displayName.isEmpty ? executable.deletingPathExtension().lastPathComponent : displayName,
                installDirectory: "Games/\(id.uuidString)",
                executable: relative,
                kind: kind,
                architecture: architecture,
                arguments: rendererArgs,
                desktopMode: false,
                createdAt: Date(),
                lastPlayedAt: nil,
                executableOptions: executables.map {
                    GameExecutableOption(path: relativePath(of: $0, to: destination),
                                         architecture: inspectArchitecture($0))
                },
                importWarning: values.isDirectory == true
                    ? nil
                    : "Chỉ chép .exe — nếu thiếu DLL/data hãy thêm lại thư mục game"
            )
            destinationForCleanup = nil
            return ImportResult(profile: profile)
        } catch let error as GameImportError {
            throw error
        } catch {
            throw GameImportError.copyFailed(error.localizedDescription)
        }
    }

    private nonisolated static func estimatedSize(of root: URL,
                                                  fileManager: FileManager) throws -> Int64 {
        let rootValues = try root.resourceValues(forKeys: [.isDirectoryKey, .fileSizeKey])
        if rootValues.isDirectory != true {
            return Int64(rootValues.fileSize ?? 0)
        }
        guard let enumerator = fileManager.enumerator(
            at: root,
            includingPropertiesForKeys: [.isRegularFileKey, .fileSizeKey],
            options: [.skipsPackageDescendants]) else { return 0 }
        var total: Int64 = 0
        for case let item as URL in enumerator {
            let values = try item.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
            if values.isRegularFile == true {
                total += Int64(values.fileSize ?? 0)
            }
        }
        return total
    }

    private nonisolated static func rejectSymlinks(in root: URL,
                                                   fileManager: FileManager) throws {
        let rootValues = try root.resourceValues(forKeys: [.isSymbolicLinkKey])
        if rootValues.isSymbolicLink == true {
            throw GameImportError.copyFailed("symlink không được phép trong game import")
        }
        guard let enumerator = fileManager.enumerator(
            at: root,
            includingPropertiesForKeys: [.isSymbolicLinkKey],
            options: []) else { return }
        for case let item as URL in enumerator {
            let values = try item.resourceValues(forKeys: [.isSymbolicLinkKey])
            if values.isSymbolicLink == true {
                throw GameImportError.copyFailed("game chứa symlink ngoài thư mục — hãy chép thư mục thật")
            }
        }
    }

    private nonisolated static func findExecutables(in root: URL) throws -> [URL] {
        guard let enumerator = FileManager.default.enumerator(
            at: root,
            includingPropertiesForKeys: [.isRegularFileKey, .isDirectoryKey],
            options: [.skipsPackageDescendants])
        else { return [] }

        var result: [URL] = []
        for case let url as URL in enumerator {
            guard url.pathExtension.lowercased() == "exe" else { continue }
            if (try? url.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile) == true {
                result.append(url)
            }
        }
        return result.sorted { $0.path.localizedCaseInsensitiveCompare($1.path) == .orderedAscending }
    }

    private nonisolated static func chooseExecutable(_ candidates: [URL], preferredName: String) -> URL? {
        guard !candidates.isEmpty else { return nil }
        let supported = candidates.filter { inspectArchitecture($0).isSupported }
        // A runnable x64/ARM64 PE always outranks a same-name 32-bit launcher.
        // Only fall back to unsupported candidates so the UI can explain why a
        // folder cannot run, rather than reporting that it contained no EXE.
        let selectionPool = supported.isEmpty ? candidates : supported
        let preferred = preferredName.lowercased()
        func score(_ url: URL) -> Int {
            let name = url.deletingPathExtension().lastPathComponent.lowercased()
            let path = url.path.lowercased()
            var score = 0
            let architecture = inspectArchitecture(url)
            if architecture.isSupported { score += 700 }
            else if architecture == .x86 || architecture == .arm32 { score -= 2_000 }
            else { score -= 300 }
            if name == preferred { score += 1000 }
            if path.contains("/renpy/") || path.contains("/lib/") { score -= 80 }
            for bad in ["unins", "uninstall", "setup", "install", "crash", "launcher", "updater", "redist"] {
                if name.contains(bad) { score -= 400 }
            }
            score -= url.pathComponents.count
            return score
        }
        return selectionPool.max { score($0) < score($1) }
    }

    private nonisolated static func relativePath(of file: URL, to root: URL) -> String {
        let prefix = root.standardizedFileURL.path + "/"
        return file.standardizedFileURL.path
            .replacingOccurrences(of: prefix, with: "")
            .replacingOccurrences(of: "\\", with: "/")
    }

    private nonisolated static func inspectArchitecture(_ url: URL) -> GameArchitecture {
        guard let handle = try? FileHandle(forReadingFrom: url) else { return .unknown }
        defer { try? handle.close() }
        let dos: Data
        do { dos = try handle.read(upToCount: 0x40) ?? Data() }
        catch { return .unknown }
        guard dos.count >= 0x40, dos[0] == 0x4d, dos[1] == 0x5a else { return .unknown }
        let peOffset = Int(readUInt32(dos, at: 0x3c))
        guard peOffset >= 0 else { return .unknown }
        let pe: Data
        do {
            try handle.seek(toOffset: UInt64(peOffset))
            pe = try handle.read(upToCount: 6) ?? Data()
        } catch { return .unknown }
        guard pe.count >= 6, pe[0] == 0x50, pe[1] == 0x45,
              pe[2] == 0, pe[3] == 0 else { return .unknown }
        switch readUInt16(pe, at: 4) {
        case 0x8664: return .x86_64
        case 0xa641: return .arm64ec
        case 0xaa64: return .arm64
        case 0x014c: return .x86
        case 0x01c4: return .arm32
        default: return .unknown
        }
    }

    private nonisolated static func readUInt16(_ data: Data, at offset: Int) -> UInt16 {
        guard offset >= 0, offset + 1 < data.count else { return 0 }
        return UInt16(data[offset]) | (UInt16(data[offset + 1]) << 8)
    }

    private nonisolated static func readUInt32(_ data: Data, at offset: Int) -> UInt32 {
        guard offset >= 0, offset + 3 < data.count else { return 0 }
        return UInt32(data[offset])
            | (UInt32(data[offset + 1]) << 8)
            | (UInt32(data[offset + 2]) << 16)
            | (UInt32(data[offset + 3]) << 24)
    }

    private nonisolated static func detectKind(in root: URL, executable: URL) -> GameKind {
        let fm = FileManager.default
        let topLevel = (try? fm.contentsOfDirectory(at: root,
                                                     includingPropertiesForKeys: nil,
                                                     options: [])) ?? []
        var names = Set<String>()
        for url in topLevel { names.insert(url.lastPathComponent.lowercased()) }
        if names.contains("renpy") {
            return .renpy
        }
        if names.contains("game") || names.contains("lib") {
            var hasRenPyFiles = false
            if let enumerator = fm.enumerator(at: root,
                                              includingPropertiesForKeys: nil,
                                              options: [.skipsHiddenFiles]) {
                for case let item as URL in enumerator {
                    let path = item.path.lowercased()
                    let name = item.lastPathComponent.lowercased()
                    if path.contains("/renpy/") || path.hasSuffix("/renpy") ||
                       path.hasSuffix(".rpy") || path.hasSuffix(".rpyc") ||
                       path.contains("/lib/py3-") ||
                       (name.hasPrefix("python") && name.hasSuffix(".dll")) {
                        hasRenPyFiles = true
                        break
                    }
                }
            }
            if hasRenPyFiles { return .renpy }
        }
        let exeName = executable.lastPathComponent.lowercased()
        if exeName.contains("renpy") { return .renpy }
        return .generic
    }

    private nonisolated static func recommendedArguments(kind: GameKind, root: URL) -> String {
        guard kind == .renpy else { return "" }
        var hasEGL = false
        var hasGLES = false
        if let files = FileManager.default.enumerator(at: root,
                                                      includingPropertiesForKeys: nil,
                                                      options: [.skipsHiddenFiles]) {
            for case let item as URL in files {
                let name = item.lastPathComponent.lowercased()
                if name == "libegl.dll" { hasEGL = true }
                if name == "libglesv2.dll" { hasGLES = true }
                if hasEGL && hasGLES { break }
            }
        }
        // ANGLE can feed D3D11 into DXMT.  If a distribution does not ship
        // ANGLE, Ren’Py's software renderer is slower but avoids a guaranteed
        // missing-DLL/OpenGL failure and is appropriate for VN workloads.
        // OpenGL is intentionally a stub in the current iOS Wine build. A
        // complete bundled ANGLE pair can instead feed D3D11 into DXMT; older
        // distributions without it fall back to Ren’Py's software renderer.
        return hasEGL && hasGLES ? "--renderer angle2" : "--renderer sw"
    }
}

private extension JSONEncoder {
    static var pretty: JSONEncoder {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        return encoder
    }
}
