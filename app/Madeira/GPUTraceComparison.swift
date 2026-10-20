import Foundation
import SwiftUI
import UIKit

// BEGIN GPU_TRACE_DROP_POLICY
/// Observes ordinary gameplay outside the capture interval. Capturing itself
/// stalls frames, so exclude a 30-second recovery interval from drop detection.
struct GPUTraceDropPolicy {
    private(set) var reference: Double?
    private var stable: [Double] = []
    private var low = 0
    private var resumeAt: Double = 0
    mutating func resume(after now: Double, delay: Double = 5) {
        stable.removeAll(); low = 0; resumeAt = max(resumeAt, now + delay)
    }
    mutating func capturedReference(at now: Double) { resume(after: now, delay: 30) }
    mutating func setManualReference(_ fps: Double) -> Bool {
        guard fps.isFinite, fps > 0 else { return false }
        reference = fps; stable.removeAll(); low = 0; return true
    }
    mutating func sample(fps: Double, at now: Double, eligible: Bool) -> String? {
        guard eligible, now >= resumeAt, fps.isFinite, fps > 0 else { stable.removeAll(); low = 0; return nil }
        if let reference {
            low = fps < reference * 0.8 ? low + 1 : 0
            if low >= 5 { low = 0; return "slow" }
        } else {
            guard fps >= 50 else { stable.removeAll(); return nil }
            stable.append(fps); if stable.count > 5 { stable.removeFirst() }
            if stable.count == 5, let minimum = stable.min(), let maximum = stable.max(), minimum >= maximum * 0.85 {
                reference = stable.reduce(0, +) / 5; stable.removeAll(); return "smooth"
            }
        }
        return nil
    }
}
// END GPU_TRACE_DROP_POLICY

@MainActor final class GPUTraceComparison: ObservableObject {
    static let shared = GPUTraceComparison()
    @Published private(set) var message = ""
    @Published private(set) var recording = false
    @Published private(set) var busy = false
    @Published private(set) var canCaptureSlow = false
    @Published private(set) var savedFolder: URL?
    @Published private(set) var savedComparisons: [GPUTraceSavedComparison] = []
    @Published private(set) var findingSaved = false
    @Published private(set) var exporting = false
    private var activeFolder: URL?
    private var savedLookup = UUID()
    private var exportTask: Task<URL, Error>?
    private var exportingFolder: URL?
    private var cachedExport: (folder: URL, zip: URL)?
    private let documents: URL
    private let preferences: UserDefaults
    @Published private(set) var eligibleSession = false
    @Published private var gameVisible = false
    private var menuOpen = false
    @Published private var foreground = true
    private var timer: Timer?
    private var policy = GPUTraceDropPolicy()
    private var singleSample = false
    private var previousFrames: UInt64 = 0
    private var previousTime: Double = 0
    private var fpsMode: Int32 = 0
    private var lastCapture: String?
    private var polling = false
    private var generation = UUID()
    private var history: [[String: Any]] = []
    private var manifest: [String: Any] = [:]
    private var observers: [NSObjectProtocol] = []
    @Published private var manualScheduled = false
    var canCaptureReference: Bool { recording && policy.reference == nil && lastCapture == nil && !manualScheduled }
    var canStartRecording: Bool { eligibleSession && gameVisible && foreground && !recording && !busy && !exporting }
    var recordHint: String? {
        if !eligibleSession { return "Open REPLACED or Liar's Bar, enter gameplay, then capture the current scene or record an FPS drop." }
        if !gameVisible { return "Wait for the game to display a frame, then enter the scene you want to compare." }
        return nil
    }

