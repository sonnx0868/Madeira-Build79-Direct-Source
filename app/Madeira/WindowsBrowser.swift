import Foundation

/// Launch policy for a complete Windows Chrome installation. This identifies
/// the package, not browser compatibility: only device execution proves that.
enum WindowsBrowser {
    struct Installation: Equatable {
        let binaryDirectory: URL
        let machine: UInt16
    }

    private static func machine(_ url: URL) -> UInt16? {
        guard let file = try? FileHandle(forReadingFrom: url) else { return nil }
        defer { try? file.close() }
        guard let dos = try? file.read(upToCount: 64), dos.count == 64,
              dos[0] == 0x4d, dos[1] == 0x5a else { return nil }
        let offset = (0..<4).reduce(UInt64(0)) { $0 | UInt64(dos[60 + $1]) << ($1 * 8) }
        guard (64..<16_777_216).contains(offset), (try? file.seek(toOffset: offset)) != nil,
              let pe = try? file.read(upToCount: 6), pe.count == 6,
              Array(pe.prefix(4)) == [0x50, 0x45, 0, 0] else { return nil }
        return UInt16(pe[4]) | UInt16(pe[5]) << 8
    }

    static func inspect(_ executable: URL) -> Installation? {
        guard executable.lastPathComponent.lowercased() == "chrome.exe",
              let arch = machine(executable), [UInt16(0x8664), 0x14c].contains(arch) else { return nil }
        let base = executable.deletingLastPathComponent().resolvingSymlinksInPath()
        // Chrome for Testing has a flat layout; installed/portable Chrome puts
        // its DLL and resources in a version directory. Do not scan profiles.
        let children = (try? FileManager.default.contentsOfDirectory(at: base,
            includingPropertiesForKeys: nil, options: .skipsHiddenFiles)) ?? []
        let versions = children.filter { item in
            let parts = item.lastPathComponent.split(separator: ".", omittingEmptySubsequences: false)
            return parts.count == 4 && parts.allSatisfy { !$0.isEmpty && $0.allSatisfy(\.isNumber) }
        }.sorted { $0.lastPathComponent.compare($1.lastPathComponent, options: .numeric) == .orderedDescending }
        for folder in [base] + Array(versions.prefix(32)) {
            let resolved = folder.resolvingSymlinksInPath()
            let dll = resolved.appendingPathComponent("chrome.dll").resolvingSymlinksInPath()
            let elf = resolved.appendingPathComponent("chrome_elf.dll").resolvingSymlinksInPath()
            guard resolved.path == base.path || resolved.path.hasPrefix(base.path + "/"),
                  dll.path.hasPrefix(base.path + "/"), machine(dll) == arch,
                  elf.path.hasPrefix(base.path + "/"), machine(elf) == arch,
                  ["resources.pak", "icudtl.dat", "v8_context_snapshot.bin"].allSatisfy({
                      let file = resolved.appendingPathComponent($0)
                      guard file.resolvingSymlinksInPath().path.hasPrefix(base.path + "/"),
                            let size = try? file.resourceValues(forKeys: [.fileSizeKey]).fileSize else { return false }
                      return size > 0
                  }), ["Locales", "locales"].contains(where: { name in
                      let locales = resolved.appendingPathComponent(name).resolvingSymlinksInPath()
                      guard locales.path.hasPrefix(base.path + "/"),
                            let items = try? FileManager.default.contentsOfDirectory(atPath: locales.path) else { return false }
                      return items.contains { $0.lowercased().hasSuffix(".pak") }
                  }) else { continue }
            return Installation(binaryDirectory: resolved, machine: arch)
        }
        return nil
    }

    static func profileRelativePath(_ id: UUID) -> String {
        "Madeira/Browsers/" + id.uuidString.lowercased()
    }

