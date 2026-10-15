// SPDX-License-Identifier: GPL-3.0-or-later
// Madeira Converter Exception: see LICENSE-EXCEPTION.md
// Chunk assembly follows Legendary's downloader/mp/manager.py and workers.py.

import Foundation
import Combine
import Darwin

struct EpicInstalledGame: Codable {
    var game: EpicGame
    var appName: String
    var installDir: String
    var buildVersion: String
    var launchExe: String
    var launchCommand: String
    var prereqName: String
    var prereqPath: String
    var prereqArgs: String
}

@MainActor final class EpicInstaller: ObservableObject {
    static let shared = EpicInstaller()
    enum State: Equatable { case queued, active, paused, failed(String), installed }
    struct Install: Equatable {
        var state: State
        var progress = SteamDownloadProgress()
        var download: SteamOwnedLibrary.Download? {
            let state: SteamOwnedLibrary.Download.State
            switch self.state {
            case .queued: state = .queued
            case .active: state = .active
            case .paused: state = .paused
            case .failed(let message): state = .failed(message)
            case .installed: return nil
            }
            return SteamOwnedLibrary.Download(state: state, progress: progress)
        }
    }
    struct Pending: Codable { var game: EpicGame; var installDir: String }
    struct Store: Codable {
        var installed: [String: EpicInstalledGame] = [:]
        var pending: [String: Pending] = [:]
    }
    @Published private(set) var installs: [String: Install] = [:]
    @Published private(set) var installed: [String: EpicInstalledGame] = [:]
    @Published private(set) var ready = false
    @Published var error: String?
    private var pending: [String: Pending] = [:]
    private var queue: [String] = []
    private var active: (id: String, task: Task<Void, Never>)?
    private var removing = Set<String>()
    private var backgroundPaused = Set<String>()
    private var saving: Task<Void, Never>?
    private let worker = EpicInstallWorker(drive: LibraryModel.drive)
    private lazy var background = SteamDownloadBackground(source: "Epic", suffix: "epic")

