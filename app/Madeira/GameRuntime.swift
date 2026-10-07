import Foundation
import Combine

/// App-private command codec shared with build/session-host/protocol.h.
/// CreateProcess gets an explicit executable and command line, never cmd /c.
enum RuntimeWire {
    static let keys = ["MADEIRA_EXE", "MADEIRA_ARGS", "MADEIRA_CPU_COUNT", "MADEIRA_SCREEN_W", "MADEIRA_SCREEN_H",
                       "MADEIRA_FOLDER_COMPAT", "MADEIRA_DINPUT_PAD", "MADEIRA_WORKDIR", "MADEIRA_DESKTOP",
                       "MADEIRA_FASTSYNC", "MADEIRA_FASTSYNC_SEM", "FEX_X87REDUCEDPRECISION", "DXMT_D9_ANISO_LIMIT",
                       "DXMT_WSI_MODE_TABLE", "DXMT_WSI_MONITOR_IDENTITY", "DXMT_CENSUS_THROTTLE",
                       "MADEIRA_MIP_CLAMP_AUTO", "SDL_OPENGL_ES_DRIVER", "LOVE_GRAPHICS_USE_OPENGLES",
                       "ANGLE_DEFAULT_PLATFORM", "WINEDLLOVERRIDES", "_MADEIRA_LUA51_GC64_PATH"]
    struct Status {
        let generation: UInt64
        let state, rootPID, gamePID, exitCode, error, active: UInt32
    }
    static func quote(_ value: String) -> String {
        var out = "\"", slashes = 0
        for character in value {
            if character == "\\" { slashes += 1; continue }
            if character == "\"" { out += String(repeating: "\\", count: slashes * 2 + 1) + "\"" }
            else { out += String(repeating: "\\", count: slashes) + String(character) }
            slashes = 0
        }
        return out + String(repeating: "\\", count: slashes * 2) + "\""
    }
    private static func put32(_ value: UInt32, into data: inout Data) {
        for shift in stride(from: 0, to: 32, by: 8) { data.append(UInt8(truncatingIfNeeded: value >> shift)) }
    }
    private static func utf16(_ value: String) -> Data {
        var data = Data()
        for unit in value.utf16 { data.append(UInt8(truncatingIfNeeded: unit)); data.append(UInt8(truncatingIfNeeded: unit >> 8)) }
        data.append(contentsOf: [0, 0]); return data
    }
    static func command(generation: UInt64, operation: UInt32, fields: [String] = []) throws -> Data {
        guard generation > 0, (1...3).contains(operation), operation == 1 ? fields.count == 6 : fields.isEmpty else {
            throw SupportError.message("Invalid runtime command.")
        }
        let chunks = operation == 1 ? fields.map(utf16) : Array(repeating: Data(), count: 6)
        guard chunks.allSatisfy({ $0.count <= 65536 }), chunks.reduce(44, { $0 + $1.count }) <= 262144 else {
            throw SupportError.message("The runtime launch command is too large.")
        }
        var data = Data("MDRUN001".utf8)
        put32(UInt32(truncatingIfNeeded: generation), into: &data); put32(UInt32(generation >> 32), into: &data)
        put32(operation, into: &data)
        for chunk in chunks { put32(UInt32(chunk.count), into: &data) }
        for chunk in chunks { data.append(chunk) }
        return data
    }
    static func status(_ data: Data) -> Status? {
        let bytes = Array(data)
        guard bytes.count == 40, Array(bytes.prefix(8)) == Array("MDSTAT01".utf8) else { return nil }
        func value(_ offset: Int) -> UInt32 {
            (0..<4).reduce(UInt32(0)) { $0 | UInt32(bytes[offset + $1]) << ($1 * 8) }
        }
        guard [UInt32(0), 2, 3, 4].contains(value(16)), value(20) != 0 else { return nil }
        return Status(generation: UInt64(value(8)) | UInt64(value(12)) << 32, state: value(16),
                      rootPID: value(20), gamePID: value(24), exitCode: value(28), error: value(32), active: value(36))
    }
}

/// Immutable launch descriptor; publication runs on the existing launch worker.
struct RuntimeLaunch {
    let directory: URL
    let generation: UInt64
    let executable, arguments, workingDirectory, registryKey: String
    let registryValues: [String: UInt32]
    let baseline: [String: String]

