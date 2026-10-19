import Foundation
import CryptoKit

// Only maintainer-audited, immutable official packages enter this catalogue.
// Neither a PE import nor a downloaded manifest can provide a download URL.
struct RuntimeCatalog: Codable, Sendable {
    struct File: Codable, Sendable {
        var name: String; var bits: Int; var sha256: String; var bytes: Int
        var kind: String?
        func matches(_ url: URL) -> Bool { kind == "manifest" || RuntimeFiles.matches(url, bits: bits) }
    }
    struct Package: Codable, Sendable {
        var id: String; var family: String; var bits: Int
        var url: String; var sha256: String; var bytes: Int; var files: [File]
        var title: String { family == "directx" ? "DirectX legacy components" : "Microsoft Visual C++ " + family.dropFirst(2) }
    }
    var revision: Int
    var packages: [Package]
    static func load(_ bundle: URL) throws -> Self {
        let catalog = try JSONDecoder().decode(Self.self, from: Data(contentsOf: bundle.appendingPathComponent("RuntimeCatalog.json")))
        let core: Set<String> = ["ntdll.dll", "kernel32.dll", "kernelbase.dll", "user32.dll", "gdi32.dll", "win32u.dll", "msvcrt.dll", "ucrtbase.dll", "mscoree.dll", "d3d11.dll", "dxgi.dll", "d3d12.dll", "xinput1_4.dll"]
        guard catalog.revision == 1, catalog.packages.count <= 64,
              catalog.packages.allSatisfy({ package in
                  package.bytes > 0 && package.bytes <= 128 * 1024 * 1024 && package.sha256.count == 64 &&
                  !package.files.isEmpty && package.files.count <= 512 && package.files.allSatisfy { file in
                      file.name == file.name.lowercased() && file.name.utf8.count <= 128 &&
                      file.name.allSatisfy { $0.isASCII && ($0.isLetter || $0.isNumber || "._-".contains($0)) } &&
                      !file.name.hasPrefix("api-ms-") && !file.name.hasPrefix("ext-ms-") && !core.contains(file.name) &&
                      (file.name.hasSuffix(".dll") || (file.kind == "manifest" && file.name.hasSuffix(".manifest"))) &&
                      [32, 64].contains(file.bits) && file.bytes > 0 && file.bytes <= 128 * 1024 * 1024 && file.sha256.count == 64
                  }
              }) else { throw RuntimePreparationError.message("The application runtime catalogue is invalid.") }
        return catalog
    }
    func package(name: String, bits: Int) -> Package? {
        packages.first { $0.files.contains { $0.name == name && $0.bits == bits } }
    }
}

enum RuntimePreparationError: LocalizedError {
    case message(String)
    var errorDescription: String? { if case .message(let text) = self { return text }; return nil }
}

enum RuntimeFiles {
    static let manager = FileManager.default
    static func inside(_ file: URL, _ root: URL) -> Bool {
        let path = file.resolvingSymlinksInPath().standardizedFileURL.path
        let base = root.resolvingSymlinksInPath().standardizedFileURL.path
        return path.hasPrefix(base + "/")
    }
    static func hash(_ file: URL) throws -> String {
        let handle = try FileHandle(forReadingFrom: file); defer { try? handle.close() }
        var hash = SHA256()
        while let data = try handle.read(upToCount: 1024 * 1024), !data.isEmpty { hash.update(data: data) }
        return hash.finalize().map { String(format: "%02x", $0) }.joined()
    }
    static func key(_ text: String) -> String {
        SHA256.hash(data: Data(text.utf8)).map { String(format: "%02x", $0) }.joined()
    }
    static func machine(_ file: URL) -> UInt16? { DockInstallers.machine(file) }
    static func matches(_ file: URL, bits: Int, bundled: Bool = false) -> Bool {
        guard let machine = machine(file) else { return false }
        return bits == 32 ? machine == 0x14c : (machine == 0x8664 || (bundled && [0xaa64, 0xa641].contains(machine)))
    }
    static func named(_ name: String, in folder: URL) -> URL? {
        // Wine is case-insensitive even on a case-sensitive APFS volume.
        guard let files = try? manager.contentsOfDirectory(at: folder, includingPropertiesForKeys: nil) else { return nil }
        return files.first { $0.lastPathComponent.lowercased() == name }
    }
    static func make(_ folder: URL) throws { try manager.createDirectory(at: folder, withIntermediateDirectories: true) }
}

