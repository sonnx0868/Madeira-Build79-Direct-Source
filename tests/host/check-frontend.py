#!/usr/bin/env python3
"""Library front end: launch profiles, controller navigation and the session
exit report.

1. Swift: compiles the production LibraryEntry and LibraryController
   (app/Madeira/Library.swift) and the display layout (app/Madeira/
   GuestDisplay.swift) with small stubs and checks the launch environment a
   profile exports (executable, arguments, virtual monitor size for every
   entry, x87 precision only when chosen, fastsync's switches only when
   Settings chose Fastsync, nothing else for the engine), the
   30 FPS fallback without DXMT's 30 FPS cap, profile validation, decoding of
   library files that carry unknown or fork-written keys (display mode,
   control opacity and size), the layout and touch-mapping math of every
   Aspect & scaling mode, and the pad-to-command mapping.
2. C: compiles the session exit hook from app/Madeira/WineProcessBridge.m and
   checks application and NTSTATUS exits of the launched program, a separate
   unknown-status state, the legacy crash-only API and session reset.
3. Source checks: ntdll reports only the launched (initial) process's exit
   status, with no image names; the display-rate hold is removed and the 30 FPS
   cap is detected; the app wires the library into ContentView and
   GamepadInput; game details offer Resolution (with Screen shape) for every
   entry, Aspect & scaling and control opacity/size; the in-game menu offers
   Aspect & scaling, opacity, size and the Touch pointer mode; a session
   saves those choices to the game; the starting screen's controls are one row
   of glyph-only buttons with VoiceOver labels; and Settings ends with Credits
   and no longer carries the drive_c note.

Run from anywhere; needs `swift` and `cc` on PATH.
"""
from pathlib import Path
import os
import re
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
lib = (root / 'app/Madeira/Library.swift').read_text()
display = (root / 'app/Madeira/GuestDisplay.swift').read_text()
bridge = (root / 'app/Madeira/WineProcessBridge.m').read_text()
loader = (root / 'build/ntdll-unix/loader_ios.c').read_text()
server = (root / 'build/ntdll-unix/server_ios.c').read_text()
content = (root / 'app/Madeira/ContentView.swift').read_text()
gamepad = (root / 'app/Madeira/GamepadInput.swift').read_text()
fps = (root / 'app/Madeira/FPSOverlay.swift').read_text()
shim = (root / 'app/Madeira/IOSDisplayShim.m').read_text()
driver = (root / 'build/win32u-unix/driver_ios.c').read_text()
failures = []


def check(cond, what):
    print(('PASS: ' if cond else 'FAIL: ') + what)
    if not cond:
        failures.append(what)


def block(text, header):
    p = text.index(header)
    a = text.index('{', p)
    n, b = 1, a + 1
    while n:
        n += (text[b] == '{') - (text[b] == '}')
        b += 1
    return text[p:b]


