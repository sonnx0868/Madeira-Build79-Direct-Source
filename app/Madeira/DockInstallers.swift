// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 125hz
// Madeira Converter Exception: see LICENSE-EXCEPTION.md

import Foundation

// MARK: - Steam install scripts

/// One "Run Process" entry of a game's Steam install script (installscript.vdf).
/// Valve's desktop client runs the entry's programs (typically runtime
/// redistributables) before a start unless the registry value `name` under `key`
/// is at least `value`, and records it once they have run (Steamworks, "Creating
/// and using InstallScripts").
struct SteamInstallRun: Equatable, Hashable, Sendable, Codable {
    enum Hive: String, Sendable, Codable { case machine, user }
    var name: String
    var hive: Hive
    var key: String      // under the hive, e.g. Software\Valve\Steam\Apps\<appid>
    var value: UInt32    // MinimumHasRunValue, else 1
}

/// One program of a "Run Process" entry, resolved to a Windows path.
struct SteamInstallProcess: Equatable, Sendable {
    var run: SteamInstallRun
    var executable: String      // C:\...
    var arguments: String
}

/// Reading install scripts and recording runs in the prefix's registry files.
/// The .reg files are only written while no session runs: the wineserver keeps
/// the registry in memory and writes it back when it stops.
enum SteamInstallScripts {
    static let maxScriptBytes = 1 << 20

    /// "HKEY_LOCAL_MACHINE\Software\..." -> (.machine, "Software\...").
    static func hive(_ path: String) -> (SteamInstallRun.Hive, String)? {
        let parts = path.replacingOccurrences(of: "/", with: "\\").split(separator: "\\").map(String.init)
        guard let first = parts.first?.uppercased() else { return nil }
        let rest = parts.dropFirst().joined(separator: "\\")
        switch first {
        case "HKEY_LOCAL_MACHINE", "HKLM": return (.machine, rest)
        case "HKEY_CURRENT_USER", "HKCU": return (.user, rest)
        default: return nil
        }
    }

    /// The keys a run is recorded under. A 32-bit reader sees HKLM\Software through
    /// Wow6432Node in a 64-bit prefix; the plain key covers a 64-bit reader.
    static func keys(_ run: SteamInstallRun) -> [String] {
        let lower = run.key.lowercased()
        guard run.hive == .machine, lower.hasPrefix("software\\"), !lower.hasPrefix("software\\wow6432node\\") else { return [run.key] }
        return [run.key, "Software\\Wow6432Node\\" + run.key.dropFirst("software\\".count)]
    }

    /// Install scripts: .vdf files with a "Run Process" section in `folder` and up to
    /// `depth` levels below it. Symbolic links are not followed.
    static func scripts(folder: URL, depth: Int = 1) -> [URL] {
        var found: [URL] = []
        let keys: [URLResourceKey] = [.isDirectoryKey, .isSymbolicLinkKey, .fileSizeKey]
        guard let items = try? FileManager.default.contentsOfDirectory(at: folder, includingPropertiesForKeys: keys, options: [.skipsHiddenFiles]) else { return [] }
        for item in items.sorted(by: { $0.path < $1.path }).prefix(500) {
            guard let values = try? item.resourceValues(forKeys: Set(keys)), values.isSymbolicLink != true else { continue }
            if values.isDirectory == true {
                if depth > 0 { found += scripts(folder: item, depth: depth - 1) }
            } else if item.pathExtension.lowercased() == "vdf", (values.fileSize ?? .max) <= maxScriptBytes,
                      let contents = try? Data(contentsOf: item),
                      String(decoding: contents, as: UTF8.self).range(of: "run process", options: .caseInsensitive) != nil {
                found.append(item)
            }
        }
        return found
    }

    /// Where Valve's client records an entry that names no "HasRunKey": a DWORD named
    /// after the entry (lowercase, as its script parser keys it) under the app's own key.
    static func defaultRunKey(appID: Int) -> String { "Software\\Valve\\Steam\\Apps\\\(appID)" }

    /// The "HasRunKey" of an entry's fields, else (with a valid `appID` and a program to
    /// run) the per-app default key in full-path form.
    static func runKey(_ fields: [String: String], appID: Int?) -> String? {
        if let key = fields["hasrunkey"] { return key }
        guard let appID, appID > 0, appID < 1 << 31, fields.keys.contains(where: { $0.hasPrefix("process ") }) else { return nil }
        return "HKEY_LOCAL_MACHINE\\" + defaultRunKey(appID: appID)
    }

    /// Every "Run Process" entry with its fields (lowercased keys), read straight from the
    /// text. Scripts repeat the "Run Process" key, one section per installer, so a
    /// key-value reader that keeps the last copy of a repeated key would miss entries.
    static func entries(script: Data, appID: Int? = nil) -> [(SteamInstallRun, [String: String])] {
        var bytes = Array(script.prefix(maxScriptBytes))
        if bytes.starts(with: [0xef, 0xbb, 0xbf]) { bytes.removeFirst(3) }
        var position = 0
        func token() -> (text: String, quoted: Bool)? {
            while position < bytes.count {
                if bytes[position] <= 32 { position += 1; continue }
                if bytes[position] == 47, position + 1 < bytes.count, bytes[position + 1] == 47 {
                    while position < bytes.count, bytes[position] != 10 { position += 1 }
                    continue
                }
                break
            }
            guard position < bytes.count else { return nil }
            let first = bytes[position]; position += 1
            if first == 123 { return ("{", false) }
            if first == 125 { return ("}", false) }
            var value: [UInt8] = []
            if first == 34 {
                while position < bytes.count {
                    let byte = bytes[position]; position += 1
                    if byte == 34 { break }
                    if byte == 92, position < bytes.count, bytes[position] == 34 || bytes[position] == 92 { value.append(bytes[position]); position += 1 }
                    else { value.append(byte) }
                }
                return (String(decoding: value, as: UTF8.self), true)
            }
            value.append(first)
            while position < bytes.count, bytes[position] > 32, bytes[position] != 123, bytes[position] != 125 { value.append(bytes[position]); position += 1 }
            return (String(decoding: value, as: UTF8.self), true)
        }
        var result: [(SteamInstallRun, [String: String])] = []
        var path: [String] = []          // keys of the open sections, lowercased
        var pending: String?             // a key waiting for its value or section
        var fields: [String: String] = [:]
        var tokens = 0
        while let (text, quoted) = token() {
            tokens += 1
            if tokens > 200_000 || path.count > 32 { break }
            if !quoted && text == "{" {
                path.append(pending?.lowercased() ?? ""); pending = nil
                if path.count >= 2, path[path.count - 2] == "run process" { fields = [:] }
            } else if !quoted && text == "}" {
                // An entry section closes: path is [..., "run process", <entry>].
                if path.count >= 2, path[path.count - 2] == "run process", let name = path.last,
                   let key = runKey(fields, appID: appID), let (hive, sub) = hive(key), !sub.isEmpty {
                    let minimum = fields["minimumhasrunvalue"].flatMap { UInt32($0.trimmingCharacters(in: .whitespaces)) } ?? 1
                    let run = SteamInstallRun(name: name, hive: hive, key: sub, value: max(1, minimum))
                    if !result.contains(where: { $0.0 == run }) { result.append((run, fields)) }
                }
                if !path.isEmpty { path.removeLast() }
                pending = nil
            } else if let key = pending {
                if path.count >= 2, path[path.count - 2] == "run process" { fields[key.lowercased()] = text }
                pending = nil
            } else {
                pending = text
            }
        }
        return result
    }