    static func prepare(executable: URL, drive: URL, id: UUID) throws {
        let root = drive.resolvingSymlinksInPath().standardizedFileURL
        guard executable.resolvingSymlinksInPath().path.hasPrefix(root.path + "/"),
              inspect(executable) != nil else {
            throw NSError(domain: "Madeira.Chrome", code: 1, userInfo: [NSLocalizedDescriptionKey:
                "Chrome is incomplete. Copy its entire Windows folder, including chrome.dll, resources and Locales, into drive_c; select chrome.exe, not its installer."])
        }
        let folder = root.appendingPathComponent(profileRelativePath(id)).appendingPathComponent("Profile").resolvingSymlinksInPath()
        guard folder.path.hasPrefix(root.path + "/") else {
            throw NSError(domain: "Madeira.Chrome", code: 2, userInfo: [NSLocalizedDescriptionKey:
                "The Chrome profile folder points outside drive_c."])
        }
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        // Never copy a PC's cookies, edit Preferences, remove a singleton lock,
        // disable the sandbox or start a debugging listener during preparation.
    }

    static func quote(_ text: String) -> String {
        var output = "\"", slashes = 0
        for character in text {
            if character == "\\" { slashes += 1; continue }
            if character == "\"" { output += String(repeating: "\\", count: slashes * 2 + 1) + "\"" }
            else { output += String(repeating: "\\", count: slashes) + String(character) }
            slashes = 0
        }
        return output + String(repeating: "\\", count: slashes * 2) + "\""
    }

    /// Read switch names with Windows backslash/quote rules. Preserve the
    /// user's original text when adding defaults, including URLs and Unicode.
    static func tokens(_ text: String) -> [String] {
        let chars = Array(text)
        var result: [String] = [], i = 0
        while i < chars.count {
            while i < chars.count && (chars[i] == " " || chars[i] == "\t") { i += 1 }
            if i == chars.count { break }
            var token = "", quoted = false
            while i < chars.count {
                if !quoted && (chars[i] == " " || chars[i] == "\t") { break }
                var slashes = 0
                while i < chars.count && chars[i] == "\\" { slashes += 1; i += 1 }
                if i < chars.count && chars[i] == "\"" {
                    token += String(repeating: "\\", count: slashes / 2)
                    if slashes % 2 == 1 { token.append("\"") }
                    else if quoted && i + 1 < chars.count && chars[i + 1] == "\"" {
                        token.append("\""); i += 1
                    } else { quoted.toggle() }
                    i += 1
                } else {
                    token += String(repeating: "\\", count: slashes)
                    if i < chars.count && !quoted && (chars[i] == " " || chars[i] == "\t") { break }
                    if i < chars.count { token.append(chars[i]); i += 1 }
                }
            }
            result.append(token)
        }
        return result
    }

    static func arguments(_ original: String, id: UUID, resolution: String,
                          software: Bool, disableSandbox: Bool) -> String {
        let switches = Set(tokens(original).map { String($0.split(separator: "=", maxSplits: 1).first ?? "").lowercased() })
        var added: [String] = []
        func add(_ name: String, _ value: String? = nil) {
            if !switches.contains(name) { added.append(value.map { name + "=" + quote($0) } ?? name) }
        }
        add("--user-data-dir", "C:\\" + profileRelativePath(id).replacingOccurrences(of: "/", with: "\\") + "\\Profile")
        add("--no-first-run"); add("--no-default-browser-check"); add("--disable-background-mode")
        let size = resolution.split(separator: "x", omittingEmptySubsequences: false)
        if size.count == 2, let width = Int(size[0]), let height = Int(size[1]),
           (320...4096).contains(width), (240...4096).contains(height) {
            add("--window-size", "\(width),\(height)")
        }
        if software { add("--disable-gpu") }
        if disableSandbox { add("--no-sandbox") }
        // Keep Chromium's normal multi-process model. --single-process and
        // --in-process-gpu are not browser compatibility fixes.
        return (added + (original.isEmpty ? [] : [original])).joined(separator: " ")
    }
}