struct RuntimeStamp: Codable, Equatable, Sendable {
    var path: String; var size: UInt64; var modified: Date; var inode: UInt64
    static func read(_ url: URL) -> Self? {
        guard let a = try? RuntimeFiles.manager.attributesOfItem(atPath: url.path),
              let date = a[.modificationDate] as? Date else { return nil }
        return Self(path: url.path, size: (a[.size] as? NSNumber)?.uint64Value ?? 0,
                    modified: date, inode: (a[.systemFileNumber] as? NSNumber)?.uint64Value ?? 0)
    }
    var valid: Bool { Self.read(URL(fileURLWithPath: path)) == self }
}

struct RuntimeDependency: Codable, Hashable, Sendable {
    enum Load: String, Codable, Hashable, Sendable {
        case startup, delayed, unverified
        var title: String {
            switch self {
            case .startup: return "Startup import"
            case .delayed: return "Delay-loaded import"
            case .unverified: return "Module may load at runtime"
            }
        }
    }
    var name: String; var folder: String; var bits: Int; var wrongArchitecture: Bool
    var source: String? = nil
    var load: Load? = nil
}
struct RuntimeReport: Codable, Sendable {
    var missing: [RuntimeDependency] = []
    var stamps: [RuntimeStamp] = []
    var scanned = 0
    var truncated = false
    var managed = false
    var modernDotNet = false
    var installed: [String] = []
    var cached = false
    var selectionPending: Bool?
    // Old receipts without provenance stay conservative until rescanned.
    var startupMissing: [RuntimeDependency] { missing.filter { $0.load == nil || $0.load == .startup } }
    var additionalReferences: [RuntimeDependency] { missing.filter { $0.load == .delayed || $0.load == .unverified } }
    var summary: String {
        if selectionPending == true { return "Steam will select this game's program at launch. Choose a program in Game details › Steam › Program to inspect its runtimes." }
        if modernDotNet { return "This game requests a separate .NET Desktop runtime. Review its runtimeconfig.json requirements." }
        if !startupMissing.isEmpty {
            let names = Array(Set(startupMissing.map(\.name))).sorted().prefix(8).joined(separator: ", ")
            return "Unresolved startup DLLs: " + names + ". Review the requesting programs below."
        }
        if !additionalReferences.isEmpty {
            return "Startup imports checked. Additional DLL references need review; they do not by themselves confirm a launch failure."
        }
        if truncated { return "Known runtimes are ready. This large game needs further checks when it starts." }
        return installed.isEmpty ? "Runtime check complete" : "Installed and verified: " + installed.joined(separator: ", ")
    }
}