    /// The .reg text with every run's value at least its minimum, and how many values changed.
    /// Wine's format: "[Key\\Sub] <time>" section headers, then "\"name\"=dword:00000001" lines.
    static func mark(_ runs: [SteamInstallRun], in text: String, now: Int) -> (text: String, changed: Int) {
        var lines = text.components(separatedBy: "\n")
        var changed = 0
        for run in runs {
            let valueName = "\"" + escape(run.name) + "\"="
            let valueLine = valueName + String(format: "dword:%08x", run.value)
            for key in keys(run) {
                let header = "[" + escape(key) + "]"
                if let start = lines.firstIndex(where: { $0.lowercased().hasPrefix(header.lowercased()) }) {
                    var end = start + 1
                    while end < lines.count, !lines[end].hasPrefix("[") { end += 1 }
                    if let index = (start + 1..<end).first(where: { lines[$0].lowercased().hasPrefix(valueName.lowercased()) }) {
                        if let current = dword(lines[index]), current >= run.value { continue }
                        lines[index] = valueLine
                    } else {
                        var at = end
                        while at > start + 1, lines[at - 1].trimmingCharacters(in: .whitespacesAndNewlines).isEmpty { at -= 1 }
                        lines.insert(valueLine, at: at)
                    }
                } else {
                    while let last = lines.last, last.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty { lines.removeLast() }
                    lines += ["", header + " \(now)", valueLine, ""]
                }
                changed += 1
            }
        }
        return (lines.joined(separator: "\n"), changed)
    }

    /// Records the runs in the prefix's system.reg and user.reg (a .madeira-bak copy is kept
    /// once). Returns the number of values written. Only while no session runs.
    @discardableResult
    static func mark(_ runs: [SteamInstallRun], prefix: URL) throws -> Int {
        var total = 0
        for (hive, file) in [(SteamInstallRun.Hive.machine, "system.reg"), (.user, "user.reg")] {
            let selected = runs.filter { $0.hive == hive }
            guard !selected.isEmpty else { continue }
            let registry = prefix.appendingPathComponent(file)
            guard let contents = try? Data(contentsOf: registry), !contents.isEmpty else { continue }   // a prefix not seeded yet
            let (updated, changed) = mark(selected, in: String(decoding: contents, as: UTF8.self), now: Int(Date().timeIntervalSince1970))
            guard changed > 0 else { continue }
            let backup = registry.appendingPathExtension("madeira-bak")
            if !FileManager.default.fileExists(atPath: backup.path) { try? FileManager.default.copyItem(at: registry, to: backup) }
            try Data(updated.utf8).write(to: registry, options: .atomic)
            total += changed
        }
        return total
    }

    static func escape(_ s: String) -> String { s.replacingOccurrences(of: "\\", with: "\\\\").replacingOccurrences(of: "\"", with: "\\\"") }
    private static func dword(_ line: String) -> UInt32? {
        guard let range = line.range(of: "=dword:", options: .caseInsensitive) else { return nil }
        return UInt32(line[range.upperBound...].trimmingCharacters(in: .whitespacesAndNewlines), radix: 16)
    }
}

// MARK: - Planning, the batch and its results

/// What a Dock start does with one install-script program.
enum DockInstallStatus: String, Sendable {
    case done          // already recorded in the prefix's registry
    case missing       // the program is not on disk
    case unsupported   // this build cannot run it (see DockInstallers.runnable)
    case pending       // runs in the session before the host
    case limit         // over the per-start limit; runs at a later start
}

struct DockInstallPlanItem: Equatable, Sendable {
    var process: SteamInstallProcess
    var status: DockInstallStatus
    var reason: String? = nil     // why an unsupported program cannot run
}

/// The batch's service-manager step: `.start` runs the Dock executable with
/// `--start-services` before the first installer; `.off` only records "services off".
enum DockInstallServices: Equatable, Sendable {
    case off
    case start(String)
}

/// Valve's client runs a game's install scripts as part of its own launch tasks,
/// before the step Madeira Dock asks it for (LaunchApp), so on the Dock route
/// nothing runs them. Madeira does: every program not yet recorded runs once in
/// the Dock session before the host, from a batch that reports each exit status.
/// Every program is treated alike; nothing is decided by program name.
enum DockInstallScripts {
    /// The programs of one install script. `installDir` replaces %INSTALLDIR% (a Windows
    /// path). `appID` records entries without a "HasRunKey" under Steam's per-app key.
    static func processes(script: Data, installDir: String, appID: Int? = nil) -> [SteamInstallProcess] {
        var result: [SteamInstallProcess] = []
        for (run, fields) in SteamInstallScripts.entries(script: script, appID: appID) {
            let numbers = fields.keys.compactMap { key -> Int? in
                guard key.hasPrefix("process ") else { return nil }
                return Int(key.dropFirst("process ".count).trimmingCharacters(in: .whitespaces))
            }.sorted()
            for number in numbers.prefix(8) {
                guard let raw = fields["process \(number)"], !raw.isEmpty, raw.utf8.count <= 1024 else { continue }
                let exe = expand(raw, installDir: installDir)
                let lower = exe.lowercased()
                guard exe.count > 3, exe.hasPrefix("C:\\"), !exe.contains("%"), !exe.contains("\""),
                      !exe.contains(".."), lower.hasSuffix(".exe") || lower.hasSuffix(".msi") else { continue }
                let args = expand(fields["command \(number)"] ?? "", installDir: installDir, path: false)
                    .trimmingCharacters(in: .whitespaces)
                // No cmd.exe metacharacters: the arguments go into a batch line as they are.
                guard args.utf8.count <= 1024, !args.contains("\r"), !args.contains("\n"), !args.contains("&"),
                      !args.contains("|"), !args.contains(">"), !args.contains("<"), !args.contains("^"),
                      !args.contains("%") else { continue }
                let process = SteamInstallProcess(run: run, executable: exe, arguments: args)
                if !result.contains(process) { result.append(process) }
            }
        }
        return result
    }

    /// A program path gets Windows separators; an argument list keeps its "/switches".
    static func expand(_ text: String, installDir: String, path: Bool = true) -> String {
        var value = path ? text.replacingOccurrences(of: "/", with: "\\") : text
        value = value.replacingOccurrences(of: "%INSTALLDIR%", with: installDir, options: .caseInsensitive)
        if path { while value.contains("\\\\") { value = value.replacingOccurrences(of: "\\\\", with: "\\") } }
        return value
    }

    /// The value a run has in a Wine .reg text (highest of both views), nil when absent.
    static func recorded(_ run: SteamInstallRun, in text: String) -> UInt32? {
        let lines = text.components(separatedBy: "\n")
        let valueName = ("\"" + SteamInstallScripts.escape(run.name) + "\"=").lowercased()
        var best: UInt32?
        for key in SteamInstallScripts.keys(run) {
            let header = ("[" + SteamInstallScripts.escape(key) + "]").lowercased()
            guard let start = lines.firstIndex(where: { $0.lowercased().hasPrefix(header) }) else { continue }
            var index = start + 1
            while index < lines.count, !lines[index].hasPrefix("[") {
                let line = lines[index].lowercased()
                if line.hasPrefix(valueName), let range = line.range(of: "=dword:"),
                   let value = UInt32(line[range.upperBound...].trimmingCharacters(in: .whitespacesAndNewlines), radix: 16) {
                    best = max(best ?? 0, value)
                }
                index += 1
            }
        }
        return best
    }

    /// Whether a run is recorded done (value at least its minimum in either view).
    static func marked(_ run: SteamInstallRun, in text: String) -> Bool {
        (recorded(run, in: text) ?? 0) >= run.value
    }