    init(documents: URL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0],
         preferences: UserDefaults = .standard) {
        self.documents = documents.resolvingSymlinksInPath()
        self.preferences = preferences
        let center = NotificationCenter.default
        observers.append(center.addObserver(forName: UIApplication.willResignActiveNotification, object: nil, queue: .main) { _ in
            Task { @MainActor in self.foreground = false; self.interrupt("Recording stopped because Madeira left the foreground. Start a new comparison after returning.") }
        })
        observers.append(center.addObserver(forName: UIApplication.didBecomeActiveNotification, object: nil, queue: .main) { _ in
            Task { @MainActor in self.foreground = true }
        })
    }
    func session(title: String?, executable: String? = nil) {
        interrupt("The game session ended. Completed traces are still available.")
        let identity = ((title ?? "") + " " + (executable ?? ""))
            .filter { $0.isLetter || $0.isNumber }.lowercased()
        eligibleSession = identity.contains("replaced") || identity.contains("liarsbar")
        gameVisible = false
    }
    func gameDidBecomeVisible() { if eligibleSession { gameVisible = true } }
    func setMenuOpen(_ open: Bool) {
        menuOpen = open
        if open, lastCapture != nil { interrupt("The game menu opened during capture. Start a new comparison from gameplay.") }
        if !open { policy.resume(after: ProcessInfo.processInfo.systemUptime) }
    }
    @discardableResult func start(singleSample: Bool = false) -> Bool {
        guard canStartRecording else { return false }
        self.singleSample = singleSample
        generation = UUID(); let token = generation
        busy = true; message = "Preparing GPU comparison…"
        activeFolder = nil; manifest = [:]
        let metadata = LogStore.shared.metadataForUpload(previous: false)
        let documents = self.documents
        Task {
            do {
                let folder = try await Task.detached(priority: .utility) { () -> URL in
                    let root = documents.appendingPathComponent("GPUTraces", isDirectory: true)
                    try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
                    guard root.resolvingSymlinksInPath().standardizedFileURL == root.standardizedFileURL else {
                        throw SupportError.message("GPUTraces must be a folder inside Madeira, not a symbolic link.")
                    }
                    let space = try root.resourceValues(forKeys: [.volumeAvailableCapacityForImportantUsageKey]).volumeAvailableCapacityForImportantUsage ?? 0
                    guard space > 2 * 1024 * 1024 * 1024 else { throw SupportError.message("Free at least 2 GB before recording GPU traces.") }
                    let folder = root.appendingPathComponent(token.uuidString.lowercased(), isDirectory: true)
                    try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: false)
                    return folder
                }.value
                guard token == generation, eligibleSession, foreground else { return }
                activeFolder = folder
                manifest = ["format": "madeira-gpu-comparison-v1", "context": metadata, "createdAt": ISO8601DateFormatter().string(from: Date()),
                    "sampleMode": singleSample ? "single-scene" : "comparison",
                    "captureFrames": 2, "note": "Capture perturbs timing. Compare command structure and pre-capture FPS; replay timings are not original iPad hardware timings."]
                policy = GPUTraceDropPolicy(); policy.resume(after: ProcessInfo.processInfo.systemUptime)
                fpsMode = madeira_get_vsync_locked(); history = []; lastCapture = nil
                previousFrames = madeira_frame_count(); previousTime = ProcessInfo.processInfo.systemUptime
                recording = true; busy = false; canCaptureSlow = false
                message = "Resume gameplay in the scene to compare. Waiting for 5 stable seconds at 50 FPS or more."
                timer?.invalidate(); timer = Timer.scheduledTimer(withTimeInterval: 1, repeats: true) { _ in
                    Task { @MainActor in self.tick() }
                }
                persist()
                LogStore.shared.log("[gpu-trace-pair] armed folder=GPUTraces/\(folder.lastPathComponent) mode=\(fpsMode) sample-mode=\(singleSample ? "single-scene" : "comparison") baseline-min-fps=\(singleSample ? 0 : 50)")
                if singleSample { scheduleManual(reference: true) }
            } catch { if token == generation { busy = false; message = error.localizedDescription; restoreSaved() } }
        }
        return true
    }
    private func tick() {
        guard recording, !polling else { return }
        if madeira_get_vsync_locked() != fpsMode {
            interrupt("FPS mode changed. Start a new comparison with one FPS mode for both samples."); return
        }
        let now = ProcessInfo.processInfo.systemUptime, frames = madeira_frame_count()
        let elapsed = now - previousTime
        let fps = elapsed > 0 && frames >= previousFrames ? Double(frames - previousFrames) / elapsed : 0
        previousTime = now; previousFrames = frames
        let usable = foreground && !menuOpen && elapsed >= 0.5 && elapsed < 2.5
        if usable {
            history.append(["uptime": now, "fps": fps, "thermal": ProcessInfo.processInfo.thermalState.rawValue,
                            "lowPower": ProcessInfo.processInfo.isLowPowerModeEnabled])
            if history.count > 180 { history.removeFirst() }
        }
        if lastCapture != nil {
            polling = true; let token = generation
            GPUTraceNative.poll { [self] result in
                guard token == generation, recording, let kind = lastCapture else { return }
                polling = false
                if result.phase == 4 || result.phase == 5 {
                    manifest[kind] = captureRecord(kind, result)
                    lastCapture = nil
                    if result.phase == 5 { interrupt(result.error); return }
                    if kind == "smooth" && !singleSample {
                        policy.capturedReference(at: ProcessInfo.processInfo.systemUptime); canCaptureSlow = true
                        message = "Reference sample saved. Keep playing in the same scene; a sustained 20% FPS drop triggers the second sample, or use Capture low FPS now."
                    } else {
                        recording = false; canCaptureSlow = false; timer?.invalidate(); timer = nil
                        message = singleSample
                            ? "Current scene saved. Open Madeira's menu and tap Share GPU trace ZIP."
                            : "Both GPU samples saved. Open Madeira's menu and tap Share GPU trace ZIP."
                    }
                    persist()
                    if let folder = activeFolder {
                        GPUTraceFiles.snapshot(log: LogStore.shared.fileForUpload(previous: false), folder: folder)
                        if kind == "slow" || singleSample { restoreSaved(prefer: folder) }
                    }
                    LogStore.shared.log("[gpu-trace-pair] sample=\(kind) complete=1 buffers=\(result.buffers) frames=\(result.frames)")
                }
            }
        } else if !manualScheduled, let kind = policy.sample(fps: fps, at: now, eligible: usable) { capture(kind) }
    }
    private func captureRecord(_ kind: String, _ result: GPUTraceNative.Status) -> [String: Any] {
        ["file": "\(kind).gputrace", "complete": result.captureComplete, "buffers": result.buffers,
         "frames": result.frames, "captureUptime": result.started, "error": result.error,
         "stageTimingsFile": "\(kind)-stages.json", "stageTimingsRole": "following-frame",
         "finishedAt": ISO8601DateFormatter().string(from: Date())]
    }
    private func capture(_ kind: String) {
        guard recording, !busy, lastCapture == nil, let folder = activeFolder else { return }
        lastCapture = kind; let token = generation
        manifest["\(kind)BeforeCapture"] = history.suffix(15).map { $0 }
        if let reference = policy.reference { manifest["referenceFPS"] = reference }
        manifest["fpsMode"] = fpsMode
        message = "Recording two complete \(kind == "smooth" ? "reference" : "low FPS") frames… A brief pause is expected."
        persist()
        let file = folder.appendingPathComponent("\(kind).gputrace")
        GPUTraceNative.request(file) { [self] accepted, error in
            guard token == generation, recording else { return }
            if !accepted { interrupt(error.isEmpty ? "Could not request a GPU trace." : error) }
        }
    }
    func captureSlow() {
        scheduleManual(reference: false)
    }
    func captureReference() { scheduleManual(reference: true) }
    private func scheduleManual(reference: Bool) {
        guard recording, lastCapture == nil, !manualScheduled,
              reference ? canCaptureReference : canCaptureSlow else { return }
        manualScheduled = true
        message = "Resume gameplay. The sample will be taken in five seconds."
        // The caller closes the menu. Give the game five seconds to resume.
        let token = generation, scheduled = ProcessInfo.processInfo.systemUptime
        Task {
            try? await Task.sleep(for: .seconds(5))
            guard token == generation else { return }
            manualScheduled = false
            guard foreground, !menuOpen, recording else { return }
            if reference {
                let samples = history.compactMap { item -> Double? in
                    guard let uptime = item["uptime"] as? Double, uptime >= scheduled else { return nil }
                    return item["fps"] as? Double
                }
                guard samples.count >= 3, policy.setManualReference(samples.reduce(0, +) / Double(samples.count)) else {
                    message = "No gameplay FPS sample was available. Resume the game and try again."; return
                }
                manifest["referenceSelection"] = "manual-current-scene"
            }
            capture(reference ? "smooth" : "slow")
        }
    }
    func interrupt(_ reason: String) {
        generation = UUID()
        guard recording || busy else { return }
        recording = false; busy = true; canCaptureSlow = false; polling = false; manualScheduled = false
        timer?.invalidate(); timer = nil; message = reason
        manifest["interrupted"] = reason
        let token = generation
        // Metal may already have saved both frames while the following-frame
        // timing pass (or its file write) still makes poll report recording.
        // Read completion after serial cancellation before publishing metadata.
        GPUTraceNative.cancel { [self] result in
            guard token == generation else { return }
            var completedRecording = false
            if let kind = lastCapture {
                manifest[kind] = captureRecord(kind, result)
                if result.captureComplete {
                    LogStore.shared.log("[gpu-trace-pair] sample=\(kind) complete=1 buffers=\(result.buffers) frames=\(result.frames) recording-interrupted=1")
                    if kind == "slow" || singleSample {
                        completedRecording = true
                        message = "GPU sample saved. Share the completed recording using Share GPU trace ZIP."
                    }
                }
            }
            lastCapture = nil; persist()
            if let folder = activeFolder {
                GPUTraceFiles.snapshot(log: LogStore.shared.fileForUpload(previous: false), folder: folder)
                restoreSaved(prefer: completedRecording ? folder : savedFolder)
            }
            busy = false
        }
    }
    private func persist() {
        guard let folder = activeFolder else { return }
        // Serial writes preserve phase ordering when stop follows start quickly.
        GPUTraceFiles.writeManifest(manifest, folder: folder)
    }
    func restoreSaved(prefer: URL? = nil) {
        let lookup = UUID(); savedLookup = lookup; findingSaved = true
        let root = documents.appendingPathComponent("GPUTraces", isDirectory: true)
        Task {
            let pairs = await GPUTraceFiles.savedComparisons(in: root)
            guard lookup == savedLookup else { return }
            savedComparisons = pairs; findingSaved = false
            let preferred = prefer?.lastPathComponent ?? savedFolder?.lastPathComponent ??
                preferences.string(forKey: "madeira.gpuTrace.latest")
            chooseSaved(GPUTraceSavedComparison.select(from: pairs, preferredID: preferred))
        }
    }
    func selectSaved(_ id: String) { chooseSaved(savedComparisons.first { $0.id == id }) }
    private func chooseSaved(_ comparison: GPUTraceSavedComparison?) {
        savedFolder = comparison?.folder
        if let comparison { preferences.set(comparison.id, forKey: "madeira.gpuTrace.latest") }
    }
    func export(folder: URL) async throws -> URL {
        if let cachedExport, cachedExport.folder == folder, FileManager.default.fileExists(atPath: cachedExport.zip.path) {
            return cachedExport.zip
        }
        if let exportTask, exportingFolder == folder { return try await exportTask.value }
        guard !recording, !busy, !exporting else { throw SupportError.message("Stop recording before sharing the saved GPU comparison.") }
        exporting = true; exportingFolder = folder; message = "Preparing GPU trace ZIP…"
        let directory = documents.appendingPathComponent("GPUTraceExports", isDirectory: true)
        let task = Task { try await GPUTraceFiles.export(folder: folder, directory: directory) }; exportTask = task
        defer { exporting = false; exportTask = nil; exportingFolder = nil }
        do {
            let url = try await task.value
            cachedExport = (folder, url)
            message = "ZIP ready. A copy is also available in Files → Madeira → GPUTraceExports."
            return url
        } catch {
            message = error.localizedDescription; throw error
        }
    }
    func exportFinished(_ status: String) { message = status }
}