enum RuntimeScanner {
    /// Inspect actual module search directories, not a union of all DLL names
    /// found anywhere in the game. Optional plugins are included, bounded.
    static func scan(executable: URL, bits: Int, drive: URL, bundle: URL) throws -> RuntimeReport {
        guard RuntimeFiles.inside(executable, drive), RuntimeFiles.matches(executable, bits: bits) else {
            throw RuntimePreparationError.message("The executable is missing or has an unsupported architecture.")
        }
        let root = executable.deletingLastPathComponent()
        var report = RuntimeReport(), modules = [executable], watched = Set<URL>()
        let walker = RuntimeFiles.manager.enumerator(at: root, includingPropertiesForKeys: [.isSymbolicLinkKey, .isDirectoryKey], options: [.skipsHiddenFiles])
        watched.insert(root)
        var count = 0
        while let file = walker?.nextObject() as? URL {
            count += 1
            if count > 20_000 { report.truncated = true; break }
            guard RuntimeFiles.inside(file, drive), let values = try? file.resourceValues(forKeys: [.isSymbolicLinkKey, .isDirectoryKey]), values.isSymbolicLink != true else { walker?.skipDescendants(); continue }
            if values.isDirectory == true {
                if [".git", "redist", "_commonredist", "__installer"].contains(file.lastPathComponent.lowercased()) { walker?.skipDescendants() }
                else { watched.insert(file) }
                continue
            }
            if file.lastPathComponent.lowercased() == executable.deletingPathExtension().lastPathComponent.lowercased() + ".runtimeconfig.json" {
                watched.insert(file)
                if let size = (try? RuntimeFiles.manager.attributesOfItem(atPath: file.path)[.size]) as? NSNumber,
                   size.intValue <= 128 * 1024, let data = try? Data(contentsOf: file),
                   let json = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any],
                   let options = json["runtimeOptions"] as? [String: Any] {
                    // Self-contained .NET games list includedFrameworks. A
                    // framework/frameworks reference requires a shared host.
                    let shared = options["framework"] != nil || (options["frameworks"] as? [Any])?.isEmpty == false
                    let local = ["hostfxr.dll", "coreclr.dll"].allSatisfy { name in
                        RuntimeFiles.named(name, in: root).map { RuntimeFiles.matches($0, bits: bits) } == true
                    }
                    report.modernDotNet = shared && !local
                }
            }
            if file.pathExtension.lowercased() == "dll", modules.count < 512, RuntimeFiles.matches(file, bits: bits) { modules.append(file) }
            else if modules.count >= 512 { report.truncated = true }
        }
        let system = drive.appendingPathComponent(bits == 32 ? "windows/syswow64" : "windows/system32")
        // Do not count ARM64-only files as a replacement for the i386 farm.
        let farms = (bits == 32 ? ["i386-windows"] : ["arm64ec-windows", "aarch64-windows", "x86_64-vcruntime"]).map { bundle.appendingPathComponent($0) }
        watched.formUnion(farms)
        watched.insert(bundle)
        let available = farms.flatMap { (try? RuntimeFiles.manager.contentsOfDirectory(at: $0, includingPropertiesForKeys: nil)) ?? [] }
        let bundleByName = Dictionary(grouping: available, by: { $0.lastPathComponent.lowercased() })
        var directoryCache: [String: [String: URL]] = [:]
        func localFile(_ name: String, _ folder: URL) -> URL? {
            if directoryCache[folder.path] == nil {
                let files = (try? RuntimeFiles.manager.contentsOfDirectory(at: folder, includingPropertiesForKeys: nil)) ?? []
                directoryCache[folder.path] = Dictionary(files.map { ($0.lastPathComponent.lowercased(), $0) }, uniquingKeysWith: { first, _ in first })
            }
            return directoryCache[folder.path]?[name]
        }
        watched.insert(system)
        var missing: [(URL, RuntimeDependency)] = []
        var startupEdges: [URL: Set<URL>] = [:]
        let moduleSet = Set(modules)
        for module in modules {
            watched.insert(module); report.scanned += 1
            for imported in LibraryModel.moduleImports(module) {
                let name = imported.name
                if name == "mscoree.dll" { report.managed = true }
                if name.hasPrefix("api-ms-") || name.hasPrefix("ext-ms-") { continue }
                let folders = [module.deletingLastPathComponent(), root, system]
                var satisfied = false, wrong = false
                for folder in folders {
                    watched.insert(folder)
                    // Before Wine starts, system32/syswow64 can still contain
                    // links staged for the preceding session's architecture.
                    // The launch bridge rebuilds these from the app's farms.
                    // A game-local DLL still shadows them and is checked first.
                    if folder == system, let shipped = bundleByName[name]?.first(where: {
                        RuntimeFiles.matches($0, bits: bits, bundled: true)
                    }) {
                        watched.insert(shipped); satisfied = true; break
                    }
                    if let file = localFile(name, folder) {
                        watched.insert(file)
                        if RuntimeFiles.matches(file, bits: bits, bundled: folder == system) {
                            satisfied = true
                            if !imported.delayed, moduleSet.contains(file) { startupEdges[module, default: []].insert(file) }
                        }
                        else { wrong = folder != system }
                        break // An incompatible local DLL shadows later search paths.
                    }
                }
                if !satisfied && !wrong {
                    satisfied = bundleByName[name]?.contains { RuntimeFiles.matches($0, bits: bits, bundled: true) } == true
                }
                if !satisfied {
                    let source = String(module.path.dropFirst(root.path.count + 1))
                    missing.append((module, RuntimeDependency(name: name, folder: module.deletingLastPathComponent().path,
                        bits: bits, wrongArchitecture: wrong, source: source, load: imported.delayed ? .delayed : .unverified)))
                }
            }
        }
        // Only ordinary imports reachable from the chosen EXE establish a
        // startup dependency. Unity plugins and delay imports can be loaded
        // later; their presence on disk does not prove that code path is used.
        var startup: Set<URL> = [executable], pending = [executable]
        while let module = pending.popLast() {
            for child in startupEdges[module] ?? [] where startup.insert(child).inserted { pending.append(child) }
        }
        report.missing = Set(missing.map { module, dependency in
            var result = dependency
            if result.load != .delayed, startup.contains(module) { result.load = .startup }
            return result
        }).sorted { ($0.source ?? $0.folder, $0.name, $0.load?.rawValue ?? "") < ($1.source ?? $1.folder, $1.name, $1.load?.rawValue ?? "") }
        report.stamps = watched.compactMap(RuntimeStamp.read).sorted { $0.path < $1.path }
        return report
    }
}