    /// Every program's fate at this start: done first, then whether the program exists,
    /// then whether this build can run it (`runnable` returns the reason it cannot);
    /// at most `limit` programs run per start.
    static func plan(_ found: [SteamInstallProcess], limit: Int = 8,
                     done: (SteamInstallRun) -> Bool, exists: (String) -> Bool,
                     runnable: (SteamInstallProcess) -> String? = { _ in nil }) -> [DockInstallPlanItem] {
        var result: [DockInstallPlanItem] = []
        var queued = 0
        for process in found {
            if done(process.run) { result.append(DockInstallPlanItem(process: process, status: .done)); continue }
            if !exists(process.executable) { result.append(DockInstallPlanItem(process: process, status: .missing)); continue }
            if let reason = runnable(process) {
                result.append(DockInstallPlanItem(process: process, status: .unsupported, reason: reason)); continue
            }
            if queued < limit { queued += 1; result.append(DockInstallPlanItem(process: process, status: .pending)) }
            else { result.append(DockInstallPlanItem(process: process, status: .limit)) }
        }
        return result
    }

    /// The Dock sheet's line about the plan, or nil when every program is already done.
    static func note(_ items: [DockInstallPlanItem]) -> String? {
        func names(_ status: DockInstallStatus) -> String {
            var seen: [String] = []
            for item in items where item.status == status {
                var text = label(item.process.run)
                if let reason = item.reason { text += " (" + reason + ")" }
                if !seen.contains(text) { seen.append(text) }
            }
            return seen.joined(separator: ", ")
        }
        guard items.contains(where: { $0.status != .done }) else { return nil }
        var parts: [String] = []
        if !names(.pending).isEmpty { parts.append("Runs first: " + names(.pending)) }
        if !names(.done).isEmpty { parts.append("Already done: " + names(.done)) }
        if !names(.missing).isEmpty { parts.append("Installer file missing: " + names(.missing)) }
        if !names(.unsupported).isEmpty { parts.append("Cannot run in this build: " + names(.unsupported)) }
        if !names(.limit).isEmpty { parts.append("Next start: " + names(.limit)) }
        return "One-time installs. " + parts.joined(separator: ". ") + "."
    }

    /// A program's label for logs, the batch and the sheet: the entry name, reduced to
    /// safe characters.
    static func label(_ run: SteamInstallRun) -> String {
        String(run.name.filter { $0.isLetter || $0.isNumber || $0 == " " || $0 == "." || $0 == "-" || $0 == "_" }.prefix(64))
    }

    /// The batch the Dock session runs before the host. Each program's start and exit
    /// status are appended to `resultFile` ("start <i> <label>", "exit <i> <status>"; every
    /// redirection follows a space, so a status digit is never read as a handle number).
    /// The batch records nothing in the registry itself: Madeira records the programs that
    /// exited with 0, 3010 or 1641 (installed, restart requested) at the next Dock start,
    /// when no session runs (DockInstallers.absorbResults).
    ///
    /// `services` first makes Wine's service manager reachable (`dockhost.exe
    /// --start-services` prints "services <outcome>" into the result file; "services failed"
    /// when it could not run) or records "services off".
    static func batch(_ processes: [SteamInstallProcess], resultFile: String, services: DockInstallServices) -> String {
        let result = " >>\"" + resultFile.replacingOccurrences(of: "\"", with: "") + "\""
        var lines = ["@echo off", "rem Madeira Dock: a game's one-time installs, before the host starts the game",
                     "echo begin \(processes.count)" + result.replacingOccurrences(of: " >>", with: " >")]
        switch services {
        case .off: lines.append("echo services off" + result)
        case .start(let executable):
            lines.append("\"" + executable.replacingOccurrences(of: "\"", with: "") + "\" --start-services" + result)
            lines.append("if not \"%ERRORLEVEL%\"==\"0\" echo services failed" + result)
        }
        for (offset, process) in processes.enumerated() {
            let index = offset + 1
            let quoted = "\"" + process.executable + "\""
            let command = process.executable.lowercased().hasSuffix(".msi")
                ? "C:\\windows\\system32\\msiexec.exe /i " + quoted + (process.arguments.isEmpty ? "" : " " + process.arguments)
                : "call " + quoted + (process.arguments.isEmpty ? "" : " " + process.arguments)
            let text = label(process.run)
            lines.append("echo [dock-installers] running \(index)/\(processes.count) " + text)
            lines.append("echo start \(index) " + text + result)
            lines.append(command)
            lines.append("echo exit \(index) %ERRORLEVEL%" + result)
        }
        lines.append("echo end" + result)
        return lines.joined(separator: "\r\n") + "\r\n"
    }

    /// A result file read back: programs in the batch, which started (index -> label),
    /// which exited (index -> status), the service step's outcome and whether the batch ended.
    struct Results: Equatable, Sendable {
        var total = 0
        var started: [Int: String] = [:]
        var exits: [Int: Int] = [:]
        var ended = false
        var services: String?
        var running: Int? { started.keys.filter { exits[$0] == nil }.max() }
        static func succeeded(_ status: Int) -> Bool { status == 0 || status == 3010 || status == 1641 }
    }

    static func results(_ text: String) -> Results {
        var results = Results()
        // cmd.exe writes CRLF; "\r\n" is one Character, so split on any newline Character.
        for raw in text.split(whereSeparator: { $0.isNewline }).prefix(200) {
            let parts = raw.trimmingCharacters(in: .whitespacesAndNewlines).split(separator: " ", maxSplits: 2).map(String.init)
            guard let verb = parts.first else { continue }
            switch verb {
            case "begin":
                results.total = parts.count > 1 ? min(Int(parts[1]) ?? 0, 64) : 0
            case "start":
                if parts.count > 1, let index = Int(parts[1]), (1...64).contains(index) {
                    results.started[index] = parts.count > 2 ? String(parts[2].prefix(64)) : ""
                }
            case "exit":
                if parts.count > 2, let index = Int(parts[1]), (1...64).contains(index),
                   let status = Int(parts[2].trimmingCharacters(in: .whitespaces)) { results.exits[index] = status }
            case "services":
                let word = parts.count > 1 ? String(parts[1].filter { $0.isLetter }.prefix(16)) : ""
                if !word.isEmpty { results.services = word }
            case "end":
                results.ended = true
            default:
                continue
            }
        }
        return results
    }
}

/// Kept next to the prefix's registry files, one small JSON file: the programs the last
/// batch runs, in its order (result lines refer to them by index), and each game's
/// One-time installs choice.
struct DockInstallLedger: Codable, Equatable, Sendable {
    static let fileName = "madeira-dock-installs.json"
    var session: [SteamInstallRun] = []
    var sessionApp: Int = 0
    /// App ID -> false: the game's next Dock start skips its one-time installs (Skip).
    /// Absent or true: the next start runs the pending ones ("Run at next start").
    var runNext: [String: Bool] = [:]

    static func load(prefix: URL) -> DockInstallLedger {
        let file = prefix.appendingPathComponent(fileName)
        guard let contents = try? Data(contentsOf: file), contents.count <= 1 << 20,
              let ledger = try? JSONDecoder().decode(DockInstallLedger.self, from: contents) else { return DockInstallLedger() }
        return ledger
    }
    func save(prefix: URL) throws {
        let encoder = JSONEncoder(); encoder.outputFormatting = [.sortedKeys]
        try encoder.encode(self).write(to: prefix.appendingPathComponent(Self.fileName), options: .atomic)
    }
    func runsNext(_ appID: Int) -> Bool { runNext[String(appID)] != false }
}

// MARK: - A Dock start's one-time installs