/// Enqueue synchronously from main so request/cancel/new-session cannot reorder
/// across detached tasks. Metal stop/serialization never blocks UIKit's thread.
enum GPUTraceNative {
    typealias Status = (phase: Int, buffers: Int, frames: Int, started: Double, error: String, captureComplete: Bool)
    private static let queue = DispatchQueue(label: "madeira.gpu-trace.control", qos: .userInitiated)
    @MainActor static func request(_ file: URL, done: @escaping @MainActor (Bool, String) -> Void) {
        queue.async {
            let accepted = file.path.withCString { madeira_gpu_trace_request($0) } != 0
            let error = madeira_gpu_trace_error() ?? ""
            DispatchQueue.main.async { done(accepted, error) }
        }
    }
    private static func status() -> Status {
        let value = madeira_gpu_trace_status()
        return (Int(value.phase), Int(value.buffers), Int(value.frames), value.started,
                madeira_gpu_trace_error() ?? "", value.captureComplete != 0)
    }
    @MainActor static func poll(_ done: @escaping @MainActor (Status) -> Void) {
        queue.async {
            let result = status()
            DispatchQueue.main.async { done(result) }
        }
    }
    @MainActor static func cancel(_ done: @escaping @MainActor (Status) -> Void) {
        queue.async {
            madeira_gpu_trace_cancel()
            finishCancellation(done)
        }
    }
    private static func finishCancellation(_ done: @escaping @MainActor (Status) -> Void) {
        let result = status()
        if result.captureComplete && result.phase == 3 {
            // The cancelled timing sidecar writes on a utility queue. Keep
            // sharing disabled until it is ready alongside the raw document.
            queue.asyncAfter(deadline: .now() + .milliseconds(20)) { finishCancellation(done) }
        } else {
            DispatchQueue.main.async { done(result) }
        }
    }
}

