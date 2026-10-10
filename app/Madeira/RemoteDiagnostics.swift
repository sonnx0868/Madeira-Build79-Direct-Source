import Foundation
import UIKit
import Combine
import Security

enum SupportError: LocalizedError {
    case message(String)
    var errorDescription: String? { if case .message(let text) = self { return text }; return nil }
}

enum DiagnosticContext {
    static func current() -> [String: String] {
        let info = Bundle.main.infoDictionary ?? [:]
        var machine = utsname(); uname(&machine)
        let model = withUnsafePointer(to: &machine.machine) { pointer in
            pointer.withMemoryRebound(to: CChar.self, capacity: 256) { String(cString: $0) }
        }
        return ["version": info["CFBundleShortVersionString"] as? String ?? "unknown",
                "build": info["CFBundleVersion"] as? String ?? "unknown",
                "commit": info["MadeiraSourceCommit"] as? String ?? "unknown",
                "buildLabel": info["MadeiraBuild"] as? String ?? "unknown",
                "device": model, "os": UIDevice.current.systemVersion,
                "physicalMemoryMB": String(ProcessInfo.processInfo.physicalMemory / 1048576),
                "cpuCount": String(ProcessInfo.processInfo.processorCount)]
    }
}

enum DiagnosticKeychain {
    private static var query: [String: Any] {
        [kSecClass as String: kSecClassGenericPassword,
         kSecAttrService as String: "com.madeira.diagnostics",
         kSecAttrAccount as String: "sites-upload"]
    }
    static func load() -> String {
        var attributes = query; attributes[kSecReturnData as String] = true
        attributes[kSecMatchLimit as String] = kSecMatchLimitOne
        var result: CFTypeRef?
        guard SecItemCopyMatching(attributes as CFDictionary, &result) == errSecSuccess,
              let data = result as? Data else { return "" }
        return String(data: data, encoding: .utf8) ?? ""
    }
    static func save(_ value: String) throws {
        if value.isEmpty { SecItemDelete(query as CFDictionary); return }
        let data = Data(value.utf8)
        let updated = SecItemUpdate(query as CFDictionary, [kSecValueData as String: data] as CFDictionary)
        if updated == errSecSuccess { return }
        guard updated == errSecItemNotFound else { throw SupportError.message("The upload key could not be saved securely.") }
        var attributes = query
        attributes[kSecValueData as String] = data
        attributes[kSecAttrAccessible as String] = kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly
        guard SecItemAdd(attributes as CFDictionary, nil) == errSecSuccess else { throw SupportError.message("The upload key could not be saved securely.") }
    }
}

/// Never forward the private Sites service credential to a redirect destination.
final class DiagnosticUploadDelegate: NSObject, URLSessionTaskDelegate {
    func urlSession(_ session: URLSession, task: URLSessionTask,
                    willPerformHTTPRedirection response: HTTPURLResponse, newRequest request: URLRequest,
                    completionHandler: @escaping (URLRequest?) -> Void) { completionHandler(nil) }
}