/// The app side: plan the game's programs before a Dock start, write the batch the
/// session runs before the host, report progress, and record results at the next start.
///
/// Switches (`env.NAME` in madeira.cfg; "0" turns one off):
/// - `MADEIRA_DOCK_INSTALLERS`: the whole feature.
/// - `MADEIRA_INSTALL_DEFAULT_KEY`: entries without a "HasRunKey" use Steam's per-app key
///   (off: such entries are ignored).
/// - `MADEIRA_DOCK_INSTALL_CHOICE`: the per-game One-time installs choice (off: every start
///   runs whatever is pending).
/// - `MADEIRA_DOCK_INSTALL_SCM`: the service-manager step before the first installer.
/// - `MADEIRA_DOCK_INSTALL_SERVER_SYNC`: a start that runs installers turns madsync off for
///   its session (off: madsync stays and the service-manager step is left out).
/// - `MADEIRA_DOTNET_FUSION`: place Wine's fusion.dll in the .NET 2.0 folder when missing.
@MainActor
enum DockInstallers {
    static let scriptName = "madeira-dock-installers.cmd"
    static let resultName = "madeira-dock-installers.result"

    /// The batch this start runs before the host (a C:\ path), nil when none runs.
    private(set) static var script: String?
    /// This start runs installers and turns madsync off for its session.
    private(set) static var serverSync = false
    /// This start's plan in words (the Dock sheet's status), nil when there is nothing to say.
    private(set) static var note: String?
    /// When poll first read the batch's end: the host starts next. nil before, and
    /// for a start without installs.
    private(set) static var finishedAt: Date?
    private static var logged: Set<String> = []

    nonisolated static func flag(_ name: String) -> Bool { SteamSignIn.flag(name, default: true) }
    nonisolated static var enabled: Bool { flag("MADEIRA_DOCK_INSTALLERS") }
    nonisolated static var choiceEnabled: Bool { enabled && flag("MADEIRA_DOCK_INSTALL_CHOICE") }

    /// Whether madeira.cfg selects madsync, as the engine reads it (madeira_cfg_sync_engine
    /// in build/madeira_cfg.h): only inproc-sync set to 1/on/true/yes; unset is fastsync,
    /// the default engine.
    static var madsyncConfigured: Bool {
        guard let value = MadeiraConfig.get("inproc-sync") else { return false }
        return ["1", "on", "true", "yes"].contains(value)
    }

    // MARK: Finding programs

    /// The install scripts' programs of a game: its own folder (one level down) and the
    /// shared redistributables Steam keeps next to it, and how many scripts were read.
    nonisolated static func found(_ game: DockGame, drive: URL,
                                  defaultKey: Bool = DockInstallers.flag("MADEIRA_INSTALL_DEFAULT_KEY")) -> (processes: [SteamInstallProcess], scripts: Int) {
        let common = game.library + "/common"
        let shared = common + "/Steamworks Shared"
        func windows(_ relative: String) -> String { "C:\\" + relative.replacingOccurrences(of: "/", with: "\\") }
        let roots: [(URL, Int, String, Int?)] = [
            (drive.appendingPathComponent(common + "/" + game.installDir, isDirectory: true), 1, game.windowsInstallPath,
             defaultKey ? game.id : nil),
            (drive.appendingPathComponent(shared + "/_CommonRedist", isDirectory: true), 3, windows(shared), nil),
        ]
        var result: [SteamInstallProcess] = []
        var scripts = 0
        for (root, depth, installDir, owner) in roots {
            guard inside(root, drive: drive) else { continue }
            for file in SteamInstallScripts.scripts(folder: root, depth: depth) {
                guard let contents = try? Data(contentsOf: file) else { continue }
                scripts += 1
                for process in DockInstallScripts.processes(script: contents, installDir: installDir, appID: owner)
                where !result.contains(process) {
                    result.append(process)
                }
            }
        }
        return (result, scripts)
    }

    /// The number of install-script programs a game has (the Dock sheet's list).
    nonisolated static func programCount(_ game: DockGame, drive: URL) -> Int {
        enabled ? found(game, drive: drive).processes.count : 0
    }

    nonisolated static func inside(_ url: URL, drive: URL) -> Bool {
        url.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(drive.resolvingSymlinksInPath().standardizedFileURL.path + "/")
    }

    /// A C:\ path resolved on disk the way Wine resolves it (case-insensitive), staying
    /// inside drive_c; nil when it does not exist.
    nonisolated static func resolve(_ windowsPath: String, drive: URL) -> URL? {
        guard windowsPath.uppercased().hasPrefix("C:\\"), windowsPath.utf8.count < 1024 else { return nil }
        let parts = windowsPath.dropFirst(3).split(separator: "\\", omittingEmptySubsequences: false).map(String.init)
        guard !parts.isEmpty, !parts.contains(where: { $0.isEmpty || $0 == "." || $0 == ".." || $0.contains("/") }) else { return nil }
        var current = drive
        for part in parts {
            let exact = current.appendingPathComponent(part)
            if FileManager.default.fileExists(atPath: exact.path) { current = exact }
            else {
                guard let names = try? FileManager.default.contentsOfDirectory(atPath: current.path),
                      let match = names.first(where: { $0.caseInsensitiveCompare(part) == .orderedSame }) else { return nil }
                current.appendPathComponent(match)
            }
            guard inside(current, drive: drive) else { return nil }
        }
        return current
    }

    /// The PE machine of an executable (0x14c: 32-bit x86, 0x8664: x64), nil when unreadable.
    nonisolated static func machine(_ file: URL) -> UInt16? {
        guard let handle = try? FileHandle(forReadingFrom: file) else { return nil }
        defer { try? handle.close() }
        guard let dos = try? handle.read(upToCount: 64), dos.count == 64, dos[0] == 0x4d, dos[1] == 0x5a else { return nil }
        let offset = (0..<4).reduce(UInt64(0)) { $0 | UInt64(dos[60 + $1]) << ($1 * 8) }
        guard offset >= 64, offset <= 1 << 20, (try? handle.seek(toOffset: offset)) != nil,
              let header = try? handle.read(upToCount: 6), header.count == 6, header.starts(with: [0x50, 0x45, 0, 0]) else { return nil }
        return UInt16(header[4]) | UInt16(header[5]) << 8
    }

    /// Why this build cannot run a program, nil when it can. Decided by what the program is
    /// (a Windows Installer package, a 32-bit executable) and what the bundle has, never by
    /// its name: a package needs msiexec.exe, a 32-bit executable needs 32-bit support.
    nonisolated static func runnable(_ process: SteamInstallProcess, drive: URL, has32Bit: Bool, hasMsiexec: Bool) -> String? {
        if process.executable.lowercased().hasSuffix(".msi") { return hasMsiexec ? nil : "Windows Installer package" }
        guard let file = resolve(process.executable, drive: drive), let machine = machine(file) else { return nil }
        return machine == 0x14c && !has32Bit ? "32-bit installer" : nil
    }

    nonisolated static var bundleHas32Bit: Bool { bundled("i386-windows") }
    nonisolated static var bundleHasMsiexec: Bool {
        ["aarch64-windows", "arm64ec-windows", "i386-windows"].contains { bundled($0 + "/msiexec.exe") }
    }
    nonisolated private static func bundled(_ relative: String) -> Bool {
        guard let root = Bundle.main.resourceURL else { return false }
        return FileManager.default.fileExists(atPath: root.appendingPathComponent(relative).path)
    }

    // MARK: The choice

    nonisolated static func runsNext(_ appID: Int, prefix: URL) -> Bool { DockInstallLedger.load(prefix: prefix).runsNext(appID) }

