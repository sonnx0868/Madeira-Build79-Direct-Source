import Foundation
import SwiftUI
import UIKit
import CoreTransferable
import UniformTypeIdentifiers

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
    private var eligibleSession = false
    private var menuOpen = false
    private var foreground = true
    private var timer: Timer?
    private var policy = GPUTraceDropPolicy()
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
    func session(title: String?) {
        interrupt("The game session ended. Completed traces are still available.")
        eligibleSession = title?.localizedCaseInsensitiveContains("replaced") == true
    }
    func setMenuOpen(_ open: Bool) {
        menuOpen = open
        if open, lastCapture != nil { interrupt("The game menu opened during capture. Start a new comparison from gameplay.") }
        if !open { policy.resume(after: ProcessInfo.processInfo.systemUptime) }
    }
    func start() {
        guard eligibleSession, !recording, !busy, !exporting, foreground else { return }
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
                LogStore.shared.log("[gpu-trace-pair] armed folder=GPUTraces/\(folder.lastPathComponent) mode=\(fpsMode) baseline-min-fps=50")
            } catch { if token == generation { busy = false; message = error.localizedDescription; restoreSaved() } }
        }
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
                if result.0 == 4 || result.0 == 5 {
                    manifest[kind] = ["file": "\(kind).gputrace", "complete": result.0 == 4, "buffers": result.1,
                                      "frames": result.2, "captureUptime": result.3, "error": result.4,
                                      "finishedAt": ISO8601DateFormatter().string(from: Date())]
                    lastCapture = nil
                    if result.0 == 5 { interrupt(result.4); return }
                    if kind == "smooth" {
                        policy.capturedReference(at: ProcessInfo.processInfo.systemUptime); canCaptureSlow = true
                        message = "Smooth sample saved. Keep playing in the same scene; a sustained 20% FPS drop triggers the second sample."
                    } else {
                        recording = false; canCaptureSlow = false; timer?.invalidate(); timer = nil
                        message = "Both GPU samples saved. Open Madeira's menu and tap Share GPU trace ZIP."
                    }
                    persist()
                    if let folder = activeFolder {
                        GPUTraceFiles.snapshot(log: LogStore.shared.fileForUpload(previous: false), folder: folder)
                        if kind == "slow" { restoreSaved(prefer: folder) }
                    }
                    LogStore.shared.log("[gpu-trace-pair] sample=\(kind) complete=1 buffers=\(result.1) frames=\(result.2)")
                }
            }
        } else if !manualScheduled, let kind = policy.sample(fps: fps, at: now, eligible: usable) { capture(kind) }
    }
    private func capture(_ kind: String) {
        guard recording, !busy, lastCapture == nil, let folder = activeFolder else { return }
        lastCapture = kind; let token = generation
        manifest["\(kind)BeforeCapture"] = history.suffix(15).map { $0 }
        if let reference = policy.reference { manifest["referenceFPS"] = reference }
        manifest["fpsMode"] = fpsMode
        message = "Recording two complete \(kind == "smooth" ? "smooth" : "low FPS") frames… A brief pause is expected."
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
        if let kind = lastCapture { manifest[kind] = ["file": "\(kind).gputrace", "complete": false, "error": reason] }
        lastCapture = nil; persist()
        if let folder = activeFolder { GPUTraceFiles.snapshot(log: LogStore.shared.fileForUpload(previous: false), folder: folder) }
        let token = generation
        GPUTraceNative.cancel { [self] in if token == generation { busy = false } }
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
        let task = Task { try await GPUTraceFiles.export(folder: folder) }; exportTask = task
        defer { exporting = false; exportTask = nil; exportingFolder = nil }
        do {
            let url = try await task.value
            cachedExport = (folder, url)
            message = "ZIP ready. Choose Save to Files or a sharing destination."
            return url
        } catch {
            message = error.localizedDescription; throw error
        }
    }
}

/// Enqueue synchronously from main so request/cancel/new-session cannot reorder
/// across detached tasks. Metal stop/serialization never blocks UIKit's thread.
enum GPUTraceNative {
    private static let queue = DispatchQueue(label: "madeira.gpu-trace.control", qos: .userInitiated)
    @MainActor static func request(_ file: URL, done: @escaping @MainActor (Bool, String) -> Void) {
        queue.async {
            let accepted = file.path.withCString { madeira_gpu_trace_request($0) } != 0
            let error = madeira_gpu_trace_error() ?? ""
            DispatchQueue.main.async { done(accepted, error) }
        }
    }
    @MainActor static func poll(_ done: @escaping @MainActor ((Int, Int, Int, Double, String)) -> Void) {
        queue.async {
            let value = madeira_gpu_trace_status()
            let result = (Int(value.phase), Int(value.buffers), Int(value.frames), value.started, madeira_gpu_trace_error() ?? "")
            DispatchQueue.main.async { done(result) }
        }
    }
    @MainActor static func cancel(_ done: @escaping @MainActor () -> Void) {
        queue.async { madeira_gpu_trace_cancel(); DispatchQueue.main.async { done() } }
    }
}