enum DiagnosticSnapshot {
    static let maxBytes = 20 * 1024 * 1024
    static func create(from source: URL, companions: [DiagnosticCompanionLogs.Source] = []) throws
        -> (url: URL, originalBytes: UInt64, truncated: Bool, companionCount: Int) {
        let input = try FileHandle(forReadingFrom: source)
        defer { try? input.close() }
        let size = try input.seekToEnd()
        guard size > 0 else { throw SupportError.message("There is no log in this session yet.") }
        // Read only small tails at upload time; no game-thread polling or full
        // Steam/profile directory archive. Reserve their space inside 20 MB.
        var sections: [Data] = []
        var companionTruncated = false
        for log in companions.prefix(8) {
            guard let handle = try? FileHandle(forReadingFrom: log.url) else { continue }
            defer { try? handle.close() }
            guard let bytes = try? handle.seekToEnd(), bytes > 0 else { continue }
            let limit = 512 * 1024
            let offset = bytes > UInt64(limit) ? bytes - UInt64(limit) : 0
            do { try handle.seek(toOffset: offset) } catch { continue }
            guard let tail = try? handle.read(upToCount: limit), !tail.isEmpty else { continue }
            let name = String(log.name.prefix(300)).replacingOccurrences(of: "\n", with: " ").replacingOccurrences(of: "\r", with: " ")
            var section = Data("\n[companion-log] name=\(name) originalBytes=\(bytes) tailOffset=\(offset)\n".utf8)
            section.append(tail)
            section.append(Data("\n[/companion-log]\n".utf8))
            sections.append(section)
            companionTruncated = companionTruncated || offset > 0
        }
        let sessionBudget = maxBytes - sections.reduce(0) { $0 + $1.count }
        let target = FileManager.default.temporaryDirectory.appendingPathComponent("madeira-report-\(UUID().uuidString).txt")
        guard FileManager.default.createFile(atPath: target.path, contents: nil) else { throw SupportError.message("The log snapshot could not be created.") }
        let output = try FileHandle(forWritingTo: target)
        defer { try? output.close() }
        do {
            func copy(_ offset: UInt64, _ count: Int) throws {
                try input.seek(toOffset: offset)
                var remaining = count
                while remaining > 0 {
                    let data = try input.read(upToCount: min(65536, remaining)) ?? Data()
                    if data.isEmpty { break }
                    try output.write(contentsOf: data); remaining -= data.count
                }
            }
            if size <= UInt64(sessionBudget) { try copy(0, Int(size)) }
            else {
                let marker = Data("\n[report] Middle of log omitted; keeping startup and final output. Original bytes=\(size)\n".utf8)
                let head = 64 * 1024
                let tail = sessionBudget - head - marker.count
                try copy(0, head); try output.write(contentsOf: marker)
                try copy(size - UInt64(tail), tail)
            }
            for section in sections { try output.write(contentsOf: section) }
            return (target, size, size > UInt64(sessionBudget) || companionTruncated, sections.count)
        } catch { try? FileManager.default.removeItem(at: target); throw error }
    }
}

enum DiagnosticCompanionLogs {
    struct Source { let url: URL; let name: String }

    /// Unity and Steam write important network errors to their own files,
    /// outside stderr. Include only known log names updated during this test.
    /// Previous-session reports exclude files overwritten by the current run.
    static func find(in drive: URL, since: Date, before: Date) -> [Source] {
        let fm = FileManager.default
        let root = drive.resolvingSymlinksInPath()
        let keys: Set<URLResourceKey> = [.isRegularFileKey, .isDirectoryKey, .isSymbolicLinkKey, .contentModificationDateKey]
        var found: [Source] = []
        var visited = 0
        func inside(_ url: URL) -> Bool { url.resolvingSymlinksInPath().path.hasPrefix(root.path + "/") }
        func add(_ url: URL) {
            guard found.count < 8, inside(url),
                  let values = try? url.resourceValues(forKeys: keys), values.isRegularFile == true,
                  values.isSymbolicLink != true, let modified = values.contentModificationDate,
                  modified >= since, modified <= before else { return }
            let name = String(url.path.dropFirst(root.path.count + 1))
            found.append(Source(url: url, name: name))
        }
        for name in ["connection_log.txt", "networking_sockets.txt", "steamnetworkingsockets.log"] {
            add(root.appendingPathComponent("Program Files (x86)/Steam/logs/" + name))
        }
        add(root.appendingPathComponent("madeira-translation-lab.txt"))
        let users = root.appendingPathComponent("users", isDirectory: true)
        func folders(at directory: URL) -> [URL] {
            guard inside(directory), let entries = try? fm.contentsOfDirectory(at: directory, includingPropertiesForKeys: Array(keys), options: .skipsHiddenFiles) else { return [] }
            return entries.filter {
                guard inside($0), let values = try? $0.resourceValues(forKeys: keys) else { return false }
                return values.isDirectory == true && values.isSymbolicLink != true
            }.sorted { $0.path < $1.path }
        }
        for profile in folders(at: users).prefix(32) {
            let localLow = profile.appendingPathComponent("AppData/LocalLow", isDirectory: true)
            // Visit the known Unity company/product layout explicitly. Avoid
            // recursive enumeration and skipDescendants semantics for links.
            for company in folders(at: localLow) {
                for product in folders(at: company) {
                    visited += 1
                    if visited > 4096 || found.count >= 8 { return found }
                    add(product.appendingPathComponent("Player.log"))
                }
            }
        }
        return found
    }
}