    static func setRunsNext(_ appID: Int, _ run: Bool, prefix: URL) {
        var ledger = DockInstallLedger.load(prefix: prefix)
        ledger.runNext[String(appID)] = run ? nil : false
        do { try ledger.save(prefix: prefix) }
        catch { LogStore.shared.log("[dock-installers] app=\(appID) choice not saved: \(error.localizedDescription)", level: .error); return }
        LogStore.shared.log("[dock-installers] app=\(appID) choice=\(run ? "run" : "skip")")
    }

    // MARK: A start

    /// Called right before a Dock start, while no session runs (the registry is on disk).
    /// Records the previous batch's results, plans the game's programs and writes the batch.
    static func prepare(_ game: DockGame, drive: URL, prefix: URL,
                        has32Bit: Bool = DockInstallers.bundleHas32Bit, hasMsiexec: Bool = DockInstallers.bundleHasMsiexec,
                        fusionSource: URL? = Bundle.main.resourceURL?.appendingPathComponent("i386-windows/fusion.dll")) {
        script = nil; serverSync = false; note = nil; finishedAt = nil; logged = []
        let app = game.id
        let batchFile = drive.appendingPathComponent(scriptName)
        try? FileManager.default.removeItem(at: batchFile)
        absorbResults(reason: "next-start", drive: drive, prefix: prefix)
        try? FileManager.default.removeItem(at: drive.appendingPathComponent(resultName))
        guard enabled else {
            LogStore.shared.log("[dock-installers] app=\(app) off (MADEIRA_DOCK_INSTALLERS=0)"); return
        }
        let defaultKey = flag("MADEIRA_INSTALL_DEFAULT_KEY")
        let (found, scripts) = Self.found(game, drive: drive, defaultKey: defaultKey)
        guard !found.isEmpty else {
            LogStore.shared.log("[dock-installers] app=\(app) scripts=\(scripts) programs=0"); return
        }
        let registry = [SteamInstallRun.Hive.machine: "system.reg", .user: "user.reg"].mapValues {
            (try? String(contentsOf: prefix.appendingPathComponent($0), encoding: .utf8)) ?? ""
        }
        let items = DockInstallScripts.plan(found,
                                            done: { DockInstallScripts.marked($0, in: registry[$0.hive] ?? "") },
                                            exists: { resolve($0, drive: drive) != nil },
                                            runnable: { runnable($0, drive: drive, has32Bit: has32Bit, hasMsiexec: hasMsiexec) })
        var pending = items.filter { $0.status == .pending }.map(\.process)
        var ledger = DockInstallLedger.load(prefix: prefix)
        // The game's One-time installs choice: absent or "Run at next start" runs the pending
        // programs at this start, after which the choice becomes Skip (unless some wait for a
        // later start); Skip starts the game without them. MADEIRA_DOCK_INSTALL_CHOICE=0 ignores it.
        let choiceOn = flag("MADEIRA_DOCK_INSTALL_CHOICE")
        let skipped = choiceOn && !ledger.runsNext(app) && !pending.isEmpty
        if skipped {
            LogStore.shared.log("[dock-installers] app=\(app) choice=skip pending=\(pending.count)")
            pending = []
        } else if choiceOn && !pending.isEmpty && !items.contains(where: { $0.status == .limit }) {
            ledger.runNext[String(app)] = false
            LogStore.shared.log("[dock-installers] app=\(app) choice=run pending=\(pending.count); the next start skips them")
        }
        // Installers expect a service manager, and a Dock session starts none before the
        // host. With madsync, Wine's services.exe never answered its RPC clients in the
        // fork's device runs, so the service step and then every installer waited; msiexec's
        // custom actions use the same RPC server. A start that runs installers therefore
        // turns madsync off for its whole session (MADEIRA_MADSYNC_SESSION=0, set by the
        // caller); later sessions use the configured engine again.
        // MADEIRA_DOCK_INSTALL_SERVER_SYNC=0 keeps madsync and leaves the service step out.
        let sessionOff = !pending.isEmpty && flag("MADEIRA_DOCK_INSTALL_SERVER_SYNC")
        let services: DockInstallServices = flag("MADEIRA_DOCK_INSTALL_SCM") && (sessionOff || !madsyncConfigured)
            ? .start(MadeiraDock.executable) : .off
        if !pending.isEmpty {
            do {
                let text = DockInstallScripts.batch(pending, resultFile: "C:\\" + resultName, services: services)
                try Data(text.utf8).write(to: batchFile, options: .atomic)
                script = "C:\\" + scriptName
                serverSync = sessionOff
            } catch {
                LogStore.shared.log("[dock-installers] app=\(app) batch not written: \(error.localizedDescription)", level: .error)
            }
        }
        ledger.session = script != nil ? pending.map(\.run) : []
        ledger.sessionApp = ledger.session.isEmpty ? 0 : app
        do { try ledger.save(prefix: prefix) }
        catch { LogStore.shared.log("[dock-installers] app=\(app) ledger not saved: \(error.localizedDescription)", level: .error) }
        for item in items {
            let run = item.process.run
            let recorded = DockInstallScripts.recorded(run, in: registry[run.hive] ?? "").map { String($0) } ?? "-"
            let file = item.process.executable.split(separator: "\\").last.map(String.init) ?? ""
            LogStore.shared.log("[dock-installers] app=\(app) program=\(DockInstallScripts.label(run)) file=\(file) " +
                                "status=\(item.status.rawValue)\(item.reason.map { " reason=" + $0.replacingOccurrences(of: " ", with: "-") } ?? "") " +
                                "key=\(run.hive == .machine ? "HKLM" : "HKCU")\\\(run.key) recorded=\(recorded) minimum=\(run.value)")
        }
        if skipped {
            note = "One-time installs skipped. To run them, choose Run at next start under One-time installs."
        } else {
            note = DockInstallScripts.note(items)
            if serverSync && madsyncConfigured {
                note = (note.map { $0 + " " } ?? "") + "This start uses Wine's standard synchronization instead of madsync; the next start uses madsync again."
            }
        }
        if script != nil && flag("MADEIRA_DOTNET_FUSION") {
            LogStore.shared.log("[dock-installers] app=\(app) dotnet-fusion=\(placeDotNetFusion(drive: drive, source: fusionSource))")
        }
        LogStore.shared.log("[dock-installers] app=\(app) scripts=\(scripts) programs=\(found.count) pending=\(pending.count) " +
                            "done=\(items.filter { $0.status == .done }.count) missing=\(items.filter { $0.status == .missing }.count) " +
                            "unsupported=\(items.filter { $0.status == .unsupported }.count) default-key=\(defaultKey ? 1 : 0) " +
                            "services-step=\(script == nil ? "-" : services == .off ? "off" : "start") session-madsync-off=\(serverSync ? 1 : 0)")
    }

    /// An installer with a managed step (DirectX setup's is one) loads fusion.dll from the
    /// .NET 2.0 framework folder through mscoree. Wine normally gets that file from its Mono
    /// package, which Madeira does not ship, and such a setup ended with -9 in the fork's
    /// device runs. Before a start that runs installers, Wine's own builtin 32-bit fusion.dll
    /// is placed there, only when missing. Returns "placed", "present", "no-source" or "failed".
    static func placeDotNetFusion(drive: URL, source: URL?) -> String {
        let fm = FileManager.default
        let folder = drive.appendingPathComponent("windows/Microsoft.NET/Framework/v2.0.50727", isDirectory: true)
        let target = folder.appendingPathComponent("fusion.dll")
        if fm.fileExists(atPath: target.path) { return "present" }
        guard let source, fm.fileExists(atPath: source.path) else { return "no-source" }
        do {
            try fm.createDirectory(at: folder, withIntermediateDirectories: true)
            try fm.copyItem(at: source, to: target)
            return "placed"
        } catch { return "failed" }
    }

