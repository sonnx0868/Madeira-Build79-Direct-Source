import Foundation

enum ReadingProgressPolicy {
    /// An explicit per-game choice wins over automatic detection, including
    /// profiles created before this feature existed (their preference is nil).
    static func enabled(preference: Bool?, supported: Bool) -> Bool {
        supported && preference != false
    }
}

/// The engine's cumulative dialogue coverage, not route/gallery completion.
struct ReadingProgressSnapshot: Codable, Equatable {
    let schema: Int
    let session: String
    let seen: Int
    let total: Int

    static func decode(_ data: Data, session: String) -> Self? {
        guard data.count <= 1024, let value = try? JSONDecoder().decode(Self.self, from: data),
              value.schema == 1, value.session == session,
              value.seen >= 0, value.total >= 0,
              value.seen <= 100_000_000, value.total <= 100_000_000 else { return nil }
        return value
    }

    // Scripts updated since an old save can have a different denominator.
    // Do not silently clamp an invalid count to a fabricated 100% completion.
    var fraction: Double? {
        guard total > 0, seen <= total else { return nil }
        return Double(seen) / Double(total)
    }
    var percentage: Int? { fraction.map { Int(($0 * 100).rounded(.down)) } }
}

struct ReadingProgressPosition: Codable, Equatable {
    var x: Double
    var y: Double
    static let initial = Self(x: 0.90, y: 0.30)
    var bounded: Self {
        Self(x: x.isFinite ? min(1, max(0, x)) : Self.initial.x,
             y: y.isFinite ? min(1, max(0, y)) : Self.initial.y)
    }
}

enum ReadingProgressFiles {
    static let script = "zz_madeira_reading_progress_v1.rpy"
    static let control = ".madeira-reading-session-v1.json"
    static let output = ".madeira-reading-progress-v1.json"
    static let marker = Data("# Madeira reading progress bridge v1 - managed by Madeira.".utf8)

    enum Failure: LocalizedError {
        case conflictingFile, unsafePath, missingBridge
        var errorDescription: String? {
            switch self {
            case .conflictingFile: return "Reading progress could not be enabled because its helper filename is already in use."
            case .unsafePath: return "Reading progress requires a game folder inside Madeira."
            case .missingBridge: return "The reading progress helper is missing from this build."
            }
        }
    }

    /// Bounded directory checks only, including EXEs inside lib/windows-*.
    static func gameFolder(location: URL, drive: URL) -> URL? {
        let fm = FileManager.default
        let base = drive.resolvingSymlinksInPath().standardizedFileURL.path + "/"
        var isDir: ObjCBool = false
        let directory = fm.fileExists(atPath: location.path, isDirectory: &isDir) && isDir.boolValue
        var root = directory ? location : location.deletingLastPathComponent()
        for _ in 0..<5 {
            root = root.resolvingSymlinksInPath().standardizedFileURL
            guard root.path.hasPrefix(base) else { return nil }
            let game = root.appendingPathComponent("game", isDirectory: true).resolvingSymlinksInPath()
            var gameIsDir: ObjCBool = false, engineIsDir: ObjCBool = false
            if game.path.hasPrefix(base),
               fm.fileExists(atPath: game.path, isDirectory: &gameIsDir), gameIsDir.boolValue,
               fm.fileExists(atPath: root.appendingPathComponent("renpy").path, isDirectory: &engineIsDir), engineIsDir.boolValue {
                return game
            }
            root.deleteLastPathComponent()
        }
        return nil
    }

    static func boundedRead(_ file: URL, limit: Int = 1024) -> Data? {
        guard let handle = try? FileHandle(forReadingFrom: file) else { return nil }
        defer { try? handle.close() }
        guard let data = try? handle.read(upToCount: limit + 1), data.count <= limit else { return nil }
        return data
    }

    private static func safeFile(_ name: String, folder: URL) throws -> URL {
        let file = folder.appendingPathComponent(name)
        if let attributes = try? FileManager.default.attributesOfItem(atPath: file.path),
           attributes[.type] as? FileAttributeType == .typeSymbolicLink { throw Failure.unsafePath }
        // Foundation URL equality includes directory hints/trailing slashes.
        // Compare canonical filesystem paths, as callers may omit that hint.
        guard file.resolvingSymlinksInPath().standardizedFileURL.deletingLastPathComponent().path == folder.resolvingSymlinksInPath().standardizedFileURL.path else {
            throw Failure.unsafePath
        }
        return file
    }

    /// Only our marked helper may be replaced/removed; saves and archives are
    /// untouched. Disabled profiles remove our compiled helper as well.
    static func prepare(folder: URL, enabled: Bool, bridge: Data?, session: String) throws {
        let fm = FileManager.default
        let source = try safeFile(script, folder: folder)
        let compiled = try safeFile(script + "c", folder: folder)
        let config = try safeFile(control, folder: folder)
        let result = try safeFile(output, folder: folder)
        let exists = fm.fileExists(atPath: source.path)
        let owned = boundedRead(source, limit: 32 * 1024)?.starts(with: marker) == true
        if !enabled {
            if owned {
                for file in [compiled, config, result, source] where fm.fileExists(atPath: file.path) { try fm.removeItem(at: file) }
            }
            return
        }
        guard (!exists || owned) && (owned || !fm.fileExists(atPath: compiled.path)),
              owned || (!fm.fileExists(atPath: config.path) && !fm.fileExists(atPath: result.path)) else { throw Failure.conflictingFile }
        guard let bridge, bridge.starts(with: marker), bridge.count <= 32 * 1024 else { throw Failure.missingBridge }
        if boundedRead(source, limit: 32 * 1024) != bridge {
            try bridge.write(to: source, options: .atomic)
            if fm.fileExists(atPath: compiled.path) { try fm.removeItem(at: compiled) }
        }
        try setEnabled(true, folder: folder, session: session)
    }

    static func setEnabled(_ enabled: Bool, folder: URL, session: String) throws {
        let file = try safeFile(control, folder: folder)
        let data = try JSONSerialization.data(withJSONObject: ["schema": 1, "session": session, "enabled": enabled])
        try data.write(to: file, options: .atomic)
    }
}