enum GPUTraceFiles {
    private static let queue = DispatchQueue(label: "madeira.gpu-trace.files", qos: .utility)
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
                    pairs.append(GPUTraceSavedComparison(folder: folder, title: title, created: date))
                }
                pairs.sort { $0.created == $1.created ? $0.id < $1.id : $0.created > $1.created }
                continuation.resume(returning: pairs)
            }
        }
    }
    private static func completeManifest(folder: URL) throws -> [String: Any] {
        let fm = FileManager.default, manifest = folder.appendingPathComponent("manifest.json")
        let values = try manifest.resourceValues(forKeys: [.isRegularFileKey, .isSymbolicLinkKey, .fileSizeKey])
        guard values.isRegularFile == true, values.isSymbolicLink != true, let size = values.fileSize, size <= 1024 * 1024 else {
            throw SupportError.message("GPU comparison metadata is invalid.")
        }
        let object = try JSONSerialization.jsonObject(with: Data(contentsOf: manifest)) as? [String: Any]
        guard let object, object["format"] as? String == "madeira-gpu-comparison-v1" else {
            throw SupportError.message("GPU comparison metadata is missing.")
        }
        for kind in ["smooth", "slow"] {
            let sample = object[kind] as? [String: Any]
            guard sample?["complete"] as? Bool == true, sample?["frames"] as? Int == 2,
                  sample?["file"] as? String == "\(kind).gputrace",
                  let trace = try? folder.appendingPathComponent("\(kind).gputrace").resourceValues(forKeys: [.isDirectoryKey, .isRegularFileKey, .isSymbolicLinkKey]),
                  trace.isSymbolicLink != true, trace.isDirectory == true || trace.isRegularFile == true else {
                throw SupportError.message("Record both complete GPU samples before exporting the comparison.")
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
    static func export(folder: URL) async throws -> URL {
        try await withCheckedThrowingContinuation { continuation in
            queue.async {
                do {
                    let fm = FileManager.default
                    guard folder.resolvingSymlinksInPath().standardizedFileURL == folder.standardizedFileURL else {
                        throw SupportError.message("The GPU comparison folder must not be a symbolic link.")
                    }
                    _ = try completeManifest(folder: folder)
                    NSLog("[gpu-trace-pair] export-start pair=%@", folder.lastPathComponent)
                    let allowed: Set<String> = ["smooth.gputrace", "slow.gputrace", "manifest.json", "session.txt"]
                    let children = try fm.contentsOfDirectory(at: folder, includingPropertiesForKeys: nil)
                    guard children.allSatisfy({ allowed.contains($0.lastPathComponent) }),
                          let walker = fm.enumerator(at: folder, includingPropertiesForKeys: [.isSymbolicLinkKey]) else {
                        throw SupportError.message("The comparison folder contains unexpected files.")
                    }
                    var count = 0
                    for case let file as URL in walker {
                        count += 1
                        guard count <= 100_000, try file.resourceValues(forKeys: [.isSymbolicLinkKey]).isSymbolicLink != true else {
                            throw SupportError.message("The comparison contains a symbolic link or too many files.")
                        }
                    }
                    let sessionLog = folder.appendingPathComponent("session.txt")
                    // Keep the log of the captured test after a restart or a
                    // different game launch. Never replace it with that run.
                    guard fm.fileExists(atPath: sessionLog.path) else {
                        throw SupportError.message("The captured session log is missing. Record a new comparison.")
                    }
                    let target = fm.temporaryDirectory.appendingPathComponent("Madeira-GPU-\(folder.lastPathComponent)-\(UUID().uuidString).zip")
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

struct GPUTraceArchive: Transferable {
    let folder: URL
    static var transferRepresentation: some TransferRepresentation {
        FileRepresentation(exportedContentType: .zip) { archive in
            let url = try await GPUTraceComparison.shared.export(folder: archive.folder)
            return SentTransferredFile(url)
        }
    }
}

struct GPUTraceComparisonControls: View {
    @ObservedObject private var trace = GPUTraceComparison.shared
    var canStart = false
    var resume: () -> Void = {}
    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text("GPU comparison").font(.headline)
            if trace.savedComparisons.count > 1 {
                Picker("Saved comparison", selection: Binding(get: { trace.savedFolder?.lastPathComponent ?? "" }, set: { trace.selectSaved($0) })) {
                    ForEach(trace.savedComparisons) { pair in Text(pair.label).tag(pair.id) }
                }.disabled(trace.exporting)
            } else if let pair = trace.savedComparisons.first {
                Text(pair.label).font(.caption).foregroundStyle(.secondary)
            }
            if let folder = trace.savedFolder {
                ShareLink(item: GPUTraceArchive(folder: folder), preview: SharePreview("GPU trace ZIP")) {
                    Label("Share GPU trace ZIP", systemImage: "square.and.arrow.up")
                }.buttonStyle(.borderedProminent).disabled(trace.recording || trace.busy || trace.exporting)
            } else {
                Label("Share GPU trace ZIP", systemImage: "square.and.arrow.up")
                    .foregroundStyle(.secondary)
                Text(trace.findingSaved ? "Finding saved traces…" : "No completed comparison found yet.").font(.caption)
            }
            if canStart {
                Button(trace.savedFolder == nil ? "Record FPS drop" : "Record another comparison", systemImage: "waveform.path.ecg") { trace.start(); resume() }
                    .disabled(trace.recording || trace.busy || trace.exporting)
                if trace.canCaptureSlow {
                    Button("Capture low FPS now", systemImage: "camera") { trace.captureSlow(); resume() }
                }
                if trace.canCaptureReference {
                    Button("Capture reference now", systemImage: "camera") { trace.captureReference(); resume() }
                }
            }
            if trace.recording { Button("Stop recording", role: .cancel) { trace.interrupt("Recording stopped. Completed samples were kept.") } }
            if trace.recording, trace.savedFolder != nil { Text("Stop recording to share the saved comparison.").font(.caption) }
            if trace.exporting { ProgressView("Preparing GPU trace ZIP…") }
            if !trace.message.isEmpty { Text(trace.message).font(.caption).textSelection(.enabled) }
            Text("Share GPU trace ZIP opens the sharing menu and prepares the file for your chosen destination. Saved comparisons remain available after restarting Madeira. Send log uploads only the text report.")
                .font(.caption).foregroundStyle(.secondary)
        }.buttonStyle(.borderless).onAppear { trace.restoreSaved() }
    }
}