swift = r'''
import Foundation
import CryptoKit
enum LibraryModel { static let drive = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent("madeira-frontend-cpu-fixture") }
#if canImport(CoreGraphics)
import CoreGraphics   // CGRect.width and friends: Foundation alone no longer re-exports them on macOS (Swift 6.4)
#endif
#if canImport(Combine)
import Combine
#else
// Linux hosts: just enough of Combine for LibraryController.
protocol ObservableObject: AnyObject {}
@propertyWrapper struct Published<Value> { var wrappedValue: Value; init(wrappedValue: Value) { self.wrappedValue = wrappedValue } }
final class AnyCancellable { init() {} }
final class PassthroughSubject<Output, Failure: Error> {
    private var receivers: [(Output) -> Void] = []
    func send(_ value: Output) { receivers.forEach { $0(value) } }
    func sink(receiveValue: @escaping (Output) -> Void) -> AnyCancellable { receivers.append(receiveValue); return AnyCancellable() }
}
#endif
enum MadeiraConfig {
    static var values: [String: String] = [:]   // stands in for madeira.cfg
    static func flag(_ name: String, fallback: Bool = true) -> Bool { values["env." + name].map { $0 != "0" } ?? fallback }
    static func get(_ key: String) -> String? { values[key] }
    static func bool(_ key: String, default dflt: Bool = false) -> Bool { values[key].map { ["1", "on", "true", "yes"].contains($0) } ?? dflt }
    @discardableResult static func set(_ key: String, _ value: String?) -> Bool { values[key] = value; return true }
}
final class LogStore { static let shared = LogStore(); var lines: [String] = []; func log(_ s: String) { lines.append(s) } }
var published: (Int32, Int32) = (0, 0)
func winios_display_mode_changed(_ w: Int32, _ h: Int32) { published = (w, h) }
var vsync: Int32 = -1
func madeira_set_vsync_locked(_ mode: Int32) { vsync = mode }
enum ProMotionIntent { static var has30Cap = true }
enum ThermalPacingController { static func request(_ mode: Int32) { madeira_set_vsync_locked(mode) } }
struct TouchControl: Codable, Equatable { var nx = 0.5 }
enum ControlAction: Codable, Equatable, Hashable { case none }   // LibraryEntry.controllerBinds
func wineserver_is_running() -> Int32 { 0 }
// The production file reader is exercised in check-pe-launch.py; this fixture
// controls its result to exercise LibraryEntry's preflight presentation.
var fixtureRequiresWOW64: Int32 = 0
func wine_executable_requires_wow64(_ path: String) -> Int32 { fixtureRequiresWOW64 }
enum LibraryError: LocalizedError { case message(String) }
func env(_ name: String) -> String? { getenv(name).map { String(cString: $0) } }
'''
dock_source = (root/'app/Madeira/MadeiraDock.swift').read_text(encoding='utf-8')
swift += 'enum GamepadInput { static let keyboardMouseAvailable = true\n' + block(gamepad, 'static func flag(') + '\n}\n'
swift += block(gamepad, 'enum SDLGamepadPolicy') + '\n'
swift += block(dock_source, 'struct DockGame: Identifiable') + '\n'
swift += block(dock_source, 'struct DockExternalProgram: Equatable') + '\n'
swift += 'enum MadeiraDock { static func validAppID(_ value: Int) -> Bool { value > 0 && UInt64(value) < UInt64(UInt32.max) } }\n'
swift += block(lib, 'struct LibraryEntry: Codable, Identifiable') + '\n'
swift += block(lib, 'enum LightEffectsPolicy') + '\n'
swift += block(lib, 'enum GameGraphicsPreferences') + '\n'
swift += block(lib, 'enum GameMemorySettings') + '\n'
swift += block(lib, 'enum GameExitReport') + '\n'
translation = (root/'app/Madeira/TranslationTools.swift').read_text(encoding='utf-8')
swift += block(translation, 'enum TranslationTools') + '\n'
swift += block(translation, 'enum CPUTranslationSettings') + '\n'
swift += block(translation, 'enum ShaderPreparationSettings') + '\n'
swift += block(lib, 'enum ControllerCompatibility') + '\n'
swift += block(lib, 'enum UnityLaunch') + '\n'
swift += block(lib, 'enum GodotLaunch') + '\n'
swift += block(lib, 'enum FolderSteamIdentity') + '\n'
swift += block(lib, 'enum UnityStartupSync') + '\n'
swift += (root/'app/Madeira/GameMakerCompatibility.swift').read_text(encoding='utf-8') + '\n'
swift += block(lib, 'enum ExternalGameCompatibility') + '\n'
swift += (root / 'app/Madeira/GameEngineDetection.swift').read_text(encoding='utf-8') + '\n'
swift += block(lib, 'enum SyncEngine: String, CaseIterable, Identifiable') + '\n'
swift += '\n'.join(l for l in display.splitlines() if not l.startswith('import ')) + '\n'
swift += block(lib, 'final class LibraryController: ObservableObject, @unchecked Sendable') + '\n'
swift += r'''
var failed = 0
func expect(_ cond: Bool, _ what: String) { print((cond ? "PASS: " : "FAIL: ") + what); if !cond { failed += 1 } }

// Desktop entry: services in a virtual desktop of the chosen size.
var desk = LibraryEntry.desktopEntry
desk.resolution = "1280x720"
expect(desk.launchArguments == "/desktop=shell,1280x720 C:\\windows\\system32\\services.exe", "desktop arguments")
desk.configureLaunch()
expect(env("MADEIRA_EXE") == "explorer.exe" && env("MADEIRA_DESKTOP") == "1", "desktop starts explorer in desktop mode")
expect(env("MADEIRA_SCREEN_W") == "1280" && env("MADEIRA_SCREEN_H") == "720", "desktop size exported")
expect(published == (1280, 720), "desktop size published to the display shim")

// A direct game: its Windows path and arguments; no desktop state left over.
var game = LibraryEntry(title: "Game", relativePath: "Games/Some Game/bin/game.exe", bits: 32)
var importedSteam = LibraryEntry(title: "Imported", relativePath: "Steam Session Fixture/program.exe", bits: 64)
importedSteam.steamID = 10
importedSteam.configureExternalSteam(true)
expect(importedSteam.externalSteamAppID == 10 && importedSteam.usesSteamSession && importedSteam.steamAppID == nil,
       "imported Steam session keeps manual identity and uses matched metadata")
let importedFolder = LibraryModel.drive.appendingPathComponent("Steam Session Fixture")
try FileManager.default.createDirectory(at: importedFolder, withIntermediateDirectories: true)
try Data("20".utf8).write(to: importedFolder.appendingPathComponent("steam_appid.txt"))
importedSteam.configureExternalSteam(true)
expect(importedSteam.externalSteamAppID == 20 && importedSteam.externalSteamGame?.external?.path == importedSteam.launchWindowsPath,
       "actual folder App ID converts to profile type and keeps the selected EXE")
importedSteam.configureExternalSteam(false)
expect(importedSteam.externalSteamAppID == nil && !importedSteam.usesSteamSession, "external Steam opt-out restores direct launch")
try FileManager.default.removeItem(at: importedFolder)
expect(!game.cardinalLeftStick, "ordinary games retain analog directions")
var silkPad = LibraryEntry(title: "Hollow Knight: Silksong", relativePath: "Games/Silksong/Silksong.exe", bits: 64)
expect(silkPad.cardinalLeftStick, "Silksong defaults to eight-way platformer directions")
expect(silkPad.launchCPUCacheMode == "reuse", "Silksong automatic profile learns and reuses verified CPU blocks")
silkPad.cpuCacheMode = "off"
expect(silkPad.launchCPUCacheMode == "off", "explicit CPU cache off overrides Silksong automatic mode")
silkPad.cpuCacheMode = nil; silkPad.unityOptimizations = false
expect(silkPad.launchCPUCacheMode == nil, "disabling the Unity profile disables automatic CPU reuse")
silkPad.unityOptimizations = nil
MadeiraConfig.set("env.MADEIRA_CPU_CACHE", "0")
expect(silkPad.launchCPUCacheMode == nil, "global CPU cache opt-out overrides automatic profile")
MadeiraConfig.set("env.MADEIRA_CPU_CACHE", nil)
expect(game.launchCPUCacheMode == nil, "other games retain CPU cache off")
silkPad.stickDirectionMode = "analog"
expect(!silkPad.cardinalLeftStick, "Silksong can retain unmodified analog axes")
game.stickDirectionMode = "eightWay"
expect(game.cardinalLeftStick, "any game can opt into eight-way directions")
game.stickDirectionMode = nil
unsetenv("DXMT_CENSUS_THROTTLE"); game.applyEnvironment()
expect(env("DXMT_CENSUS_THROTTLE") == "1", "imported games throttle DXMT census by default")
game.cpuCacheMode = "reuse"; game.cpuTranslationBudget = 512; game.applyEnvironment()
expect(env("MADEIRA_CPU_CACHE") == "reuse" && env("FEX_MAXINST") == "512", "per-game CPU experiments exported")
game.cpuCacheMode = nil; game.cpuTranslationBudget = nil; game.applyEnvironment()
expect(env("MADEIRA_CPU_CACHE") == "0" && env("MADEIRA_CPU_CACHE_PATH") == nil && env("FEX_MAXINST") == nil, "next game's default clears CPU experiments")
game.glBackend = "zink"; game.thinReserve = true; game.applyEnvironment()
expect(env("MADEIRA_GL_BACKEND") == "zink" && env("_MADEIRA_GL_PROFILE") == "zink", "the game selects its native GL renderer")
expect(env("MADEIRA_THIN_RESERVE") == "1" && env("_MADEIRA_THIN_PROFILE") == "1", "thin reservations are explicitly selected per game")
game.glBackend = nil; game.thinReserve = nil; game.applyEnvironment()
expect(env("MADEIRA_GL_BACKEND") == nil && env("_MADEIRA_GL_PROFILE") == nil, "next game's automatic GL route clears the previous choice")
expect(env("MADEIRA_THIN_RESERVE") == "0" && env("_MADEIRA_THIN_PROFILE") == "0", "next game disables thin reservations")
game.arguments = "-windowed \"-name=a b\""
game.configureLaunch()
expect(env("MADEIRA_EXE") == "C:\\Games\\Some Game\\bin\\game.exe", "direct executable path")
expect(env("MADEIRA_ARGS") == "-windowed \"-name=a b\"", "direct arguments verbatim")
expect(env("MADEIRA_DESKTOP") == nil, "desktop state cleared")
// Every entry's Resolution becomes the session's virtual monitor.
expect(game.resolution == "1920x1080", "new entries default to 1920x1080")
expect(env("MADEIRA_SCREEN_W") == "1920" && env("MADEIRA_SCREEN_H") == "1080" && env("MADEIRA_SCREEN_SRC") == "knob",
       "a direct game's resolution is exported as the session default")
game.resolution = "1560x720"; game.configureLaunch()
expect(env("MADEIRA_SCREEN_W") == "1560" && env("MADEIRA_SCREEN_H") == "720" && published == (1560, 720),
       "a screen-shape resolution is exported and published")
expect((try? game.validate()) != nil, "a screen-shape resolution validates")
game.resolution = "1280x720"; game.configureLaunch()

// A Steam game: Madeira Dock sets what starts, so its profile leaves MADEIRA_EXE alone...
var steamGame = LibraryEntry(title: "Steam game", relativePath: "Program Files (x86)/Steam/steamapps/common/Some Game", bits: 0)
steamGame.steamAppID = 4242
unsetenv("DXMT_CENSUS_THROTTLE"); steamGame.applyEnvironment()
expect(env("DXMT_CENSUS_THROTTLE") == "1", "Steam/Dock gets the census reporting bound before its launch early return")
MadeiraConfig.set("env.DXMT_CENSUS_THROTTLE", "0"); steamGame.applyEnvironment()
expect(env("DXMT_CENSUS_THROTTLE") == "0", "explicit diagnostic census opt-out is preserved")
MadeiraConfig.set("env.DXMT_CENSUS_THROTTLE", nil); steamGame.applyEnvironment()
expect(env("DXMT_CENSUS_THROTTLE") == "0", "inherited census choice is respected")
unsetenv("DXMT_CENSUS_THROTTLE"); steamGame.applyEnvironment()
expect(env("DXMT_CENSUS_THROTTLE") == "1", "a clean Steam launch restores the default reporting bound")
setenv("MADEIRA_EXE", "set-by-dock", 1); setenv("MADEIRA_STEAM_APPID", "1", 1)
steamGame.configureLaunch()
expect(env("MADEIRA_EXE") == "set-by-dock" && env("MADEIRA_STEAM_APPID") == nil, "Madeira Dock (the default): the profile sets nothing that starts")
// ...and "Start with: The game" starts the game's own program, with the game's own Steam identity.
steamGame.steamStart = "game"; steamGame.steamProgram = "bin/game.exe"; steamGame.steamProgramArguments = "-dx11 \"-name=a b\""
steamGame.steamProgramFolder = "data"
expect((try? steamGame.validate()) != nil, "a direct Steam start validates")
steamGame.configureLaunch()
let steamFolder = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Some Game"
expect(env("MADEIRA_EXE") == steamFolder + "\\bin\\game.exe", "The game: its program")
expect(env("MADEIRA_ARGS") == "-dx11 \"-name=a b\"", "The game: Steam's arguments verbatim")
expect(env("MADEIRA_STEAM_APPID") == "4242" && env("MADEIRA_STEAM_APPPATH") == steamFolder,
       "The game: its own App ID and install folder for the bridge")
expect(env("MADEIRA_WORKDIR") == steamFolder + "\\data", "The game: Steam's working folder")
steamGame.steamProgramFolder = ""; steamGame.configureLaunch()
expect(env("MADEIRA_WORKDIR") == steamFolder, "working folder \"\": the install folder")
steamGame.steamProgramFolder = nil; steamGame.configureLaunch()
expect(env("MADEIRA_WORKDIR") == nil, "no working folder: the program's own (the bridge's default)")
expect(steamGame.windowsPath == steamFolder &&
       steamGame.launchRelativePath == "Program Files (x86)/Steam/steamapps/common/Some Game/bin/game.exe",
       "the entry keeps its install folder; only the launch path names the program")
steamGame.steamProgram = nil; steamGame.configureLaunch()
expect(env("MADEIRA_EXE") == steamFolder, "no program: the folder (ContentView refuses it first)")
game.configureLaunch()
expect(env("MADEIRA_STEAM_APPID") == nil && env("MADEIRA_STEAM_APPPATH") == nil && env("MADEIRA_WORKDIR") == nil,
       "any other launch clears the direct start's identity and folder")

// Engine switches: only x87 precision, and only when chosen (FEX's default otherwise).
expect(!game.reducedX87, "reduced-precision x87 is off for new entries")
setenv("FEX_X87REDUCEDPRECISION", "1", 1)
game.applyEnvironment()
expect(env("FEX_X87REDUCEDPRECISION") == nil, "x87: nothing exported unless chosen")
expect(env("MADEIRA_CPU_COUNT") == nil && env("DXMT_D9_ANISO_LIMIT") == nil, "no other engine switches are exported")
game.cpuCount = 4; game.applyEnvironment()
expect(env("MADEIRA_CPU_COUNT") == "4", "explicit four-CPU choice is preserved")
game.cpuCount = nil; game.applyEnvironment()
expect(env("MADEIRA_CPU_COUNT") == nil, "automatic CPU selection clears the previous game's explicit limit")
expect(env("MADEIRA_DINPUT_PAD") == "1", "Automatic controller mode publishes DirectInput beside XInput")
game.controllerMode = "xinput"; game.applyEnvironment()
expect(env("MADEIRA_DINPUT_PAD") == "0", "XInput-only mode suppresses the duplicate DirectInput view")
let godotFolder = LibraryModel.drive.appendingPathComponent("controller-godot")
try FileManager.default.createDirectory(at: godotFolder, withIntermediateDirectories: true)
try Data("GDPC".utf8).write(to: godotFolder.appendingPathComponent("game.pck"))
var godotPad = LibraryEntry(title: "Renamed", relativePath: "controller-godot/game.exe", bits: 64)
godotPad.applyEnvironment()
expect(env("MADEIRA_DINPUT_PAD") == "0", "Godot automatic input exposes one XInput device")
godotPad.controllerMode = "dinput"; godotPad.applyEnvironment()
expect(env("MADEIRA_DINPUT_PAD") == "1", "explicit DirectInput choice still wins")
var party = LibraryEntry(title: "Party Animals", relativePath: "Steam/PartyAnimals.exe", bits: 64)
party.applyEnvironment()
expect(env("_MADEIRA_PARTY_COMPAT") == "1", "Party Animals selects a consistent native CRT")
game.applyEnvironment()
expect(env("_MADEIRA_PARTY_COMPAT") == nil && env("_MADEIRA_SPLIT_POOL") == nil, "next game clears Party compatibility flags")
var gogProfile = LibraryEntry(title:"GOG fixture", relativePath:"GOG/game.exe", bits:64)
gogProfile.gogID = 1; gogProfile.gogLaunchCommand = "-store"; gogProfile.arguments = "-custom"
gogProfile.gogWorkingDir = "Program Files/GOG Games/1"
gogProfile.configureLaunch()
expect(gogProfile.launchArguments == "-store -custom", "GOG metadata preserves user launch arguments")
expect(env("MADEIRA_WORKDIR") == "C:\\Program Files\\GOG Games\\1", "GOG launch uses validated game working directory")
var epicProfile = game; epicProfile.epicAppName = "fixture"; epicProfile.applyEnvironment()
expect(env("_MADEIRA_PRIVATE_ARGS") == "1", "Epic launch marks argument logging private")
game.applyEnvironment()
expect(env("_MADEIRA_PRIVATE_ARGS") == nil && env("MADEIRA_WORKDIR") == nil, "next game clears store launch state")


game.controllerMode = nil
expect(env("MADEIRA_FASTSYNC") == "auto" && env("MADEIRA_FASTSYNC_SEM") == "0",
       "Fastsync default: automatic wake selection and conservative semaphore policy")
expect(LogStore.shared.lines.last == "[display-shape] resolution=1280x720 mode=fit", "the profile's display shape is logged")
MadeiraConfig.values = ["sync-engine": "nutssync"]
game.applyEnvironment()
expect(env("MADEIRA_FASTSYNC") == nil && env("MADEIRA_FASTSYNC_SEM") == nil,
       "Explicit NutsSync: the game's fastsync switches are not exported")
// Fastsync's per-game switches: exported only when Settings chose Fastsync.
MadeiraConfig.values = ["inproc-sync": "0"]
unsetenv("MADEIRA_FASTSYNC"); unsetenv("MADEIRA_FASTSYNC_SEM"); game.applyEnvironment()
expect(env("MADEIRA_FASTSYNC") == nil && env("MADEIRA_FASTSYNC_SEM") == nil, "Wine standard sync: no fastsync switches")
MadeiraConfig.values = ["sync-engine": "fastsync", "env.MADEIRA_FASTSYNC": "auto"]
game.applyEnvironment()
expect(env("MADEIRA_FASTSYNC") == "auto" && env("MADEIRA_FASTSYNC_SEM") == "0",
       "Fastsync: fast synchronization on by default (the chosen mode), semaphore waits off")
game.fastSync = false; game.semaphoreFastPath = true; game.applyEnvironment()
expect(env("MADEIRA_FASTSYNC") == "0" && env("MADEIRA_FASTSYNC_SEM") == "1", "Fastsync: the game's own switches are exported")
MadeiraConfig.values = ["inproc-sync": "1", "env.MADEIRA_FASTSYNC": "auto"]
unsetenv("MADEIRA_FASTSYNC"); unsetenv("MADEIRA_FASTSYNC_SEM"); game.applyEnvironment()
expect(env("MADEIRA_FASTSYNC") == nil && env("MADEIRA_FASTSYNC_SEM") == nil, "Madsync on: the game's fastsync switches are not exported")
MadeiraConfig.values = [:]; game.fastSync = nil; game.semaphoreFastPath = nil
game.reducedX87 = true; game.applyEnvironment()
expect(env("FEX_X87REDUCEDPRECISION") == "1", "reduced x87 exported when chosen")
game.reducedX87 = false

// Controller compatibility: known in-game preferences are pre-seeded,
// Automatic includes DirectInput, and Dock keeps Valve's injected overlay out.
let sampleRegistry = """
WINE REGISTRY Version 2

[Software\\\\Team Cherry\\\\Hollow Knight]
\"OtherSetting\"=dword:0000002a
\"NativeInput_h123\"=dword:00000000
"""
let mergedRegistry = ControllerCompatibility.mergedRegistry(sampleRegistry, section: "Software\\\\Team Cherry\\\\Hollow Knight", values: ["NativeInput": 1, "XInput": 1])
expect(mergedRegistry?.contains("\"NativeInput\"=dword:00000001") == true && mergedRegistry?.contains("\"XInput\"=dword:00000001") == true,
       "controller preferences are seeded before first launch")
expect(mergedRegistry?.contains("\"OtherSetting\"=dword:0000002a") == true && mergedRegistry?.contains("NativeInput_h123") == false,
       "controller preference merge preserves unrelated registry values and retires hashed stale values")
var hollow = LibraryEntry(title: "Hollow Knight", relativePath: "Program Files (x86)/Steam/steamapps/common/Hollow Knight", bits: 64)
hollow.steamAppID = 367520
MadeiraConfig.values = ["env.MADEIRA_CONTROLLER_AUTO_PREFS": "0"]
unsetenv("MADEIRA_DINPUT_PAD"); unsetenv("_MADEIRA_STEAM_OVERLAY_OFF")
hollow.applyEnvironment()
expect(env("MADEIRA_DINPUT_PAD") == "1", "Hollow Knight automatically gets DirectInput")
expect(env("_MADEIRA_STEAM_OVERLAY_OFF") == "1", "Dock marks the injected Steam overlay disabled")
hollow.controllerMode = "keys"; unsetenv("MADEIRA_DINPUT_PAD"); hollow.applyEnvironment()
expect(env("MADEIRA_DINPUT_PAD") == "0", "keyboard/mouse mode does not expose Hollow Knight through DirectInput")
MadeiraConfig.values = ["env.MADEIRA_CONTROLLER_AUTO_PREFS": "0", "env.MADEIRA_STEAM_OVERLAY": "1"]
unsetenv("_MADEIRA_STEAM_OVERLAY_OFF"); hollow.controllerMode = nil; hollow.applyEnvironment()
expect(env("_MADEIRA_STEAM_OVERLAY_OFF") == nil, "Steam overlay has an explicit opt-in")
MadeiraConfig.values = [:]
var balatro = LibraryEntry(title: "Balatro", relativePath: "Games/Balatro/Balatro.exe", bits: 64)
unsetenv("_MADEIRA_LUA51_GC64")
balatro.applyEnvironment()
expect(env("_MADEIRA_LUA51_GC64") == "1", "Balatro selects the GC64 LuaJIT compatibility runtime")
MadeiraConfig.values = ["env.MADEIRA_LUAJIT_GC64": "0"]; balatro.applyEnvironment()
expect(env("_MADEIRA_LUA51_GC64") == nil, "Balatro GC64 compatibility has a kill switch")
MadeiraConfig.values = [:]
let compatDrive = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0].appendingPathComponent("wine/drive_c")
let compatRelative = "Games/madeira-compat-" + UUID().uuidString
let compatFolder = compatDrive.appendingPathComponent(compatRelative)
try FileManager.default.createDirectory(at: compatFolder.appendingPathComponent("lib/py3-windows-x86_64"), withIntermediateDirectories: true)
defer { try? FileManager.default.removeItem(at: compatFolder) }
try Data().write(to: compatFolder.appendingPathComponent("CrossRealms.exe"))
try Data().write(to: compatFolder.appendingPathComponent("lib/py3-windows-x86_64/librenpython.dll"))
let compatRenpy = LibraryEntry(title: "CrossRealms", relativePath: compatRelative + "/CrossRealms.exe", bits: 64)
expect(ExternalGameCompatibility.prepare(compatRenpy).contains("renpy-renderer") && env("_MADEIRA_OPENGL_ANGLE_MODE") == "renpy", "nested RenPy DLLs select renderer route")
MadeiraConfig.values = ["env.MADEIRA_RENPY_RENDERER": "0"]
expect(!ExternalGameCompatibility.prepare(compatRenpy).contains("renpy-renderer"), "RenPy renderer policy opt-out")
MadeiraConfig.values = [:]
for file in ["UnityPlayer.dll", "GameAssembly.dll", "Replaced.exe"] { try Data().write(to: compatFolder.appendingPathComponent(file)) }
let compatReplaced = LibraryEntry(title: "REPLACED", relativePath: compatRelative + "/Replaced.exe", bits: 64)
expect(ExternalGameCompatibility.prepare(compatReplaced).contains("il2cpp-strict-memory"), "REPLACED IL2CPP strict memory policy")
expect(env("FEX_VECTORTSOENABLED") == "1" && env("FEX_MEMCPYSETTSOENABLED") == "1" && env("FEX_HALFBARRIERTSOENABLED") == "0", "vector/REP ordering and safe unaligned path")
expect(env("MADEIRA_REAL_SUSPEND") == "1", "REPLACED keeps native suspend holds instead of snapshot-only suspend")
var compatSteamUnity = LibraryEntry(title: "Steam Unity", relativePath: compatRelative, bits: 64)
compatSteamUnity.steamAppID = 3097560
expect(ExternalGameCompatibility.prepare(compatSteamUnity).contains("unity-mip-pressure") && env("MADEIRA_MIP_CLAMP_AUTO") == "1",
       "Steam folder launches receive the same pressure-aware texture protection as imported Unity games")
compatSteamUnity.unityOptimizations = false
_ = ExternalGameCompatibility.prepare(compatSteamUnity)
expect(env("MADEIRA_MIP_CLAMP_AUTO") == nil, "per-game opt-out clears the preceding automatic mip protection")
compatSteamUnity.unityOptimizations = nil
MadeiraConfig.values = ["env.MADEIRA_UNITY_OPTIMIZATIONS": "0"]
expect(!ExternalGameCompatibility.prepare(compatSteamUnity).contains("unity-mip-pressure"), "global Unity opt-out also covers Steam folder launches")
MadeiraConfig.values = ["env.MADEIRA_MIP_CLAMP_AUTO": "0"]
expect(!ExternalGameCompatibility.prepare(compatSteamUnity).contains("unity-mip-pressure"), "explicit texture policy is preserved for Steam folder launches")
MadeiraConfig.values = [:]
setenv("MADEIRA_MIP_CLAMP_AUTO", "0", 1)
_ = ExternalGameCompatibility.prepare(compatSteamUnity)
expect(env("MADEIRA_MIP_CLAMP_AUTO") == "0", "caller-supplied texture environment opt-out remains unchanged")
unsetenv("MADEIRA_MIP_CLAMP_AUTO")
_ = ExternalGameCompatibility.prepare(compatSteamUnity)
_ = ExternalGameCompatibility.prepare(game)
expect(env("MADEIRA_MIP_CLAMP_AUTO") == nil && env("_MADEIRA_AUTO_MIP_CLAMP") == nil,
       "an unrelated next game does not inherit Unity's automatic texture protection")
var replacedMemory = compatReplaced
replacedMemory.realSuspend = false; replacedMemory.swapMB = 0
replacedMemory.applyEnvironment()
expect(env("MADEIRA_REAL_SUSPEND") == "0" && env("_MADEIRA_GAME_SWAP_MB") == "0", "explicit per-game Off overrides REPLACED automatic suspension and shared swap")
game.applyEnvironment()
expect(env("_MADEIRA_GAME_REAL_SUSPEND") == nil && env("_MADEIRA_GAME_SWAP_MB") == nil && env("MADEIRA_REAL_SUSPEND") == nil, "next game does not inherit another game's overrides")
MadeiraConfig.values = ["env.FEX_VECTORTSOENABLED": "0"]
_ = ExternalGameCompatibility.prepare(compatReplaced)
expect(env("FEX_VECTORTSOENABLED") == nil, "explicit CPU config overrides strict profile")
MadeiraConfig.values = ["env.MADEIRA_IL2CPP_STRICT_MEMORY": "0"]
expect(!ExternalGameCompatibility.prepare(compatReplaced).contains("il2cpp-strict-memory") && env("FEX_MEMCPYSETTSOENABLED") == nil, "strict profile opt-out clears automatic values")
MadeiraConfig.values = [:]
_ = ExternalGameCompatibility.prepare(game)
expect(env("FEX_STRICTINPROCESSSPLITLOCKS") == nil && env("MADEIRA_REAL_SUSPEND") == nil && env("_MADEIRA_OPENGL_ANGLE_MODE") == nil, "next game clears compatibility policies")

// FPS limit: 30 needs DXMT's 30 FPS cap; without it a saved 30 runs as 60.
game.fpsMode = 3; game.applyEnvironment()
expect(vsync == 3, "30 FPS applied when DXMT has the cap")
ProMotionIntent.has30Cap = false; game.applyEnvironment()
expect(vsync == 1, "a saved 30 FPS runs as 60 without DXMT's 30 FPS cap")
ProMotionIntent.has30Cap = true; game.fpsMode = 1; game.applyEnvironment()
expect(vsync == 1, "60 FPS applied")
game.fpsMode = 2; game.applyEnvironment()
expect(vsync == 0 && game.fpsMode == 2, "saved RAW uses display maximum without modifying stored profile metadata")
game.fpsMode = 1

// Validation.
expect((try? game.validate()) != nil, "a normal profile validates")
fixtureRequiresWOW64 = 1
expect((try? game.validate()) == nil, "required32 is refused when the fixture bundle has no WoW64 runtime")
fixtureRequiresWOW64 = 0
expect((try? game.validate()) != nil, "native64 and AnyCPU do not require the x86 farm")
var bad = game; bad.arguments = "\"unbalanced"
expect((try? bad.validate()) != nil, "open final quotes follow Windows startup semantics")
bad = game; bad.arguments = (0..<17).map { "a\($0)" }.joined(separator: " ")
expect((try? bad.validate()) != nil, "commands beyond the former 16-argument cap validate")
bad.arguments = (0..<96).map { "a\($0)" }.joined(separator: " ")
expect((try? bad.validate()) != nil, "argument count follows command size instead of an unrelated fixed cap")
bad.arguments = String(repeating: "a", count: 1024)
expect((try? bad.validate()) != nil, "long store/engine arguments pass the actual native parser")
bad.arguments = String(repeating: "日", count: 32767)
expect((try? bad.validate()) == nil, "the actual C parser rejects a command beyond Windows' UTF-16 size limit")
bad = game; bad.resolution = "10x10"
expect((try? bad.validate()) == nil, "invalid size refused")
bad = game; bad.fpsMode = 7
expect((try? bad.validate()) == nil, "invalid frame limit refused")

// Library files: unknown keys (fields a newer or older build wrote) are ignored.
let json = """
{"id":"AF046C35-C32A-497B-92BC-0BBD14F8CB62","title":"T","relativePath":"a/b.exe","bits":64,"arguments":"",
 "resolution":"1024x768","fpsMode":3,"reducedX87":true,"fastSync":true,"liveLogs":false,"performance":false,
 "touchControls":true,"someNewerKey":42,"display":"fit","cpuCount":2}
"""
let decoded = try? JSONDecoder().decode(LibraryEntry.self, from: Data(json.utf8))
expect(decoded?.fpsMode == 3 && decoded?.reducedX87 == true && decoded?.touchControls == true,
       "decodes a file with unknown keys (including the fork's cpuCount and fastSync)")
expect(decoded?.displayMode == .fit && decoded?.controlOpacity == nil, "older files: Fit, default controls")
expect(decoded?.swapMB == nil && decoded?.realSuspend == nil, "older files inherit shared memory and suspension settings")
var memoryGame = game
memoryGame.swapMB = 4096; memoryGame.realSuspend = false
let memoryBack = (try? JSONEncoder().encode(memoryGame)).flatMap { try? JSONDecoder().decode(LibraryEntry.self, from: $0) }
expect(memoryBack?.swapMB == 4096 && memoryBack?.realSuspend == false, "per-game memory choices persist without changing library format")
memoryGame.swapMB = 513
expect((try? memoryGame.validate()) == nil, "invalid per-game swap capacity refused before launch")
// Written by the fork's app: display mode, control opacity/size (anisotropy is ignored).
let fork = """
{"id":"AF046C35-C32A-497B-92BC-0BBD14F8CB63","title":"T","relativePath":"a/b.exe","bits":32,"arguments":"",
 "resolution":"1560x720","display":"aspect","fpsMode":1,"reducedX87":true,"fastSync":true,"liveLogs":false,
 "performance":false,"touchControls":false,"controlOpacity":0.4,"controlSize":1.5,"anisotropyLimit":4,"extendedModes":false}
"""
let forked = try? JSONDecoder().decode(LibraryEntry.self, from: Data(fork.utf8))
expect(forked?.displayMode == .aspect && forked?.resolution == "1560x720", "display mode and resolution decode")
expect(forked?.controlOpacity == 0.4 && forked?.controlSize == 1.5, "control opacity and size decode")
expect(forked?.controlLayout == nil, "older files: no remembered controller layout")
expect(forked?.controlLayoutRevision == nil, "older profiles remain eligible for the Xbox default migration")
var picked = forked!; picked.controlLayout = "builtin.xbox"
picked.controlLayoutRevision = 1
let pickedBack = (try? JSONEncoder().encode(picked)).flatMap { try? JSONDecoder().decode(LibraryEntry.self, from: $0) }
expect(pickedBack?.controlLayout == "builtin.xbox", "a game remembers its controller layout")
expect(pickedBack?.controlLayoutRevision == 1, "later explicit layout choices survive profile encoding")
var odd = forked!; odd.display = "sideways"
expect(odd.displayMode == .fit, "an unknown display mode falls back to Fit")
let saved = try? JSONEncoder().encode(forked!)
let again = saved.flatMap { try? JSONDecoder().decode(LibraryEntry.self, from: $0) }
expect(again?.display == "aspect" && again?.controlSize == 1.5, "the profile encodes its display and control choices")

// Layout: the presented rect and the touch mapping for each mode.
let guest = CGSize(width: 1280, height: 720), view = CGRect(x: 0, y: 0, width: 844, height: 390)
func near(_ a: CGFloat, _ b: CGFloat) -> Bool { abs(a - b) < 0.5 }
let fit = GameSurfaceLayout.rect(guest: guest, bounds: view, mode: .fit)
expect(near(fit.height, 390) && near(fit.width, 693.3) && near(fit.minX, 75.3), "Fit letterboxes at the guest's shape")
let fill = GameSurfaceLayout.rect(guest: guest, bounds: view, mode: .fill)
expect(near(fill.width, 844) && near(fill.height, 474.75) && fill.minY < 0, "Fill covers the view and crops")
expect(GameSurfaceLayout.rect(guest: guest, bounds: view, mode: .stretch) == view, "Stretch is the view")
let drawn = CGSize(width: 1024, height: 768)
let aspect = GameSurfaceLayout.rect(guest: guest, aspect: drawn, bounds: view, mode: .aspect)
expect(near(aspect.width / aspect.height * 3, 4) && near(aspect.height, 390), "Aspect follows the drawn shape")
expect(GameSurfaceLayout.rect(guest: guest, bounds: view, mode: .aspect) == fit, "Aspect is Fit until a frame is drawn")
let centre = GameSurfaceLayout.map(point: CGPoint(x: 422, y: 195), guest: guest, bounds: view, mode: .fit)
expect(near(centre.x, 640) && near(centre.y, 360), "the centre maps to the guest's centre")
let bar = GameSurfaceLayout.map(point: CGPoint(x: 10, y: 10), guest: guest, bounds: view, mode: .fit)
expect(bar.x == 0 && near(bar.y, 18.5), "a touch in the letterbox clamps to the edge")
let corner = GameSurfaceLayout.map(point: CGPoint(x: 844, y: 390), guest: guest, bounds: view, mode: .stretch)
expect(corner.x == 1279 && corner.y == 719, "mapping clamps to the last guest pixel")
let phone = GuestDisplay.defaultMode(forLandscapeView: CGSize(width: 844, height: 390))
let tablet = GuestDisplay.defaultMode(forLandscapeView: CGSize(width: 1024, height: 768))
expect(phone.w == 1280 && phone.h == 720, "phone default mode is 1280x720")
expect(tablet.w == 1152 && tablet.h == 864, "4:3 default mode is 1152x864 (cheapest 4:3 of at least 0.9 MP)")
expect(DisplayMode.allCases.map { $0.label } == ["Fit", "Fill", "Stretch", "Aspect"], "the four Aspect & scaling choices")

// Controller navigation.
let c = LibraryController.shared
var got: [String] = []
let sub = c.commands.sink { got.append($0) }
func pump() { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
c.configure(enabled: true, ownsInput: true)
c.sample(buttons: 0x0001, lx: 0, ly: 0); c.sample(buttons: 0, lx: 0, ly: 0)
c.sample(buttons: 0, lx: 30000, ly: 0); c.sample(buttons: 0, lx: 0, ly: 0)
c.sample(buttons: 0x1000, lx: 0, ly: 0); c.sample(buttons: 0, lx: 0, ly: 0)
pump()
expect(got == ["up", "right", "accept"], "library owns input: d-pad, stick and A navigate (\(got))")
expect(c.ownsInput, "library owns input")
got = []
c.configure(enabled: true, ownsInput: false)
c.sample(buttons: 0x0010, lx: 0, ly: 0); c.sample(buttons: 0, lx: 0, ly: 0)
c.sample(buttons: 0x0030, lx: 0, ly: 0); c.sample(buttons: 0, lx: 0, ly: 0)
pump()
expect(got == ["menu"], "in a session only Back+Start opens the menu (\(got))")
expect(!c.ownsInput, "the game owns input in a session")
got = []
c.configure(enabled: false, ownsInput: true)
c.sample(buttons: 0x1000, lx: 0, ly: 0); pump()
expect(got.isEmpty && !c.ownsInput, "developer interface: no navigation")
_ = sub
expect(GameExitReport.message(status: 1, presented: true, expectsFrame: true)?.contains("code 1") == true,
       "application/Python exit code 1 is visible even after rendering")
expect(GameExitReport.message(status: 127, presented: false, expectsFrame: true)?.contains("code 127") == true,
       "startup application error retains its exit code")
expect(GameExitReport.message(status: 0xC0000005, presented: false, expectsFrame: true)?.contains("memory access violation") == true,
       "NTSTATUS crash reporting is retained")
expect(GameExitReport.message(status: 0, presented: false, expectsFrame: true)?.contains("before displaying") == true,
       "a zero exit before any frame no longer silently disappears")
expect(GameExitReport.message(status: nil, presented: false, expectsFrame: true)?.contains("before displaying") == true,
       "unknown early shutdown does not invent a Windows status")
expect(GameExitReport.message(status: 0, presented: true, expectsFrame: true) == nil,
       "normal gameplay exit does not create an error")
expect(GameExitReport.message(status: 0, presented: false, expectsFrame: false) == nil,
       "headless utility sessions do not get a first-frame warning")
expect(GameExitReport.message(status: 0x40010004, presented: false, expectsFrame: true) == nil,
       "informational Windows termination is not labeled a crash")
expect(GameExitReport.message(status: 0x80000003, presented: true, expectsFrame: true)?.contains("breakpoint") == true,
       "an unhandled assertion breakpoint is reported after a rendered game exits")
expect(GameExitReport.message(status: 1, presented: false, expectsFrame: true,
                             startupDialog: "SDL initialization failed")?.contains("SDL initialization failed") == true,
       "an observed startup dialog survives the return to the library")
var lightProfile = LibraryEntry(title: "REPLACED", relativePath: "Games/REPLACED/Replaced.exe", bits: 64)
lightProfile.resolution = "1920x1080"
lightProfile.applyEnvironment()
expect(env("MADEIRA_LIGHT_EFFECTS") == "1" && env("_MADEIRA_LIGHT_EFFECTS_PROFILE") == "1",
       "targeted graphics profile reaches the real launch environment")
expect(lightProfile.resolution == "1920x1080", "light filtering preserves the saved 1080p resolution")
let unsupportedLight = LibraryEntry(title: "Other", relativePath: "Games/REPLACED/Replaced.exe", bits: 32)
expect(!unsupportedLight.lightEffectsEnabled, "a 32-bit DLL farm is not advertised as the refreshed 64-bit filtering backend")
lightProfile.lightEffects = false; lightProfile.applyEnvironment()
expect(env("MADEIRA_LIGHT_EFFECTS") == "0", "explicit graphics opt-out wins over the target default")
var ordinaryProfile = LibraryEntry(title: "Other", relativePath: "Games/Other/Other.exe", bits: 64)
ordinaryProfile.applyEnvironment()
expect(env("MADEIRA_LIGHT_EFFECTS") == "0" && env("_MADEIRA_LIGHT_EFFECTS_PROFILE") == "0",
       "the next game does not inherit the preceding filtering profile")
exit(failed == 0 ? 0 : 1)
'''