enum GPUTraceFiles {
    private static let queue = DispatchQueue(label: "madeira.gpu-trace.files", qos: .utility)
    private static func isTraceResourceAlias(_ file: URL, in folder: URL) -> Bool {
        let root = folder.standardizedFileURL.path + "/"
        guard file.path.hasPrefix(root) else { return false }
        let relative = file.path.dropFirst(root.count).split(separator: "/")
        guard relative.count > 1, relative[0] == "smooth.gputrace" || relative[0] == "slow.gputrace",
              let name = try? FileManager.default.destinationOfSymbolicLink(atPath: file.path),
              !name.isEmpty, name != ".", name != "..", !name.contains("/"), !name.contains("\\") else { return false }
        let target = file.deletingLastPathComponent().appendingPathComponent(name)
        guard target.resolvingSymlinksInPath().standardizedFileURL == target.standardizedFileURL,
              let values = try? target.resourceValues(forKeys: [.isRegularFileKey, .isSymbolicLinkKey]) else { return false }
        return values.isRegularFile == true && values.isSymbolicLink != true
    }
    static func savedComparisons(in root: URL) async -> [GPUTraceSavedComparison] {
        await withCheckedContinuation { continuation in
            queue.async {
                let fm = FileManager.default
                guard root.resolvingSymlinksInPath().standardizedFileURL == root.standardizedFileURL,
                      let folders = try? fm.contentsOfDirectory(at: root, includingPropertiesForKeys: [.isDirectoryKey, .isSymbolicLinkKey]) else {
                    continuation.resume(returning: []); return
                }
                var pairs: [GPUTraceSavedComparison] = []
                for folder in folders {
                    guard UUID(uuidString: folder.lastPathComponent) != nil,
                          let values = try? folder.resourceValues(forKeys: [.isDirectoryKey, .isSymbolicLinkKey]),
                          values.isDirectory == true, values.isSymbolicLink != true,
                          let object = try? completeManifest(folder: folder) else { continue }
                    let context = object["context"] as? [String: Any]
                    let title = String((context?["gameTitle"] as? String ?? "GPU comparison").prefix(200))
                    let date = (object["createdAt"] as? String).flatMap { ISO8601DateFormatter().date(from: $0) } ?? .distantPast
                    // Directory enumeration can return URLs relative to a
                    // base URL. Keep a stable absolute URL for sharing/cache.
                    let absolute = URL(fileURLWithPath: folder.path, isDirectory: true)
                    pairs.append(GPUTraceSavedComparison(folder: absolute, title: title, created: date))
                }
                pairs.sort { $0.created == $1.created ? $0.id < $1.id : $0.created > $1.created }
                continuation.resume(returning: pairs)
            }
        }
    }
    private static func completeManifest(folder: URL) throws -> [String: Any] {
        let manifest = folder.appendingPathComponent("manifest.json")
        let values = try manifest.resourceValues(forKeys: [.isRegularFileKey, .isSymbolicLinkKey, .fileSizeKey])
        guard values.isRegularFile == true, values.isSymbolicLink != true, let size = values.fileSize, size <= 1024 * 1024 else {
            throw SupportError.message("GPU comparison metadata is invalid.")
        }
        let object = try JSONSerialization.jsonObject(with: Data(contentsOf: manifest)) as? [String: Any]
        guard let object, object["format"] as? String == "madeira-gpu-comparison-v1" else {
            throw SupportError.message("GPU comparison metadata is missing.")
        }
        let kinds: [String]
        switch object["sampleMode"] as? String {
        case nil, "comparison": kinds = ["smooth", "slow"]
        case "single-scene": kinds = ["smooth"]
        default: throw SupportError.message("GPU recording mode is not supported.")
        }
        for kind in kinds {
            let sample = object[kind] as? [String: Any]
            guard sample?["complete"] as? Bool == true, sample?["frames"] as? Int == 2,
                  sample?["file"] as? String == "\(kind).gputrace",
                  let trace = try? folder.appendingPathComponent("\(kind).gputrace").resourceValues(forKeys: [.isDirectoryKey, .isRegularFileKey, .isSymbolicLinkKey]),
                  trace.isSymbolicLink != true, trace.isDirectory == true || trace.isRegularFile == true else {
                throw SupportError.message("Finish the requested GPU samples before exporting the recording.")
            }
        }
        let log = try folder.appendingPathComponent("session.txt").resourceValues(forKeys: [.isRegularFileKey, .isSymbolicLinkKey])
        guard log.isRegularFile == true, log.isSymbolicLink != true else {
            throw SupportError.message("The captured session log is missing.")
        }
        return object
    }
    static func writeManifest(_ object: [String: Any], folder: URL) {
        guard let data = try? JSONSerialization.data(withJSONObject: object, options: [.prettyPrinted, .sortedKeys]) else { return }
        queue.async { do { try data.write(to: folder.appendingPathComponent("manifest.json"), options: .atomic) }
            catch { NSLog("[gpu-trace-pair] manifest save failed: %@", error.localizedDescription) } }
    }
    static func snapshot(log: URL, folder: URL) {
        queue.async {
            do {
                let snapshot = try DiagnosticSnapshot.create(from: log)
                defer { try? FileManager.default.removeItem(at: snapshot.url) }
                let target = folder.appendingPathComponent("session.txt")
                let data = try Data(contentsOf: snapshot.url, options: .mappedIfSafe)
                try data.write(to: target, options: .atomic)
            } catch { NSLog("[gpu-trace-pair] session snapshot failed: %@", error.localizedDescription) }
        }
    }
    static func export(folder: URL, directory: URL? = nil) async throws -> URL {
        try await withCheckedThrowingContinuation { continuation in
            queue.async {
                do {
                    let fm = FileManager.default
                    guard folder.resolvingSymlinksInPath().standardizedFileURL == folder.standardizedFileURL else {
                        throw SupportError.message("The GPU comparison folder must not be a symbolic link.")
                    }
                    _ = try completeManifest(folder: folder)
                    NSLog("[gpu-trace-pair] export-start pair=%@", folder.lastPathComponent)
                    let allowed: Set<String> = ["smooth.gputrace", "slow.gputrace", "manifest.json", "session.txt",
                                                "smooth-stages.json", "slow-stages.json"]
                    let children = try fm.contentsOfDirectory(at: folder, includingPropertiesForKeys: nil)
                    guard children.allSatisfy({ allowed.contains($0.lastPathComponent) }),
                          let walker = fm.enumerator(at: folder, includingPropertiesForKeys: [.isSymbolicLinkKey]) else {
                        throw SupportError.message("The comparison folder contains unexpected files.")
                    }
                    var count = 0
                    for case let file as URL in walker {
                        count += 1
                        let relative = String(file.path.dropFirst(folder.path.count + 1))
                        guard count <= 100_000 else {
                            throw SupportError.message("The comparison exceeds the 100,000-file limit at \(relative).")
                        }
                        if try file.resourceValues(forKeys: [.isSymbolicLinkKey]).isSymbolicLink == true,
                           !isTraceResourceAlias(file, in: folder) {
                            throw SupportError.message("The comparison contains an unsupported symbolic link: \(relative).")
                        }
                    }
                    let sessionLog = folder.appendingPathComponent("session.txt")
                    // Keep the log of the captured test after a restart or a
                    // different game launch. Never replace it with that run.
                    guard fm.fileExists(atPath: sessionLog.path) else {
                        throw SupportError.message("The captured session log is missing. Record a new comparison.")
                    }
                    // Own the ready ZIP before asking UIKit to hand it to Files
                    // or another app. Do not generate it inside an item provider.
                    let destination = directory ?? fm.temporaryDirectory
                    try fm.createDirectory(at: destination, withIntermediateDirectories: true)
                    let target = destination.appendingPathComponent("Madeira-GPU-\(folder.lastPathComponent)-\(UUID().uuidString).zip")
                    var coordinationError: NSError?, copyError: Error?, copied = false
                    NSFileCoordinator().coordinate(readingItemAt: folder, options: .forUploading, error: &coordinationError) { archive in
                        do { try fm.copyItem(at: archive, to: target); copied = true } catch { copyError = error }
                    }
                    if let error = coordinationError ?? (copyError as NSError?) { throw error }
                    guard copied else { throw SupportError.message("The GPU trace ZIP could not be created.") }
                    NSLog("[gpu-trace-pair] export-ready file=%@", target.lastPathComponent)
                    continuation.resume(returning: target)
                } catch {
                    NSLog("[gpu-trace-pair] export-failed: %@", error.localizedDescription)
                    continuation.resume(throwing: error)
                }
            }
        }
    }
}