    /// The previous batch's exit statuses, read at the next Dock start (no session runs, so
    /// the registry is on disk): programs that succeeded are recorded done, one summary line
    /// is logged, and the result file and session list are cleared.
    static func absorbResults(reason: String, drive: URL, prefix: URL) {
        var ledger = DockInstallLedger.load(prefix: prefix)
        guard !ledger.session.isEmpty else { return }
        let app = ledger.sessionApp
        let file = drive.appendingPathComponent(resultName)
        if let contents = try? Data(contentsOf: file), contents.count <= 16384 {
            let results = DockInstallScripts.results(String(decoding: contents, as: UTF8.self))
            var succeeded: [SteamInstallRun] = []
            var statuses: [String] = []
            for (offset, run) in ledger.session.enumerated() {
                let index = offset + 1, text = DockInstallScripts.label(run)
                if let status = results.exits[index] {
                    if DockInstallScripts.Results.succeeded(status) { succeeded.append(run) }
                    statuses.append("\(text)=\(status)")
                } else {
                    statuses.append("\(text)=\(results.started[index] != nil ? "unfinished" : "not-started")")
                }
            }
            var written = 0
            do { written = try SteamInstallScripts.mark(succeeded, prefix: prefix) }
            catch { LogStore.shared.log("[dock-installers] app=\(app) results not recorded: \(error.localizedDescription)", level: .error) }
            let failed = ledger.session.count - succeeded.count
            LogStore.shared.log("[dock-installers] app=\(app) results reason=\(reason) succeeded=\(succeeded.count) failed=\(failed) " +
                                "recorded-values=\(written) services=\(results.services ?? "-") ended=\(results.ended ? 1 : 0) " +
                                "statuses=\(statuses.joined(separator: ","))", level: failed > 0 ? .error : .info)
        } else {
            LogStore.shared.log("[dock-installers] app=\(app) results reason=\(reason) none: the batch did not start", level: .error)
        }
        try? FileManager.default.removeItem(at: file)
        ledger.session = []; ledger.sessionApp = 0
        try? ledger.save(prefix: prefix)
    }

