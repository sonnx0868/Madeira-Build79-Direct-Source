import Foundation

/// Guards the identified GameMaker Steam extension's nullable stats read.
/// The existing false-result path is retained; no achievements are unlocked.
enum GameMakerCompatibility {
    static func statsGuard(_ original: Data) -> Data? {
        let bytes = Data(original)
        func number(_ offset: Int, _ count: Int = 4) -> UInt32 {
            guard offset >= 0, offset + count <= bytes.count else { return 0 }
            return (0..<count).reduce(0) { $0 | UInt32(bytes[offset + $1]) << ($1 * 8) }
        }
        guard bytes.count >= 256, bytes.count <= 8 * 1024 * 1024,
              bytes.prefix(2) == Data("MZ".utf8) else { return nil }
        let pe = Int(number(60)), count = Int(number(pe + 6, 2)), optional = pe + 24
        guard pe >= 64, pe < bytes.count - 256, number(pe) == 0x4550,
              number(pe + 4, 2) == 0x8664, (1...96).contains(count),
              number(optional, 2) == 0x20b, number(optional + 108) >= 4 else { return nil }
        let sections = optional + Int(number(pe + 20, 2))
        guard sections + count * 40 <= bytes.count else { return nil }
        func fileOffset(_ rva: UInt32, _ length: Int) -> Int? {
            for index in 0..<count {
                let item = sections + index * 40, start = number(item + 12), size = number(item + 16), raw = number(item + 20)
                if rva >= start, UInt64(rva - start) + UInt64(length) <= UInt64(size) {
                    let value = UInt64(raw) + UInt64(rva - start)
                    if value + UInt64(length) <= UInt64(bytes.count) { return Int(value) }
                }
            }
            return nil
        }
        guard let exports = fileOffset(number(optional + 112), 40) else { return nil }
        let names = Int(number(exports + 24)), functions = Int(number(exports + 20))
        guard (1...4096).contains(names), (1...4096).contains(functions),
              let namesAt = fileOffset(number(exports + 32), names * 4),
              let ordinals = fileOffset(number(exports + 36), names * 2),
              let addresses = fileOffset(number(exports + 28), functions * 4) else { return nil }
        var getter: UInt32?
        for index in 0..<names {
            guard let at = fileOffset(number(namesAt + index * 4), 1),
                  let end = bytes[at..<min(at + 256, bytes.count)].firstIndex(of: 0),
                  let name = String(data: bytes[at..<end], encoding: .ascii),
                  name.hasPrefix("?steam_get_achievement@@") else { continue }
            let ordinal = Int(number(ordinals + index * 2, 2))
            guard ordinal < functions else { return nil }
            getter = number(addresses + ordinal * 4); break
        }
        guard let getter, getter <= UInt32.max - 176, let at = fileOffset(getter, 176) else { return nil }
        // Exact getter, its existing false-result path and unused alignment
        // padding. An unknown extension is left intact.
        let expected: [UInt8] = [0x48,0x8b,0x01,0xff,0x50,0x30,0x3c,0x01,0x75,0x11]
        let branch: [UInt8] = [0xeb,0x38,0x90]
        let stub: [UInt8] = [0xe3,0xdf,0x48,0x8b,0x01,0xeb,0xc2]
        if Array(bytes[(at + 109)..<(at + 112)]) == branch,
           Array(bytes[(at + 167)..<(at + 174)]) == stub { return nil }
        guard Array(bytes[(at + 109)..<(at + 119)]) == expected,
              Array(bytes[(at + 136)..<(at + 139)]) == [0x0f,0x57,0xc0],
              bytes[(at + 167)..<(at + 176)].allSatisfy({ $0 == 0xcc }) else { return nil }
        let tableRVA = number(optional + 112 + 3 * 8), tableSize = Int(number(optional + 116 + 3 * 8))
        guard tableSize >= 12, tableSize <= 1024 * 1024, tableSize % 12 == 0,
              let table = fileOffset(tableRVA, tableSize) else { return nil }
        var unwindEntry: Int?
        for item in stride(from: table, to: table + tableSize, by: 12) {
            if number(item) == getter && number(item + 4) == getter + 167 { unwindEntry = item; break }
        }
        guard let unwindEntry else { return nil }
        var patched = bytes
        patched.replaceSubrange((at + 109)..<(at + 112), with: branch)
        patched.replaceSubrange((at + 167)..<(at + 174), with: stub)
        let end = getter + 174
        for i in 0..<4 { patched[unwindEntry + 4 + i] = UInt8(truncatingIfNeeded: end >> (i * 8)) }
        return patched
    }

    static func prepare(_ executable: URL, drive: URL) -> Bool {
        guard executable.resolvingSymlinksInPath().path.hasPrefix(drive.resolvingSymlinksInPath().path + "/"),
              let files = try? FileManager.default.contentsOfDirectory(at: executable.deletingLastPathComponent(), includingPropertiesForKeys: nil),
              let file = files.first(where: { $0.lastPathComponent.lowercased() == "steamworks_x64.dll" }),
              file.resolvingSymlinksInPath().path.hasPrefix(drive.resolvingSymlinksInPath().path + "/"),
              let size = try? file.resourceValues(forKeys: [.fileSizeKey]).fileSize, size <= 8 * 1024 * 1024,
              let original = try? Data(contentsOf: file), let patched = statsGuard(original) else { return false }
        let backup = file.appendingPathExtension("madeira-stats-original")
        do {
            if FileManager.default.fileExists(atPath: backup.path) {
                guard try Data(contentsOf: backup) == original else { return false }
            } else { try original.write(to: backup, options: .withoutOverwriting) }
            try patched.write(to: file, options: .atomic)
            return true
        } catch { return false }
    }
}