private final class CabinetMembers {
    var files: [URL] = []
}

enum RuntimeUnpacker {
    /// Select files by their pinned uncompressed hash, not by a CAB filename.
    /// Anonymous MSI payloads and the ARM64 payload in an x64 installer are
    /// handled without trusting arbitrary paths, installer scripts or filenames.
    static func extract(_ source: URL, package: RuntimeCatalog.Package, to output: URL) throws {
        let staging = output.deletingLastPathComponent().appendingPathComponent(UUID().uuidString)
        try RuntimeFiles.make(staging); defer { try? RuntimeFiles.manager.removeItem(at: staging) }
        try RuntimeFiles.make(output)
        let expected = Dictionary(grouping: package.files, by: \.sha256)
        var found = Set<String>(), archives = 0, total = 0
        func visit(_ file: URL, depth: Int) throws {
            guard depth <= 4 else { throw RuntimePreparationError.message("Runtime archive nesting limit exceeded.") }
            let data = try Data(contentsOf: file, options: .mappedIfSafe)
            total += data.count
            guard total <= 768 * 1024 * 1024 else { throw RuntimePreparationError.message("Runtime extraction size limit exceeded.") }
            if depth < 3, data.prefix(2) == Data("MZ".utf8) || (package.family == "vc80" && data.prefix(4) == Data([0xd0, 0xcf, 0x11, 0xe0])) {
                var best: Range<Int>?, cursor = 0
                var candidates: [Range<Int>] = []
                while cursor + 36 <= data.count, let range = data.range(of: Data("MSCF".utf8), in: cursor..<data.count) {
                    let offset = range.lowerBound
                    if offset + 36 <= data.count {
                        let size = (0..<4).reduce(0) { $0 | Int(data[offset + 8 + $1]) << ($1 * 8) }
                        if size >= 36, size <= data.count - offset {
                            let candidate = offset..<(offset + size)
                            candidates.append(candidate)
                            if size > (best?.count ?? 0) { best = candidate }
                        }
                    }
                    cursor = offset + 4
                }
                if let best {
                    if data.prefix(4) == Data([0xd0, 0xcf, 0x11, 0xe0]) {
                        for range in candidates {
                            let cab = staging.appendingPathComponent(UUID().uuidString + ".cab")
                            try data.subdata(in: range).write(to: cab); try visit(cab, depth: depth + 1)
                        }
                        return
                    }
                    let cab = staging.appendingPathComponent(UUID().uuidString + ".cab")
                    try data.subdata(in: best).write(to: cab); try visit(cab, depth: depth + 1); return
                }
                if depth == 0 { throw RuntimePreparationError.message("Official runtime package has no supported cabinet.") }
            }
            if data.prefix(4) == Data("MSCF".utf8) {
                archives += 1
                guard archives <= 256 else { throw RuntimePreparationError.message("Runtime archive count limit exceeded.") }
                let folder = staging.appendingPathComponent("cab\(archives)"); try RuntimeFiles.make(folder)
                let members = CabinetMembers()
                let context = Unmanaged.passUnretained(members).toOpaque()
                let result = madeira_cab_extract(file.path, folder.path, { context, _, path, _ in
                    guard let context, let path else { return 1 }
                    Unmanaged<CabinetMembers>.fromOpaque(context).takeUnretainedValue().files.append(URL(fileURLWithPath: String(cString: path)))
                    return 0
                }, context)
                guard result == 0 else { throw RuntimePreparationError.message("Could not verify the runtime cabinet (\(result)).") }
                for child in members.files { try visit(child, depth: depth + 1) }
            } else if let pins = expected[try RuntimeFiles.hash(file)] {
                for pin in pins where pin.matches(file) && pin.bytes == data.count {
                    let name = "\(pin.bits)-\(pin.name)"
                    if found.insert(name).inserted { try RuntimeFiles.manager.copyItem(at: file, to: output.appendingPathComponent(name)) }
                }
            }
        }
        try visit(source, depth: 0)
        guard found.count == package.files.count else { throw RuntimePreparationError.message("The official package did not contain every expected runtime file.") }
    }
}