struct GPUTraceSavedComparison: Identifiable {
    let folder: URL
    let title: String
    let created: Date
    var id: String { folder.lastPathComponent }
    var label: String { "\(title) · \(created.formatted(date: .abbreviated, time: .shortened)) · \(id.prefix(8))" }
    static func select(from pairs: [Self], preferredID: String?) -> Self? {
        pairs.first { $0.id == preferredID } ?? pairs.first
    }
}

// BEGIN GPU_TRACE_FILE_PRESENTATION
/// A request only exists after the archive has finished writing. UIKit gets a
/// regular local file URL, not an asynchronous Transferable/item provider.
struct GPUTraceFileRequest: Identifiable {
    enum Action: String { case share, save }
    let id = UUID()
    let url: URL
    let action: Action
    init(url: URL, action: Action) throws {
        let values = try url.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
        guard url.isFileURL, values.isRegularFile == true, (values.fileSize ?? 0) > 0,
              FileManager.default.isReadableFile(atPath: url.path) else {
            throw CocoaError(.fileReadCorruptFile)
        }
        self.url = url; self.action = action
    }
}

/// Attach to the current SwiftUI controller, including the diagnostics sheet.
/// A global keyWindow/root controller can be behind that sheet on iPad.
struct GPUTraceFilePresenter: UIViewControllerRepresentable {
    @Binding var request: GPUTraceFileRequest?
    var finished: (String) -> Void
    func makeUIViewController(context: Context) -> GPUTraceFileHostController { GPUTraceFileHostController() }
    func updateUIViewController(_ controller: GPUTraceFileHostController, context: Context) {
        controller.finished = { id, status in
            if request?.id == id { request = nil; finished(status) }
        }
        controller.request = request
        controller.presentWhenReady()
    }
}