    private init() {
        Task {
            do {
                let store = try await worker.load()
                installed = store.installed; pending = store.pending
                for (id, record) in installed {
                    if await worker.exists(record.installDir + "/" + record.launchExe) {
                        installs[id] = Install(state: .installed)
                        register(record)
                    } else { installed.removeValue(forKey: id) }
                }
                for id in pending.keys { installs[id] = Install(state: .paused) }
                attachBackground()
                ready = true
            } catch { self.error = "Could not read Epic installs: " + error.localizedDescription }
        }
    }
    private func attachBackground() {
        background.attach(active: { [weak self] in self?.hasActiveDownload == true },
                          pause: { [weak self] in self?.pauseForBackground() },
                          resume: { [weak self] in self?.resumeAfterBackground() })
    }
    private var hasActiveDownload: Bool { active != nil || !queue.isEmpty }
    /// A download queued, running, paused or failed. A finished install stays in
    /// `installs` with the state .installed, so "has an entry" is not "downloading".
    func isDownloading(_ appName: String) -> Bool { installs[appName]?.download != nil }
    func entry(_ appName: String) -> LibraryEntry? {
        guard installed[appName] != nil else { return nil }
        return LibraryModel.shared.entries.first { $0.epicAppName == appName }
    }
    private func register(_ record: EpicInstalledGame) {
        var entry = LibraryModel.shared.entries.first { $0.epicAppName == record.appName }
            ?? LibraryEntry(title: record.game.title, relativePath: "", bits: 0)
        entry.epicAppName = record.appName
        entry.epicLaunchCommand = record.launchCommand
        entry.relativePath = record.installDir + "/" + record.launchExe
        if let executable = try? LibraryModel.inspect(LibraryModel.drive.appendingPathComponent(entry.relativePath)) {
            entry.bits = executable.bits
            entry.graphicsAPI = executable.graphicsAPI
            entry.gameEngine = executable.gameEngine
        }
        entry.epicArtworkURL = record.game.artworkURL
        entry.epicHeroURL = record.game.heroURL
        LibraryModel.shared.save(entry)
    }
    private func persist() {
        let store = Store(installed: installed, pending: pending), previous = saving
        saving = Task {
            await previous?.value
            do { try await worker.save(store) }
            catch { self.error = "Could not save Epic installs: " + error.localizedDescription }
        }
    }
    func install(_ game: EpicGame) {
        guard ready, !removing.contains(game.appName), installed[game.appName] == nil else { return }
        let id = game.appName
        if installs[id]?.state == .active || installs[id]?.state == .queued { return }
        if pending[id] == nil {
            let safe = SteamInstallFiles.safeFolderName(game.title)
            // The app name suffix prevents two sanitized titles sharing an uninstall directory.
            let suffix = EpicSHA1.hash(Data(id.utf8)).prefix(6).map { String(format: "%02x", $0) }.joined()
            pending[id] = Pending(game: game, installDir: "Program Files/Epic Games/\(safe.isEmpty ? "Game" : safe)-\(suffix)")
        } else if game.catalogItemId != nil { pending[id]?.game = game }
        installs[id] = Install(state: .queued, progress: installs[id]?.progress ?? SteamDownloadProgress())
        queue.append(id); persist(); startNext()
    }
    func pause(_ id: String) {
        backgroundPaused.remove(id)
        queue.removeAll { $0 == id }
        if active?.id == id { active?.task.cancel() }
        if installed[id] == nil, installs[id] != nil { installs[id]?.state = .paused }
        persist()
    }
    private func pauseForBackground() {
        let ids = queue + (active.map { [$0.id] } ?? [])
        for id in ids { pause(id); backgroundPaused.insert(id) }
    }
    private func resumeAfterBackground() {
        let ids = backgroundPaused; backgroundPaused.removeAll()
        for id in ids { if let game = pending[id]?.game { install(game) } }
    }
    func cancel(_ id: String) { remove(id, uninstall: false) }
    func uninstall(_ id: String) { remove(id, uninstall: true) }
    private func remove(_ id: String, uninstall: Bool) {
        guard ready, !removing.contains(id), let dir = pending[id]?.installDir ?? installed[id]?.installDir else { return }
        if uninstall, LibraryModel.shared.activeEntry?.epicAppName == id {
            error = "End the game session before uninstalling."; return
        }
        removing.insert(id); pause(id)
        let task = active?.id == id ? active?.task : nil
        Task {
            await task?.value
            do {
                try await worker.remove(dir)
                pending.removeValue(forKey: id)
                if uninstall {
                    installed.removeValue(forKey: id)
                    for entry in LibraryModel.shared.entries where entry.epicAppName == id && !LibraryModel.shared.isPrivate(entry) { LibraryModel.shared.remove(entry.id) }
                }
                installs.removeValue(forKey: id); persist()
            } catch { installs[id] = Install(state: .failed("Could not remove Epic files: " + error.localizedDescription)) }
            removing.remove(id)
        }
    }
    private func startNext() {
        guard active == nil, !queue.isEmpty else { return }
        let id = queue.removeFirst()
        guard let job = pending[id], !removing.contains(id) else { startNext(); return }
        installs[id]?.state = .active
        background.downloadStarted(appID: 0, name: job.game.title)
        let task = Task {
            var outcome: SteamDownloadBackground.Outcome = .paused
            do {
                guard let catalog = job.game.catalogItemId else {
                    EpicLibrary.shared.refresh()
                    throw EpicContentError.invalid("Refresh your Epic library, then try installing again.")
                }
                let token = try await EpicAuth.shared.validAccessToken()
                let record = try await worker.run(job, catalog: catalog, token: token) { progress in
                    await MainActor.run {
                        guard self.active?.id == id, self.installs[id]?.state == .active else { return }
                        self.installs[id]?.progress = progress
                        self.background.progress(progress)
                    }
                }
                try Task.checkCancellation()
                installed[id] = record; pending.removeValue(forKey: id)
                register(record); installs[id]?.state = .installed; persist()
                outcome = .completed
            } catch {
                if Task.isCancelled {
                    if installs[id]?.state != .queued { installs[id]?.state = .paused }
                } else {
                    let message = (error as? EpicAuthError)?.message ?? error.localizedDescription
                    installs[id]?.state = .failed(message)
                    outcome = .failed(message)
                    LogStore.shared.log("[epic-install] failure completed-bytes=\(installs[id]?.progress.doneBytes ?? 0) total-bytes=\(installs[id]?.progress.totalBytes ?? 0) reason=\(message)", level: .error)
                }
            }
            active = nil
            background.downloadEnded(appID: 0, name: job.game.title, outcome: outcome, queueEmpty: queue.isEmpty)
            startNext()
        }
        active = (id, task)
    }
}