private final class RuntimeDownloadPolicy: NSObject, URLSessionDownloadDelegate, @unchecked Sendable {
    let maximumBytes: Int64
    init(maximumBytes: Int) { self.maximumBytes = Int64(maximumBytes) }
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didFinishDownloadingTo location: URL) {}
    func urlSession(_ session: URLSession, downloadTask: URLSessionDownloadTask, didWriteData bytesWritten: Int64,
                    totalBytesWritten: Int64, totalBytesExpectedToWrite: Int64) {
        if totalBytesWritten > maximumBytes || totalBytesExpectedToWrite > maximumBytes { downloadTask.cancel() }
    }
    static func allows(_ url: URL?) -> Bool {
        guard let url, url.scheme == "https", url.user == nil, url.password == nil, url.port == nil else { return false }
        return ["download.microsoft.com", "download.visualstudio.microsoft.com"].contains(url.host?.lowercased() ?? "")
    }
    func urlSession(_ session: URLSession, task: URLSessionTask, willPerformHTTPRedirection response: HTTPURLResponse,
                    newRequest request: URLRequest, completionHandler: @escaping (URLRequest?) -> Void) {
        completionHandler(Self.allows(request.url) ? request : nil)
    }
}

actor RuntimePreparer {
    static let shared = RuntimePreparer()
    struct Receipt: Codable {
        var version: Int; var bundle: String; var catalog: String; var report: RuntimeReport
        var installed: [String: String]
    }
    private var jobs: [String: Task<RuntimeReport, Error>] = [:]
    private var packageJobs: [String: Task<URL, Error>] = [:]
    // One install transaction at a time, even when a download suspends an actor.
    private var installTail: Task<Void, Never>?

    func prepare(executable: URL, bits: Int, drive: URL, bundle: URL, cache: URL,
                 force: Bool = false, progress: @escaping @Sendable (String) async -> Void) async throws -> RuntimeReport {
        let key = RuntimeFiles.key(executable.path + "|\(bits)")
        if let job = jobs[key] { return try await job.value }
        let previous = installTail
        let job = Task { [self] in
            if let previous { await previous.value }
            return try await self.perform(executable: executable, bits: bits, drive: drive, bundle: bundle, cache: cache, key: key, force: force, progress: progress)
        }
        jobs[key] = job
        installTail = Task { _ = try? await job.value }
        defer { jobs[key] = nil }
        return try await job.value
    }

    private func package(_ pin: RuntimeCatalog.Package, cache: URL) async throws -> URL {
        if let job = packageJobs[pin.id] { return try await job.value }
        let job = Task<URL, Error> {
            let root = cache.appendingPathComponent("packages/" + pin.sha256)
            let extracted = root.appendingPathComponent("files")
            try RuntimeFiles.make(root)
            func verified() -> Bool {
                pin.files.allSatisfy { file in
                    let url = extracted.appendingPathComponent("\(file.bits)-\(file.name)")
                    return (try? RuntimeFiles.hash(url)) == file.sha256 && file.matches(url)
                }
            }
            if verified() { return extracted }
            let archive = root.appendingPathComponent("official.exe")
            if (try? RuntimeFiles.hash(archive)) != pin.sha256 {
                guard let url = URL(string: pin.url), RuntimeDownloadPolicy.allows(url) else { throw RuntimePreparationError.message("The runtime source is not approved.") }
                let policy = RuntimeDownloadPolicy(maximumBytes: pin.bytes)
                let configuration = URLSessionConfiguration.ephemeral
                configuration.timeoutIntervalForRequest = 45; configuration.timeoutIntervalForResource = 600
                configuration.httpCookieStorage = nil; configuration.urlCredentialStorage = nil
                let session = URLSession(configuration: configuration, delegate: policy, delegateQueue: nil)
                defer { session.invalidateAndCancel() }
                let (temporary, response) = try await session.download(from: url)
                defer { try? RuntimeFiles.manager.removeItem(at: temporary) }
                guard let http = response as? HTTPURLResponse, http.statusCode == 200, RuntimeDownloadPolicy.allows(http.url),
                      (try RuntimeFiles.manager.attributesOfItem(atPath: temporary.path)[.size] as? NSNumber)?.intValue == pin.bytes,
                      try RuntimeFiles.hash(temporary) == pin.sha256 else { throw RuntimePreparationError.message("Runtime download failed its size or SHA-256 check. Try again.") }
                if RuntimeFiles.manager.fileExists(atPath: archive.path) { try RuntimeFiles.manager.removeItem(at: archive) }
                try RuntimeFiles.manager.moveItem(at: temporary, to: archive)
            }
            if RuntimeFiles.manager.fileExists(atPath: extracted.path) { try RuntimeFiles.manager.removeItem(at: extracted) }
            do { try RuntimeUnpacker.extract(archive, package: pin, to: extracted) }
            catch { try? RuntimeFiles.manager.removeItem(at: extracted); throw error }
            guard verified() else { throw RuntimePreparationError.message("Extracted runtime verification failed.") }
            // Extracted payloads are the reusable cache; discard the large installer.
            try? RuntimeFiles.manager.removeItem(at: archive)
            return extracted
        }
        packageJobs[pin.id] = job; defer { packageJobs[pin.id] = nil }
        return try await job.value
    }

    private func perform(executable: URL, bits: Int, drive: URL, bundle: URL, cache: URL, key: String,
                         force: Bool, progress: @escaping @Sendable (String) async -> Void) async throws -> RuntimeReport {
        try RuntimeFiles.make(cache.appendingPathComponent("games"))
        let receiptURL = cache.appendingPathComponent("games/" + key + ".json")
        let catalog = try RuntimeCatalog.load(bundle)
        let catalogHash = try RuntimeFiles.hash(bundle.appendingPathComponent("RuntimeCatalog.json"))
        let old = (try? Data(contentsOf: receiptURL)).flatMap { try? JSONDecoder().decode(Receipt.self, from: $0) }
        if !force, let old, old.version == 3, old.bundle == bundle.path, old.catalog == catalogHash,
           !old.report.truncated,
           old.report.stamps.allSatisfy(\.valid),
           old.installed.allSatisfy({ RuntimeFiles.inside(URL(fileURLWithPath: $0.key), drive) && (try? RuntimeFiles.hash(URL(fileURLWithPath: $0.key))) == $0.value }) {
            var report = old.report; report.cached = true; return report
        }
        await progress("Checking required components…")
        var report = try RuntimeScanner.scan(executable: executable, bits: bits, drive: drive, bundle: bundle)
        if report.managed {
            await progress("Preparing .NET compatibility…")
            let result = madeira_runtime_prepare_mono(bundle.path, drive.deletingLastPathComponent().path)
            guard result == 0 else { throw RuntimePreparationError.message("The bundled .NET compatibility runtime could not be prepared (\(result)).") }
        }
        var installed = old?.installed.filter { RuntimeFiles.inside(URL(fileURLWithPath: $0.key), drive) } ?? [:]
        var preparedPackages = Set<String>()
        var created: [(URL, String)] = []
        do {
            // A runtime can introduce another family (e.g. a native plugin's
            // VC bridge). Resolve the new imports after every install wave.
            for _ in 0..<16 {
              var packages = [String: RuntimeCatalog.Package]()
              for missing in report.missing where !missing.wrongArchitecture {
                  if let pin = catalog.package(name: missing.name, bits: missing.bits) { packages[pin.id] = pin }
              }
              if packages.isEmpty { break }
              for pin in packages.values.sorted(by: { $0.id < $1.id }) {
                await progress("Preparing \(pin.title)…")
                let source = try await package(pin, cache: cache)
                let requests = report.missing.filter { !$0.wrongArchitecture && catalog.package(name: $0.name, bits: $0.bits)?.id == pin.id }
                for request in requests {
                    let target = URL(fileURLWithPath: request.folder)
                    guard RuntimeFiles.inside(target, drive) else { throw RuntimePreparationError.message("The runtime target changed. Check the game folder again.") }
                    var pending = [request.name] + pin.files.filter { $0.kind == "manifest" && $0.bits == request.bits }.map(\.name)
                    var handled = Set<String>()
                    while !pending.isEmpty {
                        let name = pending.removeFirst()
                        guard handled.insert(name).inserted,
                              let file = pin.files.first(where: { $0.name == name && $0.bits == request.bits }) else { continue }
                        if RuntimeFiles.named(file.name, in: target) != nil { continue }
                        let payload = source.appendingPathComponent("\(file.bits)-\(file.name)")
                        if file.kind != "manifest" { pending += LibraryModel.importNames(payload) }
                        let destination = target.appendingPathComponent(file.name)
                        let temporary = cache.appendingPathComponent("pending-" + UUID().uuidString)
                        defer { try? RuntimeFiles.manager.removeItem(at: temporary) }
                        try RuntimeFiles.manager.copyItem(at: payload, to: temporary)
                        guard try RuntimeFiles.hash(temporary) == file.sha256, file.matches(temporary) else { throw RuntimePreparationError.message("The runtime copy did not pass verification.") }
                        // Both locations live in Documents on the same volume.
                        // A killed app cannot leave a partial DLL in the game.
                        try RuntimeFiles.manager.moveItem(at: temporary, to: destination)
                        created.append((destination, file.sha256))
                        guard try RuntimeFiles.hash(destination) == file.sha256, file.matches(destination) else { throw RuntimePreparationError.message("The installed runtime did not pass verification.") }
                        installed[destination.path] = file.sha256
                    }
                }
                preparedPackages.insert(pin.id)
              }
              await progress("Verifying components…")
              report = try RuntimeScanner.scan(executable: executable, bits: bits, drive: drive, bundle: bundle)
            }
            await progress("Verifying components…")
            if report.managed {
                let mono = drive.appendingPathComponent("windows/mono/mono-2.0")
                for name in ["bin/libmono-2.0-x86.dll", "lib/mono/4.5/mscorlib.dll"] {
                    if let stamp = RuntimeStamp.read(mono.appendingPathComponent(name)) { report.stamps.append(stamp) }
                }
            }
            // Dependencies of the newly installed runtime are included in the
            // post-install scans. Never report success while a known gap remains.
            let unresolved = report.missing.filter { !$0.wrongArchitecture && catalog.package(name: $0.name, bits: $0.bits) != nil }
            if !unresolved.isEmpty { throw RuntimePreparationError.message("Additional runtime dependencies remain: " + Array(Set(unresolved.map(\.name))).sorted().joined(separator: ", ")) }
            report.installed = preparedPackages.sorted()
            // Remove stale receipts after an application/game legitimately replaced a DLL.
            installed = installed.filter { (try? RuntimeFiles.hash(URL(fileURLWithPath: $0.key))) == $0.value }
            let receipt = Receipt(version: 3, bundle: bundle.path, catalog: catalogHash, report: report, installed: installed)
            try JSONEncoder().encode(receipt).write(to: receiptURL, options: .atomic)
            return report
        } catch {
            for (file, hash) in created.reversed() where (try? RuntimeFiles.hash(file)) == hash { try? RuntimeFiles.manager.removeItem(at: file) }
            throw error
        }
    }
}