final class GPUTraceFileHostController: UIViewController, UIDocumentPickerDelegate, UIAdaptivePresentationControllerDelegate {
    var request: GPUTraceFileRequest?
    var finished: ((UUID, String) -> Void)?
    private var active: GPUTraceFileRequest?
    private var completedID: UUID?
    private var scheduled = false
    override func loadView() { view = UIView(); view.backgroundColor = .clear }
    override func viewDidAppear(_ animated: Bool) { super.viewDidAppear(animated); presentWhenReady() }
    func presentWhenReady() {
        guard !scheduled, active == nil, let request, request.id != completedID else { return }
        scheduled = true
        DispatchQueue.main.async { [weak self] in
            guard let self else { return }; self.scheduled = false
            // updateUIViewController can run before this controller has a window.
            guard self.viewIfLoaded?.window != nil, self.active == nil,
                  self.presentedViewController == nil, self.request?.id == request.id else { return }
            self.active = request
            let modal: UIViewController
            switch request.action {
            case .share:
                let activity = UIActivityViewController(activityItems: [request.url], applicationActivities: nil)
                activity.completionWithItemsHandler = { [weak self] _, completed, _, error in
                    if let error { self?.finish("Sharing failed: \(error.localizedDescription)") }
                    else { self?.finish(completed ? "ZIP shared. The original remains in GPUTraceExports." : "Sharing cancelled. The ZIP remains in GPUTraceExports.") }
                }
                if let popover = activity.popoverPresentationController {
                    popover.sourceView = self.view
                    popover.sourceRect = CGRect(x: self.view.bounds.midX, y: self.view.bounds.midY, width: 1, height: 1)
                    popover.permittedArrowDirections = []
                }
                modal = activity
            case .save:
                let picker = UIDocumentPickerViewController(forExporting: [request.url], asCopy: true)
                picker.delegate = self; modal = picker
            }
            modal.presentationController?.delegate = self
            self.present(modal, animated: true) {
                modal.presentationController?.delegate = self
                NSLog("[gpu-trace-pair] file-presented action=%@ file=%@", request.action.rawValue, request.url.lastPathComponent)
            }
        }
    }
    private func finish(_ status: String) {
        guard let active, completedID != active.id else { return }
        completedID = active.id; self.active = nil; request = nil
        NSLog("[gpu-trace-pair] file-result action=%@ status=%@", active.action.rawValue, status)
        finished?(active.id, status)
    }
    func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentsAt urls: [URL]) {
        finish(urls.isEmpty ? "No destination selected. The ZIP remains in GPUTraceExports." : "ZIP saved to Files. The original remains in GPUTraceExports.")
    }
    func documentPickerWasCancelled(_ controller: UIDocumentPickerViewController) { finish("Save cancelled. The ZIP remains in GPUTraceExports.") }
    func presentationControllerDidDismiss(_ presentationController: UIPresentationController) { finish("Export closed. The ZIP remains in GPUTraceExports.") }
}
// END GPU_TRACE_FILE_PRESENTATION

