import Foundation

/// Read-only runtime identification. It does not inspect story text, unpack
/// archives, replace game DLLs or claim that a detected engine is supported.
enum GameEngineDetection {
    static func detect(_ executable: URL) -> String? {
        let fm = FileManager.default
        let root = executable.deletingLastPathComponent()
        func names(_ folder: URL) -> [String: URL] {
            let items = (try? fm.contentsOfDirectory(at: folder, includingPropertiesForKeys: nil,
                                                   options: [.skipsHiddenFiles])) ?? []
            return items.reduce(into: [:]) { result, url in
                if result[url.lastPathComponent.lowercased()] == nil { result[url.lastPathComponent.lowercased()] = url }
            }
        }
        let top = names(root)
        if top["unityplayer.dll"] != nil { return "Unity" }
        if top["librenpython.dll"] != nil { return "Ren'Py" }
        if let lib = top["lib"] {
            for (name, folder) in names(lib).sorted(by: { $0.key < $1.key }).prefix(64)
                where name.contains("windows-") {
                if names(folder)["librenpython.dll"] != nil { return "Ren'Py" }
            }
        }
        // Engines packaged as web applications keep these script files in
        // the project root, www, or Electron's unpacked resources/app.
        var roots = [root]
        if let www = top["www"] { roots.append(www) }
        if let resources = top["resources"], let app = names(resources)["app"] { roots.append(app) }
        for project in roots {
            let files = names(project)
            if let tyrano = files["tyrano"], names(tyrano)["tyrano.js"] != nil { return "TyranoScript" }
            if files["index.html"] != nil, let js = files["js"], files["data"] != nil {
                let scripts = names(js)
                if scripts["rmmz_core.js"] != nil { return "RPG Maker MZ" }
                if scripts["rpg_core.js"] != nil { return "RPG Maker MV" }
            }
        }
        if top["game.ini"] != nil,
           top.keys.contains(where: { $0.hasPrefix("rgss") && $0.hasSuffix(".dll") }) {
            return "RPG Maker XP/VX/VX Ace (RGSS)"
        }
        if top["nscript.dat"] != nil || top["0.txt"] != nil {
            if top.keys.contains(where: { $0.hasSuffix(".nsa") || $0.hasSuffix(".ns2") || $0 == "arc.sar" }) {
                return "NScripter/ONScripter"
            }
        }
        for (name, file) in top.sorted(by: { $0.key < $1.key }).prefix(256) {
            if name.hasSuffix(".xp3"), prefix(file, count: 11) == Data([0x58,0x50,0x33,0x0d,0x0a,0x20,0x0a,0x1a,0x8b,0x67,0x01]) {
                return "Kirikiri/KAG"
            }
            if name.hasSuffix(".pck"), prefix(file, count: 4) == Data("GDPC".utf8) { return "Godot" }
        }
        // A bounded PE resource read also detects embedded Godot packs and
        // LiveMaker builds that carry their engine product identity.
        let resource = peResources(executable)
        for (marker, label) in [("LiveMaker", "LiveMaker"), ("Godot Engine", "Godot"), ("Kirikiri", "Kirikiri/KAG")] {
            if resource.range(of: Data(marker.utf8)) != nil ||
                resource.range(of: Data(marker.utf16.flatMap { [UInt8($0 & 255), UInt8($0 >> 8)] })) != nil { return label }
        }
        if top["nw.dll"] != nil, top["package.json"] != nil { return "NW.js" }
        if top["chrome_elf.dll"] != nil, let resources = top["resources"], names(resources)["app.asar"] != nil { return "Electron" }
        return nil
    }

    private static func prefix(_ url: URL, count: Int) -> Data {
        guard let file = try? FileHandle(forReadingFrom: url) else { return Data() }
        defer { try? file.close() }
        return (try? file.read(upToCount: count)) ?? Data()
    }

    private static func peResources(_ url: URL) -> Data {
        guard let file = try? FileHandle(forReadingFrom: url) else { return Data() }
        defer { try? file.close() }
        func read(_ offset: UInt64, _ count: Int) -> Data {
            guard (try? file.seek(toOffset: offset)) != nil else { return Data() }
            return (try? file.read(upToCount: count)) ?? Data()
        }
        func number(_ bytes: Data, _ offset: Int, _ count: Int) -> UInt64 {
            guard offset >= 0, offset + count <= bytes.count else { return 0 }
            return (0..<count).reduce(0) { $0 | UInt64(bytes[offset + $1]) << ($1 * 8) }
        }
        let dos = read(0, 64)
        guard dos.count == 64, dos.prefix(2) == Data("MZ".utf8) else { return Data() }
        let offset = number(dos, 60, 4)
        guard offset >= 64, offset <= 16 * 1024 * 1024 else { return Data() }
        let header = read(offset, 24)
        guard header.count == 24, header.prefix(4) == Data([0x50,0x45,0,0]) else { return Data() }
        let count = Int(number(header, 6, 2)), optional = number(header, 20, 2)
        guard count > 0, count <= 96, optional <= 4096 else { return Data() }
        let sections = read(offset + 24 + optional, count * 40)
        guard sections.count == count * 40 else { return Data() }
        for i in 0..<count {
            let start = i * 40
            if sections.subdata(in: start..<start+5) != Data(".rsrc".utf8) { continue }
            let length = Int(min(number(sections, start + 16, 4), 256 * 1024))
            let raw = number(sections, start + 20, 4)
            guard length > 0, raw > 0 else { return Data() }
            return read(raw, length)
        }
        return Data()
    }
}