    /// The batch's progress from its result file, for the Dock sheet and the starting
    /// screen. New start and exit lines, and the end, are logged once; the text names the
    /// program running now and any that failed, and at the end how many succeeded.
    static func poll(drive: URL) -> String? {
        guard script != nil else { return nil }
        let file = drive.appendingPathComponent(resultName)
        guard let contents = try? Data(contentsOf: file), contents.count <= 16384 else { return "Starting this game's one-time installs…" }
        let results = DockInstallScripts.results(String(decoding: contents, as: UTF8.self))
        if let services = results.services, logged.insert("services").inserted {
            LogStore.shared.log("[dock-installers] services=\(services)", level: services == "failed" || services == "timeout" ? .error : .info)
        }
        for (index, program) in results.started.sorted(by: { $0.key < $1.key }) where logged.insert("s\(index)").inserted {
            LogStore.shared.log("[dock-installers] start \(index)/\(results.total) program=\(program)")
        }
        var failed: [String] = []
        for (index, status) in results.exits.sorted(by: { $0.key < $1.key }) {
            let ok = DockInstallScripts.Results.succeeded(status)
            if !ok { failed.append("\(results.started[index] ?? "#\(index)") (exit \(status))") }
            guard logged.insert("e\(index)").inserted else { continue }
            LogStore.shared.log("[dock-installers] exit \(index)/\(results.total) program=\(results.started[index] ?? "") status=\(status) " +
                                (ok ? "succeeded; recorded at the next start" : "failed; not recorded"), level: ok ? .info : .error)
        }
        let failures = failed.isEmpty ? "" : "\nFailed: " + failed.joined(separator: ", ")
        if let running = results.running {
            return "Running one-time install \(running) of \(max(results.total, running)): \(results.started[running] ?? "")…" + failures
        }
        guard results.ended else { return "Running this game's one-time installs…" + failures }
        let total = max(results.total, results.exits.count), succeeded = results.exits.count - failed.count
        if logged.insert("end").inserted {
            finishedAt = Date()
            LogStore.shared.log("[dock-installers] end succeeded=\(succeeded) failed=\(failed.count) of=\(total); the host starts next")
        }
        return "One-time installs finished: \(succeeded) of \(total) succeeded." + failures
    }
}

// MARK: - Direct-folder components and installers

/// One dependency family found by scanning the matching-architecture PE files
/// in a directly added game's folder.
struct FolderComponentFinding: Identifiable, Hashable, Sendable {
    enum State: String, Sendable { case ready, missing, attention, game }
    var id: String
    var title: String
    var detail: String
    var state: State
}

/// An installer already shipped in a game folder. It runs in place so a setup
/// with adjacent CAB/data files keeps seeing them.
struct FolderInstallerCandidate: Identifiable, Hashable, Sendable {
    var id: String { windowsPath.lowercased() }
    var name: String
    var windowsPath: String
    var machine: UInt16?

    var architecture: String {
        switch machine {
        case 0x14c: return "x86"
        case 0x8664: return "x64"
        case 0xaa64: return "ARM64"
        case nil: return "MSI"
        default: return "Unknown"
        }
    }
}

struct FolderComponentReport: Sendable {
    var findings: [FolderComponentFinding]
    var installers: [FolderInstallerCandidate]
    var scannedPEs: Int
    var truncated: Bool
}

struct FolderDLLImportResult: Sendable {
    var copied: Int
    var names: [String]
}

/// Bounded, read-only dependency inspection for games added under drive_c.
/// This intentionally diagnoses; it never downloads or copies a system DLL.
enum FolderComponents {
    private static let protectedDLLs: Set<String> = [
        "ntdll.dll", "kernel32.dll", "kernelbase.dll", "user32.dll", "gdi32.dll", "win32u.dll",
        "advapi32.dll", "combase.dll", "ole32.dll", "rpcrt4.dll", "shell32.dll", "shlwapi.dll"
    ]
    private struct Family {
        var id: String
        var title: String
        var detail: String
        var names: Set<String>
        var prefixes: [String] = []
        var state: FolderComponentFinding.State = .ready
    }

    private static let families: [Family] = [
        Family(id: "vc140", title: "Microsoft Visual C++ 2015–2022",
               detail: "Unified VC runtime (MSVCP/VCRUNTIME 140).", names: ["concrt140.dll", "vcamp140.dll", "vccorlib140.dll", "vcomp140.dll"],
               prefixes: ["msvcp140", "vcruntime140"]),
        Family(id: "vc120", title: "Microsoft Visual C++ 2013",
               detail: "Legacy VC12 runtime.", names: ["msvcp120.dll", "msvcr120.dll"]),
        Family(id: "vc110", title: "Microsoft Visual C++ 2012",
               detail: "Legacy VC11 runtime.", names: ["msvcp110.dll", "msvcr110.dll"]),
        Family(id: "vc100", title: "Microsoft Visual C++ 2010",
               detail: "Legacy VC10 runtime.", names: ["msvcp100.dll", "msvcr100.dll"]),
        Family(id: "vc90", title: "Microsoft Visual C++ 2008",
               detail: "Legacy VC9 runtime.", names: ["msvcp90.dll", "msvcr90.dll"]),
        Family(id: "vc80", title: "Microsoft Visual C++ 2005",
               detail: "Legacy VC8 runtime.", names: ["msvcp80.dll", "msvcr80.dll"]),
        Family(id: "directx", title: "DirectX legacy runtime",
               detail: "D3DX, XAudio, XInput or D3DCompiler components from the legacy DirectX runtime.",
               names: ["xinput1_3.dll", "x3daudio1_7.dll", "xaudio2_7.dll", "d3dcompiler_43.dll"], prefixes: ["d3dx9_", "d3dx10_", "d3dx11_"]),
        Family(id: "dotnet", title: ".NET / Wine Mono",
               detail: "The PE requests the CLR bridge; the application may need a matching managed runtime.", names: ["mscoree.dll"], state: .attention),
    ]

    private static func installationRoot(_ executable: URL, drive: URL) -> URL {
        var folder = executable.deletingLastPathComponent()
        let binaryFolders: Set<String> = ["bin", "binaries", "win32", "win64", "x86", "x64", "release"]
        for _ in 0..<4 {
            guard binaryFolders.contains(folder.lastPathComponent.lowercased()) else { break }
            let parent = folder.deletingLastPathComponent()
            guard DockInstallers.inside(parent, drive: drive),
                  !["program files", "program files (x86)", "games", "common", "steamapps"].contains(parent.lastPathComponent.lowercased()) else { break }
            folder = parent
        }
        return folder
    }

    private static func bundleNames(bits: Int, resource: URL?) -> Set<String> {
        guard let resource else { return [] }
        var folders = bits == 32 ? ["i386-windows", "aarch64-windows"] : ["arm64ec-windows", "aarch64-windows", "x86_64-vcruntime"]
        if bits == 0 { folders += ["i386-windows"] }
        var result = Set<String>()
        for folder in folders {
            let url = resource.appendingPathComponent(folder, isDirectory: true)
            guard let files = try? FileManager.default.contentsOfDirectory(at: url, includingPropertiesForKeys: nil) else { continue }
            result.formUnion(files.map { $0.lastPathComponent.lowercased() })
        }
        return result
    }

    private static func isInstaller(_ url: URL) -> Bool {
        let ext = url.pathExtension.lowercased()
        guard ext == "exe" || ext == "msi" else { return false }
        let text = url.path.lowercased()
        return ["setup", "install", "redist", "prereq", "directx", "dotnet", "support", "commonredist"]
            .contains { text.contains($0) }
    }

    /// Scan at most 20,000 entries and 4,096 matching-architecture PE files.
    /// Imports are compared with game-local and bundled DLLs; API-set contracts
    /// are left to Wine's apisetschema resolver.
    static func analyze(executable: URL, bits: Int, drive: URL,
                        resource: URL? = Bundle.main.resourceURL) -> FolderComponentReport {
        let root = installationRoot(executable, drive: drive)
        let expectedMachine: UInt16 = bits == 32 ? 0x14c : 0x8664
        let manager = FileManager.default
        let keys: Set<URLResourceKey> = [.isRegularFileKey, .isSymbolicLinkKey]
        let walker = manager.enumerator(at: root, includingPropertiesForKeys: Array(keys), options: [.skipsHiddenFiles])
        var imports = Set<String>(), local = Set<String>(), candidates: [FolderInstallerCandidate] = []
        var entries = 0, peFiles = 0, truncated = false

        while let file = walker?.nextObject() as? URL {
            entries += 1
            if entries > 20_000 { truncated = true; break }
            guard DockInstallers.inside(file, drive: drive),
                  let values = try? file.resourceValues(forKeys: keys), values.isRegularFile == true,
                  values.isSymbolicLink != true else { continue }
            let ext = file.pathExtension.lowercased()
            let installerFile = isInstaller(file)
            if installerFile, candidates.count < 20 {
                let windows = "C:\\" + String(file.path.dropFirst(drive.path.count + 1)).replacingOccurrences(of: "/", with: "\\")
                candidates.append(FolderInstallerCandidate(name: file.lastPathComponent, windowsPath: windows,
                                                           machine: ext == "exe" ? DockInstallers.machine(file) : nil))
            }
            if installerFile && file.standardizedFileURL != executable.standardizedFileURL { continue }
            guard ext == "exe" || ext == "dll", DockInstallers.machine(file) == expectedMachine else { continue }
            peFiles += 1
            if peFiles > 4_096 { truncated = true; break }
            local.insert(file.lastPathComponent.lowercased())
            imports.formUnion(LibraryModel.importNames(file))
        }

        // Wine's API-set schema routes these contracts to wintypes.dll. Resolve
        // the host here so an IL2CPP failure is reported as the actionable DLL,
        // not hidden behind an API-set name that is not a physical file.
        if imports.contains("api-ms-win-core-winrt-robuffer-l1-1-0.dll") ||
           imports.contains("api-ms-win-core-winrt-propertysetprivate-l1-1-1.dll") {
            imports.insert("wintypes.dll")
        }
        let available = local.union(bundleNames(bits: bits, resource: resource))
        let missing = imports.filter {
            !available.contains($0) && !$0.hasPrefix("api-ms-") && !$0.hasPrefix("ext-ms-")
        }.sorted()
        var findings: [FolderComponentFinding] = []
        for family in families {
            let used = imports.filter { name in
                family.names.contains(name) || family.prefixes.contains(where: { name.hasPrefix($0) })
            }
            guard !used.isEmpty else { continue }
            let absent = used.filter { missing.contains($0) }.sorted()
            let state: FolderComponentFinding.State = absent.isEmpty ? family.state : .missing
            let detail = absent.isEmpty ? family.detail : "Missing: " + absent.joined(separator: ", ")
            findings.append(FolderComponentFinding(id: family.id, title: family.title, detail: detail, state: state))
        }
        let gameAssembly = root.appendingPathComponent("GameAssembly.dll")
        if manager.fileExists(atPath: gameAssembly.path) {
            findings.append(FolderComponentFinding(id: "il2cpp", title: "Unity IL2CPP", detail: "GameAssembly.dll is supplied by the game; its imported DLLs are checked above.", state: .game))
        }
        let known = Set(families.flatMap { Array($0.names) })
        let other = missing.filter { name in
            !known.contains(name) && !families.contains(where: { family in
                family.prefixes.contains(where: { name.hasPrefix($0) })
            })
        }
        if !other.isEmpty {
            let shown = other.prefix(16).joined(separator: ", ") + (other.count > 16 ? "…" : "")
            findings.append(FolderComponentFinding(id: "missing", title: "Other missing DLLs", detail: shown, state: .missing))
        }
        if findings.isEmpty {
            findings.append(FolderComponentFinding(id: "none", title: "No known runtime gap detected",
                                                   detail: "The scanned PE imports are satisfied by the game folder or Madeira bundle.", state: .ready))
        }
        candidates.sort { $0.name.localizedStandardCompare($1.name) == .orderedAscending }
        return FolderComponentReport(findings: findings, installers: candidates, scannedPEs: peFiles, truncated: truncated)
    }

    /// Install user-supplied runtime DLLs app-locally beside the selected EXE.
    /// Preflight every file before copying, so a rejected architecture or name
    /// cannot leave a half-installed set. Existing files are never overwritten.
    static func installAppLocalDLLs(_ sources: [URL], executable: URL, bits: Int,
                                    drive: URL) throws -> FolderDLLImportResult {
        guard !sources.isEmpty, sources.count <= 64 else { throw LibraryError.message("Choose between 1 and 64 DLL files.") }
        let target = executable.deletingLastPathComponent().resolvingSymlinksInPath().standardizedFileURL
        guard DockInstallers.inside(target, drive: drive) else { throw LibraryError.message("The game's executable folder is outside drive_c.") }
        let expected: UInt16 = bits == 32 ? 0x14c : 0x8664
        let manager = FileManager.default
        let existing = Set((try manager.contentsOfDirectory(atPath: target.path)).map { $0.lowercased() })
        var planned: [(URL, URL, String)] = [], selected = Set<String>()

        for source in sources {
            let name = source.lastPathComponent, lower = name.lowercased()
            guard source.pathExtension.lowercased() == "dll", name == (name as NSString).lastPathComponent,
                  name.utf8.count <= 255, !name.contains("/"), !name.contains("\\") else {
                throw LibraryError.message("Every selected file must be a plainly named .dll file.")
            }
            guard !lower.hasPrefix("api-ms-"), !lower.hasPrefix("ext-ms-"), !protectedDLLs.contains(lower) else {
                throw LibraryError.message("\(name) is a Windows/Wine core DLL and cannot be installed app-locally.")
            }
            let values = try source.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
            guard values.isRegularFile == true, (values.fileSize ?? Int.max) <= 100_000_000 else {
                throw LibraryError.message("\(name) is not a regular DLL smaller than 100 MB.")
            }
            guard DockInstallers.machine(source) == expected else {
                throw LibraryError.message("\(name) does not match this game's \(bits)-bit architecture.")
            }
            guard !existing.contains(lower), selected.insert(lower).inserted else {
                throw LibraryError.message("\(name) already exists in the game folder or was selected twice; nothing was overwritten.")
            }
            planned.append((source, target.appendingPathComponent(name), name))
        }
        for (source, destination, _) in planned { try manager.copyItem(at: source, to: destination) }
        return FolderDLLImportResult(copied: planned.count, names: planned.map { $0.2 })
    }
}

/// Stages a user-selected installer (or runs one in a game folder), writes an
/// ASCII-path command file, and returns a temporary Wine desktop profile.
enum FolderInstallerRunner {
    static let rootName = "madeira-installers"

    private static func safeArguments(_ arguments: String) throws -> String {
        guard arguments.utf8.count <= 1024, !arguments.contains("\r"), !arguments.contains("\n"),
              !arguments.contains("&"), !arguments.contains("|"), !arguments.contains(">"),
              !arguments.contains("<"), !arguments.contains("^"), !arguments.contains("%") else {
            throw LibraryError.message("Installer arguments are too long or contain command-shell metacharacters.")
        }
        var quoted = false
        for character in arguments where character == "\"" { quoted.toggle() }
        guard !quoted else { throw LibraryError.message("Use balanced double quotes in installer arguments.") }
        return arguments.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private static func windowsPath(_ url: URL, drive: URL) throws -> String {
        let resolved = url.resolvingSymlinksInPath().standardizedFileURL
        guard DockInstallers.inside(resolved, drive: drive) else { throw LibraryError.message("The installer is outside drive_c.") }
        return "C:\\" + String(resolved.path.dropFirst(drive.path.count + 1)).replacingOccurrences(of: "/", with: "\\")
    }

    private static func validate(_ file: URL, has32Bit: Bool, hasMsiexec: Bool) throws {
        switch file.pathExtension.lowercased() {
        case "msi":
            guard hasMsiexec else { throw LibraryError.message("This Madeira build does not include msiexec.exe yet, so MSI packages cannot run.") }
        case "exe":
            guard let machine = DockInstallers.machine(file) else { throw LibraryError.message("This is not a readable Windows installer executable.") }
            guard machine == 0x8664 || machine == 0xaa64 || machine == 0x14c else {
                throw LibraryError.message(String(format: "Unsupported installer architecture 0x%04x.", machine))
            }
            if machine == 0x14c && !has32Bit { throw LibraryError.message("This is a 32-bit installer, but this Madeira build has no i386 runtime bundle.") }
        default:
            throw LibraryError.message("Choose a Windows .exe or .msi installer.")
        }
    }

    /// `copy` is true for a Files-picker URL; an installer found inside the
    /// game's folder runs in place so adjacent payload files remain available.
    static func prepare(source: URL, copy: Bool, gameID: UUID, gameTitle: String,
                        arguments: String, drive: URL,
                        has32Bit: Bool = DockInstallers.bundleHas32Bit,
                        hasMsiexec: Bool = DockInstallers.bundleHasMsiexec) throws -> LibraryEntry {
        let args = try safeArguments(arguments)
        let manager = FileManager.default
        let folderName = gameID.uuidString.lowercased()
        let folder = drive.appendingPathComponent(rootName, isDirectory: true).appendingPathComponent(folderName, isDirectory: true)
        try manager.createDirectory(at: folder, withIntermediateDirectories: true)

        let package: URL
        if copy {
            let ext = source.pathExtension.lowercased()
            guard ext == "exe" || ext == "msi" else { throw LibraryError.message("Choose a Windows .exe or .msi installer.") }
            package = folder.appendingPathComponent("package." + ext)
            if source.standardizedFileURL != package.standardizedFileURL {
                if manager.fileExists(atPath: package.path) { try manager.removeItem(at: package) }
                try manager.copyItem(at: source, to: package)
            }
        } else {
            package = source
        }
        try validate(package, has32Bit: has32Bit, hasMsiexec: hasMsiexec)
        let packageWindows = try windowsPath(package, drive: drive)
        let script = folder.appendingPathComponent("run.cmd")
        let result = "C:\\" + rootName + "\\" + folderName + "\\last-result.txt"
        let command = package.pathExtension.lowercased() == "msi"
            ? "call C:\\windows\\system32\\msiexec.exe /i \"\(packageWindows)\"\(args.isEmpty ? "" : " " + args)"
            : "call \"\(packageWindows)\"\(args.isEmpty ? "" : " " + args)"
        let label = package.lastPathComponent.filter { $0.isLetter || $0.isNumber || ".-_ ".contains($0) }.prefix(80)
        let batch = """
        @echo off\r
        >"\(result)" echo begin \(label)\r
        start "" "C:\\windows\\system32\\services.exe"\r
        cd /d "\(packageWindows.dropLast(package.lastPathComponent.count))"\r
        \(command)\r
        set "madeira_status=%ERRORLEVEL%"\r
        >>"\(result)" echo exit %madeira_status%\r
        >>"\(result)" echo end\r
        """
        try Data(batch.utf8).write(to: script, options: .atomic)

        var profile = LibraryEntry(title: "Install for \(gameTitle)", relativePath: "windows/system32/explorer.exe", bits: 64)
        profile.desktop = true
        profile.temporarySession = true
        profile.desktopCommand = "/desktop=components,1280x720 C:\\windows\\system32\\cmd.exe /c C:\\" + rootName + "\\" + folderName + "\\run.cmd"
        profile.resolution = "1280x720"
        profile.graphicsAPI = "Installer"
        profile.liveLogs = true
        return profile
    }

    static func source(_ candidate: FolderInstallerCandidate, drive: URL) -> URL? {
        DockInstallers.resolve(candidate.windowsPath, drive: drive)
    }

    static func lastResult(gameID: UUID, drive: URL) -> String? {
        let file = drive.appendingPathComponent(rootName).appendingPathComponent(gameID.uuidString.lowercased()).appendingPathComponent("last-result.txt")
        guard let data = try? Data(contentsOf: file), data.count <= 4096 else { return nil }
        return String(decoding: data, as: UTF8.self).trimmingCharacters(in: .whitespacesAndNewlines)
    }

    static func winecfgProfile() -> LibraryEntry {
        var profile = LibraryEntry(title: "Wine configuration", relativePath: "windows/system32/explorer.exe", bits: 64)
        profile.desktop = true
        profile.temporarySession = true
        profile.desktopCommand = "/desktop=winecfg,1024x768 C:\\windows\\system32\\winecfg.exe"
        profile.resolution = "1024x768"
        profile.graphicsAPI = "Wine configuration"
        return profile
    }
}