@MainActor final class RemoteDiagnostics: ObservableObject {
    static let shared = RemoteDiagnostics()
    static let defaultServer = "https://madeira-diagnostics-sonnx0868.desprinterval-team2.chatgpt.site"
    @Published var server: String
    @Published var key: String
    @Published var busy = false
    @Published var status: String?
    @Published var reportID: String?
    private var pending: (url: URL, id: String, metadata: [String: String], previous: Bool, label: String)?
    private let delegate = DiagnosticUploadDelegate()
    private lazy var session = URLSession(configuration: .ephemeral, delegate: delegate, delegateQueue: nil)
    private init() {
        server = UserDefaults.standard.string(forKey: "madeira.diagnostics.server") ?? Self.defaultServer
        key = DiagnosticKeychain.load()
    }
    private func endpoint(_ address: String? = nil) throws -> URL {
        guard let url = URL(string: (address ?? server).trimmingCharacters(in: .whitespacesAndNewlines)), url.scheme == "https",
              let host = url.host, host.hasSuffix(".chatgpt.site"), url.user == nil, url.password == nil,
              url.query == nil, url.fragment == nil, url.port == nil, url.path.isEmpty || url.path == "/" else {
            throw SupportError.message("Enter the HTTPS origin of your private Sites log server.")
        }
        return url.appendingPathComponent("api/logs")
    }
    func importConfiguration(from file: URL) {
        let access = file.startAccessingSecurityScopedResource()
        defer { if access { file.stopAccessingSecurityScopedResource() } }
        do {
            guard let size = try file.resourceValues(forKeys: [.fileSizeKey]).fileSize, size <= 8192 else {
                throw SupportError.message("Choose the small server configuration file downloaded from your private log server.")
            }
            struct Configuration: Decodable { let format: Int; let server: String; let uploadKey: String }
            let config = try JSONDecoder().decode(Configuration.self, from: Data(contentsOf: file))
            _ = try endpoint(config.server)
            guard config.format == 1, !config.uploadKey.isEmpty,
                  !config.uploadKey.contains("\r"), !config.uploadKey.contains("\n") else { throw SupportError.message("Invalid server configuration.") }
            try DiagnosticKeychain.save(config.uploadKey)
            server = config.server; key = config.uploadKey
            UserDefaults.standard.set(server, forKey: "madeira.diagnostics.server")
            status = "Log server connected. You can send a test log now."
        } catch { status = error.localizedDescription }
    }
    func saveConfiguration() {
        do {
            _ = try endpoint()
            try DiagnosticKeychain.save(key.trimmingCharacters(in: .whitespacesAndNewlines))
            key = key.trimmingCharacters(in: .whitespacesAndNewlines)
            UserDefaults.standard.set(server.trimmingCharacters(in: .whitespacesAndNewlines), forKey: "madeira.diagnostics.server")
            status = "Log server saved."
        } catch { status = error.localizedDescription }
    }
    func send(previous: Bool, label: String) async {
        guard !busy else { return }
        busy = true; status = "Preparing log…"; reportID = nil
        defer { busy = false }
        do {
            let url = try endpoint()
            let token = key.trimmingCharacters(in: .whitespacesAndNewlines)
            guard !token.isEmpty else { throw SupportError.message("Connect your log server first: tap Download iPad server configuration, sign in in Safari, download the configuration, then use Import server configuration here.") }
            guard !token.contains("\r"), !token.contains("\n") else { throw SupportError.message("Invalid upload key.") }
            try DiagnosticKeychain.save(token)
            UserDefaults.standard.set(server, forKey: "madeira.diagnostics.server")
            let cleanLabel = String(decoding: label.trimmingCharacters(in: .whitespacesAndNewlines).utf16.prefix(200), as: UTF16.self)
            if pending?.previous != previous || pending?.label != cleanLabel {
                if let old = pending { try? FileManager.default.removeItem(at: old.url) }
                pending = nil
            }
            if pending == nil {
                let source = LogStore.shared.fileForUpload(previous: previous)
                var metadata = LogStore.shared.metadataForUpload(previous: previous)
                let formatter = ISO8601DateFormatter()
                let since = (metadata["testStartedAt"] ?? metadata["startedAt"]).flatMap { formatter.date(from: $0) }
                let before = previous
                    ? LogStore.shared.metadataForUpload(previous: false)["startedAt"].flatMap { formatter.date(from: $0) }
                    : Date()
                let drive = LibraryModel.drive
                let snapshot = try await Task.detached(priority: .utility) {
                    let logs: [DiagnosticCompanionLogs.Source]
                    if let since, let before { logs = DiagnosticCompanionLogs.find(in: drive, since: since, before: before) }
                    else { logs = [] }
                    return try DiagnosticSnapshot.create(from: source, companions: logs)
                }.value
                metadata["label"] = cleanLabel.isEmpty ? "Game test" : cleanLabel
                metadata["source"] = previous ? "previous" : "current"
                metadata["originalBytes"] = String(snapshot.originalBytes)
                metadata["truncated"] = snapshot.truncated ? "true" : "false"
                metadata["companionLogs"] = String(snapshot.companionCount)
                pending = (snapshot.url, UUID().uuidString.lowercased(), metadata, previous, cleanLabel)
            }
            guard let report = pending else { return }
            var request = URLRequest(url: url)
            request.httpMethod = "POST"; request.timeoutInterval = 120
            request.setValue("text/plain; charset=utf-8", forHTTPHeaderField: "Content-Type")
            request.setValue("Bearer \(token)", forHTTPHeaderField: "OAI-Sites-Authorization")
            request.setValue(report.id, forHTTPHeaderField: "X-Madeira-Report-ID")
            request.setValue(try JSONSerialization.data(withJSONObject: report.metadata).base64EncodedString(), forHTTPHeaderField: "X-Madeira-Metadata")
            status = "Sending log…"
            let (data, response) = try await session.upload(for: request, fromFile: report.url)
            guard let http = response as? HTTPURLResponse, (200...299).contains(http.statusCode),
                  let result = try? JSONSerialization.jsonObject(with: data) as? [String: Any], result["id"] as? String == report.id else {
                let code = (response as? HTTPURLResponse)?.statusCode ?? 0
                throw SupportError.message(code == 401 || code == 403 ? "The server rejected the upload key. Check the key and try again." : "Upload failed (HTTP \(code)). Your log is still on this iPad; tap Send to retry.")
            }
            reportID = report.id; status = "Log sent. Give this report ID to Codex."
            try? FileManager.default.removeItem(at: report.url); pending = nil
        } catch { status = error.localizedDescription }
    }
}