/// All hashing, decompression and file writes stay off the main actor.
private actor EpicInstallWorker {
    private let drive: URL
    private let storeURL: URL

    init(drive: URL, support: URL = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]) {
        self.drive = drive; storeURL = support.appendingPathComponent("epic-installs.json")
    }
    func load() throws -> EpicInstaller.Store {
        guard FileManager.default.fileExists(atPath: storeURL.path) else { return EpicInstaller.Store() }
        return try JSONDecoder().decode(EpicInstaller.Store.self, from: Data(contentsOf: storeURL))
    }
    func save(_ store: EpicInstaller.Store) throws {
        try FileManager.default.createDirectory(at: storeURL.deletingLastPathComponent(), withIntermediateDirectories: true)
        try JSONEncoder().encode(store).write(to: storeURL, options: .atomic)
    }
    func exists(_ path: String) -> Bool { (try? safeURL(path, under: drive)).map { FileManager.default.fileExists(atPath: $0.path) } ?? false }
    private func safeURL(_ path: String, under root: URL) throws -> URL {
        let path = path.replacingOccurrences(of: "\\", with: "/")
        let parts = path.split(separator: "/", omittingEmptySubsequences: false)
        guard !path.isEmpty, !parts.contains(where: { $0.isEmpty || $0 == "." || $0 == ".." || $0.contains(":") || $0.contains("\0") }) else {
            throw EpicContentError.invalid("Unsafe path in Epic content.")
        }
        let base = root.resolvingSymlinksInPath().standardizedFileURL
        let url = base.appendingPathComponent(path).standardizedFileURL
        // Reject existing symlinks too: writes must never escape through an old install.
        var cursor = base
        for part in parts {
            cursor.appendPathComponent(String(part))
            if let type = try? FileManager.default.attributesOfItem(atPath: cursor.path)[.type] as? FileAttributeType,
               type == .typeSymbolicLink { throw EpicContentError.invalid("Epic install path contains a symbolic link.") }
        }
        guard url.path.hasPrefix(base.path + "/") else { throw EpicContentError.invalid("Unsafe Epic install path.") }
        return url
    }
    private func folder(_ relative: String) throws -> URL {
        guard relative.hasPrefix("Program Files/Epic Games/"), relative.split(separator: "/").count == 3 else {
            throw EpicContentError.invalid("Invalid Epic install directory.")
        }
        return try safeURL(relative, under: drive)
    }
    func remove(_ relative: String) throws {
        let url = try folder(relative)
        if FileManager.default.fileExists(atPath: url.path) { try FileManager.default.removeItem(at: url) }
    }
    func run(_ job: EpicInstaller.Pending, catalog: String, token: String,
             progress: @escaping @Sendable (SteamDownloadProgress) async -> Void) async throws -> EpicInstalledGame {
        let (manifest, bases) = try await EpicManifest.fetch(namespace: job.game.namespace, catalogItemID: catalog,
                                                         appName: job.game.appName, token: token)
        guard let base = bases.first else { throw EpicContentError.invalid("Epic did not return a content server.") }
        return try await assemble(job, manifest: manifest, base: base, alternateBases: Array(bases.dropFirst()), progress: progress)
    }
    func assemble(_ job: EpicInstaller.Pending, manifest: EpicManifest, base: URL, alternateBases: [URL] = [],
                  fetch: @escaping @Sendable (URL, EpicManifest.Chunk) async throws -> Data = { url, chunk in
                      try await EpicManifest.download(URLRequest(url: url), limit: chunk.fileSize)
                  },
                  progress: @escaping @Sendable (SteamDownloadProgress) async -> Void) async throws -> EpicInstalledGame {
        try Task.checkCancellation()
        // The build's own AppName can differ from the library's (another internal name,
        // or other capitals) for the right game: the manifest came from that game's
        // assets endpoint and matched Epic's SHA-1, and Legendary does not compare them.
        // Logged, never fatal: refusing here stopped installs of games the account owns.
        if manifest.meta.appName.caseInsensitiveCompare(job.game.appName) != .orderedSame {
            LogStore.shared.log("[epic-install] manifest app name differs from the library's (accepted)")
        }
        let root = try folder(job.installDir), requestedLaunch = manifest.meta.launchExe.replacingOccurrences(of: "\\", with: "/")
        _ = try safeURL(requestedLaunch, under: root)
        if !manifest.meta.prereqPath.isEmpty { _ = try safeURL(manifest.meta.prereqPath, under: root) }
        guard let launchFile = manifest.files.first(where: { $0.filename.replacingOccurrences(of: "\\", with: "/").caseInsensitiveCompare(requestedLaunch) == .orderedSame }) else {
            throw EpicContentError.invalid("Epic manifest does not contain its launch executable.")
        }
        let launch = launchFile.filename.replacingOccurrences(of: "\\", with: "/")
        let stagingKey = EpicSHA1.hash(Data(job.game.appName.utf8)).map { String(format: "%02x", $0) }.joined()
        let staging = storeURL.deletingLastPathComponent().appendingPathComponent("Epic Staging/" + stagingKey)
        // Only unfinished data lives here. A killed process cannot leave large orphan files in the game.
        if FileManager.default.fileExists(atPath: staging.path) { try FileManager.default.removeItem(at: staging) }
        try FileManager.default.createDirectory(at: staging, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: staging) }
        var chunks: [String: EpicManifest.Chunk] = [:]
        for chunk in manifest.chunks {
            guard chunks[chunk.guid] == nil, chunk.windowSize > 0, chunk.windowSize <= EpicChunk.maximumSize,
                  chunk.fileSize > 0, chunk.fileSize <= EpicChunk.maximumSize + 1024 else {
                throw EpicContentError.invalid("Invalid Epic chunk description.")
            }
            _ = try chunk.path(featureLevel: manifest.meta.featureLevel)
            chunks[chunk.guid] = chunk
        }
        var names = Set<String>(), remaining: [String: Int] = [:], files: [EpicManifest.File] = []
        var required: UInt64 = 0, reused = 0
        for file in manifest.files {
            try Task.checkCancellation()
            let url = try safeURL(file.filename, under: root)
            guard file.symlink.isEmpty, names.insert(url.path.lowercased()).inserted else {
                throw EpicContentError.invalid("Duplicate or symbolic-link file in Epic manifest.")
            }
            for part in file.parts {
                guard let chunk = chunks[part.guid], part.offset <= chunk.windowSize, part.size <= chunk.windowSize - part.offset else {
                    throw EpicContentError.invalid("Epic file references an invalid chunk part.")
                }
            }
            let size = (try? url.resourceValues(forKeys: [.fileSizeKey]))?.fileSize
            if size.map(UInt64.init) == file.size, (try? EpicSHA1.file(url)) == file.sha { reused += 1; continue }
            files.append(file); required += file.size
            for part in file.parts { remaining[part.guid, default: 0] += 1 }
        }
        try FileManager.default.createDirectory(at: drive, withIntermediateDirectories: true)
        let values = try drive.resourceValues(forKeys: [.volumeAvailableCapacityForImportantUsageKey, .volumeAvailableCapacityKey])
        // Some volumes return zero for the reclaimable-space estimate despite having free blocks.
        let capacity = max(values.volumeAvailableCapacityForImportantUsage ?? 0, Int64(values.volumeAvailableCapacity ?? 0))
        guard UInt64(max(0, capacity)) >= required + 128 * 1024 * 1024 else {
            throw EpicContentError.invalid("Not enough free space for this Epic game (\(required / 1024 / 1024) MB plus working space required).")
        }
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        let total = remaining.keys.reduce(UInt64(0)) { $0 + UInt64(chunks[$1]!.fileSize) }
        var status = SteamDownloadProgress(phase: .downloading, totalBytes: total)
        var fetched = Set<String>(), received: UInt64 = 0
        let start = Date()
        LogStore.shared.log("[epic-install] start files=\(manifest.files.count) reused=\(reused) chunks=\(remaining.count) download-bytes=\(total) write-bytes=\(required)")
        await progress(status)
        // One download supplies every file slice that references a chunk. Scatter
        // verified data into private staging files, rather than evicting an 8 MB
        // cache and fetching shared chunks again as later files are assembled.
        struct Destination { let file: Int; let offset: UInt64; let part: EpicManifest.Part }
        var destinations: [String: [Destination]] = [:], order: [String] = []
        var outstanding = files.map { $0.parts.count }
        for (index, file) in files.enumerated() {
            guard file.size <= UInt64(Int64.max) else { throw EpicContentError.invalid("Epic file is too large.") }
            var offset: UInt64 = 0
            for part in file.parts {
                if destinations[part.guid] == nil { order.append(part.guid) }
                destinations[part.guid, default: []].append(Destination(file: index, offset: offset, part: part))
                offset += UInt64(part.size)
            }
        }
        // At most 64 MB of decoded chunks in flight, and at most 16 open files.
        // A normal 1 MB Epic chunk allows 16 concurrent requests. Large chunks
        // reduce the width instead of growing memory with the game's size.
        let largest = order.compactMap { chunks[$0]?.windowSize }.max() ?? 1
        let concurrency = min(16, max(1, 64 * 1024 * 1024 / largest - 1))
        let source = EpicInstallChunkSource(bases: [base] + alternateBases, featureLevel: manifest.meta.featureLevel, fetch: fetch)
        var handles: [Int: FileHandle] = [:], openOrder: [Int] = [], created = Set<Int>()
        defer { for handle in handles.values { try? handle.close() } }
        func temporary(_ index: Int) -> URL { staging.appendingPathComponent("\(index).part") }
        func writable(_ index: Int) throws -> FileHandle {
            openOrder.removeAll { $0 == index }; openOrder.append(index)
            if let handle = handles[index] { return handle }
            if handles.count >= 16, let oldest = openOrder.first {
                try handles.removeValue(forKey: oldest)?.close()
                openOrder.removeFirst()
            }
            let url = temporary(index)
            if created.insert(index).inserted {
                guard FileManager.default.createFile(atPath: url.path, contents: nil) else {
                    throw EpicContentError.invalid("Could not stage an Epic game file.")
                }
            }
            let handle = try FileHandle(forWritingTo: url)
            handles[index] = handle
            try handle.truncate(atOffset: files[index].size)
            return handle
        }
        func finish(_ index: Int) throws {
            try Task.checkCancellation()
            let handle = try writable(index), source = temporary(index)
            guard try EpicSHA1.file(source) == files[index].sha else {
                throw EpicContentError.invalid("Epic file SHA-1 mismatch. Resume to retry.")
            }
            try handle.synchronize(); try handle.close()
            handles.removeValue(forKey: index); openOrder.removeAll { $0 == index }
            try Task.checkCancellation()
            let target = try safeURL(files[index].filename, under: root)
            try FileManager.default.createDirectory(at: target.deletingLastPathComponent(), withIntermediateDirectories: true)
            // Each file replaces the previous version only after its whole hash
            // passes. A pause keeps finished files; unfinished staging is removed.
            guard rename(source.path, target.path) == 0 else {
                throw EpicContentError.invalid("Could not finalize an Epic game file.")
            }
        }
        for index in files.indices where outstanding[index] == 0 { try finish(index) }
        var lastProgress = ProcessInfo.processInfo.systemUptime
        LogStore.shared.log("[epic-install-v2] pipeline workers=\(concurrency) unique-chunks=\(order.count) decoded-budget=64MB hash=CommonCrypto")
        try await withThrowingTaskGroup(of: (String, Data, Int).self) { group in
            var next = 0
            func enqueue(_ guid: String) {
                let chunk = chunks[guid]!
                group.addTask {
                    let (data, received) = try await source.download(chunk)
                    return (guid, data, received)
                }
            }
            while next < min(concurrency, order.count) { enqueue(order[next]); next += 1 }
            while let (guid, data, bytes) = try await group.next() {
                try Task.checkCancellation()
                for destination in destinations[guid] ?? [] {
                    let handle = try writable(destination.file), part = destination.part
                    try data.withUnsafeBytes { buffer in
                        var written = 0
                        while written < part.size {
                            try Task.checkCancellation()
                            let result = pwrite(handle.fileDescriptor, buffer.baseAddress!.advanced(by: part.offset + written),
                                                part.size - written, off_t(destination.offset + UInt64(written)))
                            if result < 0, errno == EINTR { continue }
                            guard result > 0 else { throw EpicContentError.invalid("Could not write an Epic game file (errno \(errno)).") }
                            written += result
                        }
                    }
                    outstanding[destination.file] -= 1
                    if outstanding[destination.file] == 0 { try finish(destination.file) }
                }
                received += UInt64(bytes); fetched.insert(guid); status.doneBytes += UInt64(bytes)
                // Replace the completed request immediately. A slow request no
                // longer holds an entire batch or a small-file boundary idle.
                if next < order.count { enqueue(order[next]); next += 1 }
                let now = ProcessInfo.processInfo.systemUptime
                if now - lastProgress >= 0.2 || next == order.count && fetched.count == order.count {
                    lastProgress = now
                    status.bytesPerSecond = Double(received) / max(0.1, Date().timeIntervalSince(start))
                    await progress(status)
                }
            }
        }
        status.phase = .finishing; await progress(status)
        LogStore.shared.log("[epic-install] finish files=\(manifest.files.count) reused=\(reused) chunks=\(fetched.count) received-bytes=\(received) written-bytes=\(required)")
        return EpicInstalledGame(game: job.game, appName: job.game.appName, installDir: job.installDir,
                                 buildVersion: manifest.meta.buildVersion, launchExe: launch, launchCommand: manifest.meta.launchCommand,
                                 prereqName: manifest.meta.prereqName, prereqPath: manifest.meta.prereqPath, prereqArgs: manifest.meta.prereqArgs)
    }
}