    func restoreEnvironment() {
        for key in RuntimeWire.keys + ["_MADEIRA_OPENGL_ANGLE_MODE", "_MADEIRA_LUA51_GC64", "_MADEIRA_OPENGL_ANGLE_AUTO_ENV"] {
            if let value = baseline[key] { setenv(key, value, 1) } else { unsetenv(key) }
        }
    }
    func publish() throws {
        wine_prepare_game_compatibility()
        // Only named compatibility options enter the file. No keychain tokens,
        // Steam credentials, broad ProcessInfo environment dump or shell script.
        let environment = RuntimeWire.keys.sorted().map { key in
            key + "=" + (getenv(key).map { String(cString: $0) } ?? "")
        }.joined(separator: "\0") + "\0"
        let preferences = registryValues.sorted { $0.key < $1.key }.map { "\($0.key)=\($0.value)" }.joined(separator: "\0") + "\0"
        let command = RuntimeWire.quote(executable) + (arguments.isEmpty ? "" : " " + arguments)
        let packet = try RuntimeWire.command(generation: generation, operation: 1,
            fields: [executable, command, workingDirectory, environment, registryKey, preferences])
        try packet.write(to: directory.appendingPathComponent("request.bin"), options: .atomic)
    }
}

/// One Wine bootstrap per app run, many supervised child games. The app never
/// declares a Windows job's logical exit sufficient for JIT/GPU memory reuse.
final class GameRuntime: ObservableObject {
    static let shared = GameRuntime()
    enum State: String { case off, starting, running, draining, ready, needsRestart }
    @Published private(set) var state: State = .off
    @Published private(set) var message = ""
    private(set) var ownsSession = false
    private(set) var launch: RuntimeLaunch?
    private(set) var lastExitCode: UInt32 = 0
    private var directory: URL?
    private var generation: UInt64 = 0
    private var timer: Timer?
    private var started = Date(), drainStarted = Date()
    private var baseline: [String: String] = [:]
    private var signature = ""
    private var lastLog = ""
    private var stoppedConfirmed = false
    var hasEngine: Bool { state != .off }
    var ended: Bool { ownsSession && stoppedConfirmed }
    static var enabled: Bool { MadeiraConfig.flag("MADEIRA_MULTI_GAME") && MadeiraConfig.get("env.MADEIRA_JIT_IMAGE_RETIRE") != "0" }
    static func supports(_ entry: LibraryEntry) -> Bool {
        // A real Steam/Dock session owns a separate authenticated client/service
        // lifecycle. Never quietly bypass it to make this generic route work.
        enabled && entry.desktop != true && entry.temporarySession != true &&
        (entry.steamAppID == nil || entry.startsSteamGameDirectly) &&
        (MadeiraConfig.get("env.DXMT_REMOTE_METAL") ?? ProcessInfo.processInfo.environment["DXMT_REMOTE_METAL"] ?? "").isEmpty &&
        !MadeiraConfig.bool("d3d12", default: false)
    }
    private func backendSignature(_ entry: LibraryEntry) -> String {
        // These are latched by Wine/FEX/native backends. A changed setting needs
        // a new engine, not a claimed live per-game toggle.
        ["x87=\(entry.reducedX87)", "fast=\(entry.fastSync ?? true)", "sem=\(entry.semaphoreFastPath ?? false)",
         "aniso=\(entry.anisotropyLimit.map(String.init) ?? "auto")", "unity=\(entry.unityOptimizations != false)"]
        .joined(separator: "|") + ["pool", "inproc-sync", "env.MADEIRA_FASTSYNC", "d3d12"].map {
            "|\($0)=\(MadeiraConfig.get($0) ?? "default")"
        }.joined()
    }
    func prepare(_ entry: LibraryEntry) throws -> RuntimeLaunch {
        guard Self.supports(entry), !ownsSession else { throw SupportError.message("This launch cannot share the active runtime.") }
        if hasEngine {
            guard state == .ready else { throw SupportError.message(message.isEmpty ? "The previous game is still being cleaned up." : message) }
            guard wine_process_is_running() != 0, wineserver_is_running() != 0 else {
                throw SupportError.message("The persistent Wine engine stopped. Restart Madeira.")
            }
            guard signature == backendSignature(entry) else {
                throw SupportError.message("This game's CPU/synchronization/backend settings need a new Wine engine. Restart Madeira to apply them.")
            }
        } else {
            guard Bundle.main.url(forResource: "madeira-session-host", withExtension: "exe", subdirectory: "arm64ec-windows") != nil else {
                throw SupportError.message("The reusable session host is missing. Build the source-bootstrap workflow.")
            }
            let folder = LibraryModel.drive.appendingPathComponent("madeira-runtime", isDirectory: true)
                .appendingPathComponent(UUID().uuidString.lowercased(), isDirectory: true)
            try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true,
                attributes: [.protectionKey: FileProtectionType.completeUntilFirstUserAuthentication])
            directory = folder
            for key in RuntimeWire.keys + ["_MADEIRA_OPENGL_ANGLE_MODE", "_MADEIRA_LUA51_GC64", "_MADEIRA_OPENGL_ANGLE_AUTO_ENV"] {
                if let value = getenv(key) { baseline[key] = String(cString: value) }
            }
            signature = backendSignature(entry)
            let channel = "C:\\madeira-runtime\\" + folder.lastPathComponent
            setenv("_MADEIRA_RUNTIME_CHANNEL", channel, 1)
            setenv("MADEIRA_JIT_IMAGE_RETIRE", "1", 1)
            wine_runtime_enable()
            timer = Timer(timeInterval: 0.25, repeats: true) { [weak self] _ in self?.poll() }
            RunLoop.main.add(timer!, forMode: .common)
        }
        generation += 1
        wine_exit_status_reset(); winios_reset_session_close()
        wine_runtime_begin_generation(generation)
        let configuredCPU = Int(MadeiraConfig.get("cpu-count") ?? "") ?? 0
        let automaticCPU = (1..<64).contains(configuredCPU) ? configuredCPU : ProcessInfo.processInfo.processorCount
        let reportedCPU = entry.cpuCount ?? (ExternalGameCompatibility.isUnity(entry) && entry.unityOptimizations != false
            ? 4 : automaticCPU)
        wine_runtime_set_processors(UInt32(clamping: reportedCPU))
        let preferences = MadeiraConfig.flag("MADEIRA_CONTROLLER_AUTO_PREFS") ? ControllerCompatibility.profile(for: entry) : nil
        let key = preferences?.section.replacingOccurrences(of: "\\\\", with: "\\") ?? ""
        let cwd = entry.steamWorkingWindowsPath ?? String(entry.launchWindowsPath.prefix(through: entry.launchWindowsPath.lastIndex(of: "\\")!))
        let next = RuntimeLaunch(directory: directory!, generation: generation, executable: entry.launchWindowsPath,
            arguments: entry.launchArguments, workingDirectory: cwd, registryKey: key,
            registryValues: preferences?.values ?? [:], baseline: baseline)
        launch = next; ownsSession = true; stoppedConfirmed = false; lastExitCode = 0; started = Date()
        state = .starting; message = "Starting game in the reusable Wine runtime…"
        logState(); return next
    }
    func publicationFailed(_ reason: String) {
        state = .needsRestart; stoppedConfirmed = true; message = reason; logState()
    }
    func quit(force: Bool) {
        guard ownsSession, let directory else { return }
        do { try RuntimeWire.command(generation: generation, operation: force ? 3 : 2)
                .write(to: directory.appendingPathComponent("control.bin"), options: .atomic) }
        catch { message = error.localizedDescription }
    }
    func sessionFinished() { ownsSession = false; launch = nil }
    private func poll() {
        guard let directory else { return }
        if wine_launched_process_has_exited() != 0 {
            state = .needsRestart; message = "The persistent Wine host stopped. Restart Madeira before another game."; logState(); return
        }
        if let data = try? Data(contentsOf: directory.appendingPathComponent("status.bin")),
           let report = RuntimeWire.status(data), report.generation == generation {
            if report.state == 2, state == .starting { state = .running; message = "" }
            if report.state == 3 || (report.state == 4 && report.active == 0) {
                stoppedConfirmed = true
                lastExitCode = report.exitCode
                if state == .starting || state == .running {
                    state = .draining; drainStarted = Date()
                    message = report.error != 0 ? "Game launch failed (Windows error \(report.error)). Checking cleanup…" : "Cleaning up the previous game…"
                }
            } else if report.state == 4 {
                state = .needsRestart; message = "Wine could not verify the game's process tree stopped (\(report.error)). Restart required."
            }
        }
        if state == .draining {
            let readiness = wine_runtime_reuse_ready()
            if readiness > 0 { state = .ready; message = "Wine runtime ready — another game can start." }
            else if readiness < 0 || Date().timeIntervalSince(drainStarted) > 10 {
                state = .needsRestart
                message = "The previous game left active threads or unsafe JIT mappings. Restart required for safety."
            }
        } else if state == .starting && Date().timeIntervalSince(started) > 180 {
            state = .needsRestart; message = "The runtime did not confirm game startup. Send a log; restart before retrying."
        }
        logState()
    }
    private func logState() {
        let line = "state=\(state.rawValue) generation=\(generation)" +
            (state == .draining ? " threads=\(wine_runtime_live_threads()) gpu=\(wine_runtime_gpu_pending())" : "")
        if line != lastLog { lastLog = line; LogStore.shared.log("[multi-game] \(line)") }
    }
}