struct GPUTraceComparisonControls: View {
    @ObservedObject private var trace = GPUTraceComparison.shared
    @State private var fileRequest: GPUTraceFileRequest?
    @State private var preparing = false
    var resume: () -> Void = {}
    private func export(_ folder: URL, action: GPUTraceFileRequest.Action) {
        guard !preparing, fileRequest == nil else { return }
        preparing = true
        Task { @MainActor in
            defer { preparing = false }
            do { fileRequest = try GPUTraceFileRequest(url: await trace.export(folder: folder), action: action) }
            catch { trace.exportFinished("Could not prepare ZIP: \(error.localizedDescription)") }
        }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text("GPU comparison").font(.headline)
            if trace.savedComparisons.count > 1 {
                Picker("Saved comparison", selection: Binding(get: { trace.savedFolder?.lastPathComponent ?? "" }, set: { trace.selectSaved($0) })) {
                    ForEach(trace.savedComparisons) { pair in Text(pair.label).tag(pair.id) }
                }.disabled(trace.exporting || preparing || fileRequest != nil)
            } else if let pair = trace.savedComparisons.first {
                Text(pair.label).font(.caption).foregroundStyle(.secondary)
            }
            if let folder = trace.savedFolder {
                Button("Share GPU trace ZIP", systemImage: "square.and.arrow.up") { export(folder, action: .share) }
                    .buttonStyle(.borderedProminent).disabled(trace.recording || trace.busy || trace.exporting || preparing || fileRequest != nil)
                Button("Save ZIP to Files", systemImage: "folder") { export(folder, action: .save) }
                    .disabled(trace.recording || trace.busy || trace.exporting || preparing || fileRequest != nil)
            } else {
                Label("Share GPU trace ZIP", systemImage: "square.and.arrow.up")
                    .foregroundStyle(.secondary)
                Text(trace.findingSaved ? "Finding saved traces…" : "No completed comparison found yet.").font(.caption)
            }
            Button("Record FPS drop", systemImage: "waveform.path.ecg") { if trace.start() { resume() } }
                .disabled(!trace.canStartRecording)
            Button("Capture current scene", systemImage: "camera") { if trace.start(singleSample: true) { resume() } }
                .disabled(!trace.canStartRecording)
            if trace.canStartRecording {
                Text("Already running slowly? Capture current scene records one sample after five seconds, at the current FPS, ready to share. A brief pause is expected.")
                    .font(.caption).foregroundStyle(.secondary)
            }
            if let hint = trace.recordHint { Text(hint).font(.caption).foregroundStyle(.secondary) }
            if trace.canCaptureSlow {
                Button("Capture low FPS now", systemImage: "camera") { trace.captureSlow(); resume() }
            }
            if trace.canCaptureReference {
                Button("Capture reference now", systemImage: "camera") { trace.captureReference(); resume() }
            }
            if trace.recording { Button("Stop recording", role: .cancel) { trace.interrupt("Recording stopped. Completed samples were kept.") } }
            if trace.recording, trace.savedFolder != nil { Text("Stop recording to share the saved comparison.").font(.caption) }
            if trace.exporting { ProgressView("Preparing GPU trace ZIP…") }
            if !trace.message.isEmpty { Text(trace.message).font(.caption).textSelection(.enabled) }
            Text("ZIP is prepared before opening Share or Files. You can also find it in Files → Madeira → GPUTraceExports. Saved comparisons remain available after restarting Madeira. Send log uploads only the text report.")
                .font(.caption).foregroundStyle(.secondary)
        }.buttonStyle(.borderless)
            .background { GPUTraceFilePresenter(request: $fileRequest, finished: trace.exportFinished).frame(width: 1, height: 1) }
            .onAppear { trace.restoreSaved() }
    }
}