c_src = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
static void madeira_session_priority_presented(unsigned pid) { (void)pid; }
'''
start = bridge.index('static uint64_t g_launch_exit')
end = bridge.index('static char *g_prefix_path')
c_src += 'void wine_launched_process_did_exit(int status);\nvoid wine_exit_status_reset(void);\nint wine_crash_exit_status(uint32_t *status);\nint wine_session_close_targets_thread(uint32_t pid, uint32_t tid);\n'
c_src += 'int wine_session_close_accept_target(uint32_t pid,uint32_t tid);\nint wine_session_close_has_exited(void);\nstatic uint32_t requested_stop;\nvoid ios_wineserver_request_game_stop(uint32_t pid) { requested_stop=pid; }\n'
exit_state = bridge[start:end]
# Name matching is Objective-C and has its own Foundation/iOS SDK fixture.
# Keep the actual atomic exit state/reporting in this portable C harness.
objc_start = exit_state.index('static pthread_mutex_t g_game_path_lock')
objc_end = exit_state.index('int wine_game_process_has_exited(')
exit_state = exit_state[:objc_start] + 'static uint32_t g_game_pid;\nstatic uint64_t g_game_exit;\n' + exit_state[objc_end:]
c_src += exit_state
c_src += r'''
typedef void *HWND;
typedef uintptr_t ULONG_PTR;
enum { WindowProcess, WindowThread };
static struct { struct { uintptr_t UniqueProcess, UniqueThread; } ClientId; } test_teb;
static HWND test_window;
static unsigned int test_window_pid, test_window_tid;
#define NtCurrentTeb() (&test_teb)
static HWND NtUserGetForegroundWindow(void) { return test_window; }
static ULONG_PTR NtUserQueryWindow(HWND hwnd, int kind) {
    (void)hwnd;
    return kind == WindowProcess ? test_window_pid : test_window_tid;
}
'''
c_src += block(driver, 'HWND winios_drv_session_close_target(int force)')
c_src += r'''
static int failed;
static void expect(int cond, const char *what) { printf("%s: %s\n", cond ? "PASS" : "FAIL", what); if (!cond) failed++; }
int main(void) {
    uint32_t status = 0;
    wine_exit_status_reset();
    expect(!wine_crash_exit_status(&status), "nothing recorded at the start of a session");
    expect(!wine_launched_process_exit_status(&status), "unknown status is distinct from a clean zero");
    expect(!wine_launched_process_has_exited() && !wine_session_close_has_exited(), "session exit flags start clear");
    expect(!wine_force_close_game_session(1), "no process is guessed before publication");
    wine_presented_process_track(180);
    wine_session_process_did_exit(28, 1);
    expect(!wine_presented_process_has_exited(), "helper exit does not end the rendered game");
    wine_session_process_did_exit(180, (int)0x80000003);
    expect(wine_presented_process_has_exited() && wine_presented_process_exit_status(&status) && status==0x80000003u,
           "a rendered Steam child's breakpoint exit is observable while its launcher stays alive");
    wine_presented_process_track(28);
    expect(wine_presented_process_exit_status(&status) && status==0x80000003u, "later surfaces do not erase a completed game's status");
    wine_exit_status_reset();
    expect(!wine_presented_process_has_exited() && !wine_presented_process_exit_status(&status), "next session clears rendered PID and exit status");
    wine_launched_process_started(24, 32);
    expect(!wine_force_close_game_session(0), "a Dock host is never used as a guessed close target");
    expect(wine_force_close_game_session(1) && requested_stop == 24, "direct game force-close is queued independently of its event pump");
    wine_exit_status_reset();
    expect(wine_session_close_accept_target(24, 32), "foreground game is selected for close");
    expect(!wine_session_close_accept_target(28, 36), "a helper cannot replace the close target");
    expect(!wine_session_close_accept_target(24, 36), "a renderer/worker cannot replace the GUI thread");
    expect(wine_session_close_targets_process(24) && !wine_session_close_targets_process(28), "force is scoped to the selected Windows PID");
    wine_session_process_did_exit(28, 0);
    expect(!wine_session_close_has_exited(), "helper exit cannot return to the home page");
    wine_session_process_did_exit(24, 0);
    expect(wine_session_close_has_exited(), "a Dock child exit ends an explicitly closing game session");
    wine_launched_process_did_exit(0);
    expect(wine_launched_process_exit_status(&status) && status == 0, "successful exit remains observable by the general API");
    expect(!wine_crash_exit_status(&status), "clean exit: no report");
    expect(wine_launched_process_has_exited(), "a successful worker-thread exit is observed without waiting for wineserver");
    wine_launched_process_did_exit(0x40010004);
    expect(!wine_crash_exit_status(&status), "an informational status is not an error");
    wine_launched_process_did_exit(1);
    expect(wine_launched_process_exit_status(&status) && status == 1, "Python/application exit code 1 is retained");
    expect(!wine_crash_exit_status(&status), "ordinary application failure is not mislabeled NTSTATUS");
    expect(wine_launched_process_exit_status(NULL), "general status API accepts NULL");
    wine_launched_process_did_exit((int)0xC0000005);
    expect(wine_crash_exit_status(&status) && status == 0xC0000005u, "the launched program's error status is recorded");
    expect(wine_crash_exit_status(NULL), "a NULL status pointer is allowed");
    wine_exit_status_reset();
    expect(!wine_crash_exit_status(&status), "reset clears the status");
    expect(!wine_launched_process_exit_status(&status), "reset clears validity of the general status too");
    expect(!wine_launched_process_has_exited() && !wine_session_close_has_exited(), "reset clears every exit flag");
    expect(wine_session_close_accept_target(28, 36), "a later session may select a different target");
    wine_exit_status_reset();
    test_window = (HWND)(uintptr_t)0x2002e;
    test_window_pid = 24; test_window_tid = 32;
    test_teb.ClientId.UniqueProcess = 28; test_teb.ClientId.UniqueThread = 36;
    expect(!winios_drv_session_close_target(0) && !winios_drv_session_close_target(1), "another process's pump cannot consume close/force");
    test_teb.ClientId.UniqueProcess = 24;
    expect(!winios_drv_session_close_target(1), "another Wine worker cannot perform process teardown");
    test_teb.ClientId.UniqueThread = 32;
    expect(winios_drv_session_close_target(0) == test_window, "GUI-owner thread resolves the real foreground window");
    test_window = NULL;
    expect(!winios_drv_session_close_target(0), "no WM_CLOSE target after the window is destroyed");
    expect(winios_drv_session_close_target(1) != NULL, "recorded GUI thread can finish force-close after window destruction");
    return failed ? 1 : 0;
}
'''

with tempfile.TemporaryDirectory() as tmp:
    sp = Path(tmp) / 'frontend.swift'
    sp.write_text(swift)
    if '--c-only' not in sys.argv:
        # Link the actual production validator, rather than teaching a Swift
        # mock a second interpretation of Windows arguments or size limits.
        validator = block(bridge, 'int wine_launch_arguments_status(')
        native_source = Path(tmp) / 'arguments.c'
        native_source.write_text('#include "WindowsArguments.h"\n' + validator)
        native_header = Path(tmp) / 'arguments.h'
        native_header.write_text(validator.split('{', 1)[0] + ';\n')
        native_object = Path(tmp) / 'arguments.o'
        subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-I', str(root/'app/Madeira'),
                        '-c', str(native_source), '-o', str(native_object)], check=True)
        swift_binary = Path(tmp) / 'frontend'
        r = subprocess.run(['swiftc', '-import-objc-header', str(native_header), str(sp),
                            str(native_object), '-o', str(swift_binary)], capture_output=True, text=True)
        if r.returncode == 0:
            r = subprocess.run([str(swift_binary)], capture_output=True, text=True)
        sys.stdout.write(r.stdout)
        if r.returncode:
            sys.stdout.write(r.stderr[-4000:])
            failures.append('swift harness')
    else:
        print('SKIP: Swift harness (--c-only); Codemagic runs the complete test')
    cp = Path(tmp) / 'exit.c'
    cp.write_text(c_src)
    exe = Path(tmp) / 'exit'
    r = subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-Wall', '-Werror', '-D_DEFAULT_SOURCE', '-o', str(exe), str(cp)],
                       capture_output=True, text=True)
    if r.returncode:
        sys.stdout.write(r.stderr[-4000:])
        failures.append('C harness build')
    else:
        r = subprocess.run([str(exe)], capture_output=True, text=True)
        sys.stdout.write(r.stdout)
        if r.returncode:
            failures.append('C harness')

wrapper = block(server, 'void process_exit_wrapper( int status )')
initial = wrapper[wrapper.index('    else\n    {'):]
check('wine_launched_process_did_exit( status );' in initial and wrapper.count('wine_launched_process_did_exit( status );') == 1,
      'ntdll reports only the initial (app-launched) process exit, from the no-slot branch')
check('__attribute__((weak))' in initial and 'ImagePathName' not in wrapper and 'wine_process_did_' not in wrapper,
      'the hook is weak and gets no image name')
check('madeira_exit_is_helper' not in bridge and '.exe"' not in block(bridge, 'void wine_launched_process_did_exit('),
      'no program-name list in the exit report')
check('MADEIRA_PROMOTE' not in fps and 'DisplayRateSettings' not in lib,
      'removed display-rate override cannot be enabled by saved settings')
# Capped display behavior is exercised by check-remote-support.py and the real
# UIKit fixture in check-capped-frame-intent.py, including the REPLACED session
# override. Matching the previous implementation's source text would reject
# that intentional change without testing behavior.
check('__attribute__((weak)) void madeira_set_display_max_fps' in shim and 'ProMotionIntent.has30Cap' in fps
      and 'ProMotionIntent.has30Cap || mode == 3' in lib, 'the 30 FPS cap is offered only with DXMT support')
check('LibraryView(play: launchLibraryEntry' in content, 'ContentView shows the library when it is the chosen interface')
check('runWineFullSequence(profile: entry' in content and 'profile.applyEnvironment()' in content,
      'library launches use the shared launch path with the profile applied')
check('_MADEIRA_LUA51_GC64_PATH' in loader and '[luajit-gc64] redirect' in loader
      and 'lua51-gc64.dll' in bridge, 'Balatro loads the bundled GC64 LuaJIT without replacing the game file')
check('Button("Use New Interface")' in content, 'the developer interface can switch back to the library')
check('LibraryController.shared' in gamepad and 'library.ownsInput' in gamepad,
      'player 1 pad drives the library and is neutral while the library owns input')

# The options restored from the fork's app.
detail = block(lib, 'struct LibraryDetail: View')
hud = block(lib, 'struct LibraryHUD: View')
model = block(lib, 'final class LibraryModel: ObservableObject')
check('Picker("Resolution", selection: $entry.resolution)' in detail and 'Desktop size' not in lib,
      'game details: one Resolution picker for every entry (not only the Desktop)')
check('screenShapeResolution' in detail and 'Text("Screen shape (' in detail and 'MADEIRA_SCREEN_SHAPE_RESOLUTION' in detail,
      'game details: Screen shape resolution choice')
check('Picker("Aspect & scaling"' in detail and 'entry.display = $0' in detail, 'game details: Aspect & scaling')
check('LabeledContent("Control opacity")' in detail and 'LabeledContent("Control size")' in detail,
      'game details: control opacity and size sliders')
check('Picker("Aspect & scaling", selection: $model.displayMode)' in hud and 'MADEIRA_SESSION_TOOLS' in hud,
      'in-game menu: Aspect & scaling (MADEIRA_SESSION_TOOLS)')
check('LabeledContent("Opacity")' in hud and 'LabeledContent("Size")' in hud, 'in-game menu: control opacity and size')
check('Text("Touch").tag("touch")' in lib and 'input.touchMode = value == "touch"' in lib,
      'pointer settings: Absolute, Relative and Touch')
save = block(model, 'func saveCurrentProfile()')
check('entry.display = displayMode.rawValue' in save and 'entry.controlOpacity = opacity' in save
      and 'entry.controlSize = controls.sizeScale' in save, 'a session saves display mode, opacity and size to the game')
begin = block(model, 'func begin(_ entry: LibraryEntry')
check('displayMode = entry.displayMode' in begin and 'controls.sizeScale =' in begin and 'opacity =' in begin,
      "a session starts with the game's display mode, opacity and size")
check('GuestDisplay.configureSessionDefault(' in block(lib, 'func configureLaunch('),
      'every launch sets the virtual monitor from the Resolution')
check('GameSurfaceLayout.rect(' in content and 'GameSurfaceLayout.map(' in content and 'effectiveDisplayMode()' in content
      and '* 1024 / r.width' not in content, 'the game view lays out and maps touches through GameSurfaceLayout')
check('if touchPointerMode { touchModeBegan(touches); return }' in content, 'the game view handles the Touch pointer mode')
check('TouchControlsModel.diameter(control)' in content and 'library.opacity' in content,
      "touch controls follow the session's size and opacity")

# Owner requests: the starting screen's glyph row, Settings credits last, no drive_c note.
launch = block(hud, 'private func launchView(')
glyph = block(hud, 'private func launchGlyph(')
row = block(launch, 'HStack(spacing: 14)')
check('launchGlyph(showLogs ? "Hide live log" : "Show live log", "text.alignleft", on: showLogs)' in row
      and 'Button(showLogs ?' not in launch,
      'starting screen: the live-log control is a glyph in one row')
check('Image(systemName: symbol)' in glyph and '.accessibilityLabel(label)' in glyph and 'Text(' not in glyph
      and 'Circle()' in glyph, 'starting screen: glyph buttons show no text and keep their words as VoiceOver labels')
settings = block(lib, 'private var settings: some View')
form = block(settings, 'Form {')
last = form[[m.start() for m in re.finditer(r'\bSection\b', form)][-1]:]
check('header: { Text("Credits") }' in last and form.count('Text("Credits")') == 1,
      'Settings: Credits is the last section')
for who in ('name: "Will Faust", handle: "willfaust"', 'name: "Nick", handle: "125hz"',
            'name: "Jfishin", handle: "Jfishin"', 'name: "Jesse", handle: "JesseLovelace"',
            'name: "Dan Perks", handle: "danperks"'):
    check('MadeiraCredit(' + who in last, 'Settings credits: ' + who)
check('https://github.com/\\(handle)' in block(lib, 'struct MadeiraCredit: View'),
      'a credit links the GitHub account')
check('complete application folders' not in lib and 'Section("Library")' not in settings,
      'Settings: the drive_c note is removed')

print('check-frontend:', 'FAIL' if failures else 'PASS')
sys.exit(1 if failures else 0)