/// Structured breadcrumbs, at launch/first frame/exit and at most every 10 s.
enum DiagnosticEvents {
    private static var sampledAt = Date.distantPast
    private static var lastFrames: UInt64 = 0
    private static var sessionStartFrames: UInt64 = 0
    static func phase(_ phase: String) {
        LogStore.shared.log("[diagnostics] phase=\(phase) uptime=\(String(format: "%.3f", ProcessInfo.processInfo.systemUptime)) thermal=\(ProcessInfo.processInfo.thermalState.rawValue)")
    }
    static func begin(_ entry: LibraryEntry) {
        LogStore.shared.recordTest(entry)
        sampledAt = Date(); lastFrames = madeira_frame_count()
        sessionStartFrames = lastFrames
        phase("launch")
        LogStore.shared.log("[diagnostics] bits=\(entry.bits) api=\(entry.graphicsAPI ?? "unknown") resolution=\(entry.resolution) display=\(entry.displayMode.rawValue) controller=\(entry.controllerMode ?? "default") fpsMode=\(entry.effectiveFPSMode) cpuChoice=\(entry.cpuCount.map(String.init) ?? "auto") unityProfile=\(entry.unityOptimizations != false ? 1 : 0)")
        var settings: [String: String] = [:]
        for key in ["pool", "vram-mb", "swap-mb", "env.MADEIRA_SWAP_PRESSURE", "inproc-sync", "env.MADEIRA_FASTSYNC",
                    "env.MADEIRA_RUNTIME_PROFILERS", "env.MADEIRA_MIP_CLAMP_AUTO",
                    "env.MADEIRA_MEMORY_CENSUS",
                    "env.DXMT_CENSUS_THROTTLE", "env.DXMT_WSI_MODE_TABLE",
                    "env.DXMT_WSI_MONITOR_IDENTITY",
                    "env.MADEIRA_UNITY_RESOLUTION",
                    "env.MADEIRA_UNITY_STARTUP_SYNC", "env.DXMT_COMPILER_THREADS",
                    "env.MADEIRA_CTX_FRAME",
                    "env.MADEIRA_MULTI_GAME", "env.DXMT_IOS_CACHE_DIR",
                    "env.DXMT_SHADER_CACHE", "env.DXMT_CACHE_STATS"] {
            settings[key] = String((MadeiraConfig.get(key) ?? "default").prefix(160))
        }
        if let data = try? JSONSerialization.data(withJSONObject: settings, options: .sortedKeys),
           let line = String(data: data, encoding: .utf8) { LogStore.shared.log("[diagnostics-settings] \(line)") }
    }
    static func sample() {
        // One counter snapshot and task_info every 10 s, with no thread
        // suspension or VM-map walks. Keep it in ordinary gameplay reports.
        let now = Date(), elapsed = now.timeIntervalSince(sampledAt)
        guard elapsed >= 10 else { return }
        let frames = madeira_frame_count()
        var info = task_vm_info_data_t()
        var count = mach_msg_type_number_t(MemoryLayout<task_vm_info_data_t>.size / MemoryLayout<natural_t>.size)
        let result = withUnsafeMutablePointer(to: &info) {
            $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) {
                task_info(mach_task_self_, task_flavor_t(TASK_VM_INFO), $0, &count)
            }
        }
        let fps = frames >= lastFrames ? Double(frames - lastFrames) / elapsed : 0
        var screenW: Int32 = 0, screenH: Int32 = 0
        winios_screen_size(&screenW, &screenH)
        // Before the first present the layer still has its seed size. Do not
        // report that as the game's render resolution.
        let drawable = frames > sessionStartFrames ? MetalHostView.shared.metalLayer.drawableSize : .zero
        let cpu = getenv("MADEIRA_CPU_COUNT").map { String(cString: $0) } ?? "auto"
        let input = HardwareInput.shared
        LogStore.shared.log("[diagnostics] sample seconds=\(String(format: "%.1f", elapsed)) fps=\(String(format: "%.1f", fps)) footprintMB=\(result == KERN_SUCCESS ? info.phys_footprint / 1048576 : 0) thermal=\(ProcessInfo.processInfo.thermalState.rawValue) monitor=\(screenW)x\(screenH) drawable=\(Int(drawable.width))x\(Int(drawable.height)) fpsMode=\(madeira_get_vsync_locked()) cpuReported=\(cpu) pointerRequested=\(input.pointerLocked ? 1 : 0) pointerCaptured=\(input.pointerCaptured ? 1 : 0) mousePath=\(input.mousePath.rawValue)")
        sampledAt = now; lastFrames = frames
    }
}