/// Health is scoped to one install, and contains no account credentials. Use
/// the other official bases when a CDN times out, refuses or corrupts a chunk.
private actor EpicInstallChunkSource {
    let bases: [URL]
    let featureLevel: Int
    let fetch: @Sendable (URL, EpicManifest.Chunk) async throws -> Data
    private var failures: [URL: Int] = [:]

    init(bases: [URL], featureLevel: Int, fetch: @escaping @Sendable (URL, EpicManifest.Chunk) async throws -> Data) {
        self.bases = bases; self.featureLevel = featureLevel; self.fetch = fetch
    }

    func download(_ chunk: EpicManifest.Chunk) async throws -> (Data, Int) {
        let path = try chunk.path(featureLevel: featureLevel)
        var last: Error = EpicContentError.invalid("Epic did not return a content server.")
        for attempt in 0..<4 {
            try Task.checkCancellation()
            let healthy = bases.filter { failures[$0, default: 0] < 4 }
            let pool = healthy.isEmpty ? bases : healthy
            guard !pool.isEmpty else { throw last }
            let base = pool[attempt % pool.count]
            do {
                let bytes = try await fetch(base.appendingPathComponent(path), chunk)
                try Task.checkCancellation()
                let data = try EpicChunk.parse(bytes, expected: chunk)
                failures[base] = 0
                return (data, bytes.count)
            } catch is CancellationError { throw CancellationError() }
            catch {
                try Task.checkCancellation()
                failures[base, default: 0] += 1
                LogStore.shared.log("[epic-install-v2] chunk retry attempt=\(attempt + 1) host=\(base.host ?? "unknown")")
                last = error
            }
        }
        throw last
    }
}
