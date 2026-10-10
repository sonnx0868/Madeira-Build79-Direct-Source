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
   checks that only an NTSTATUS error of the launched program is recorded and
   that a reset clears it.
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
struct TouchControl: Codable, Equatable { var nx = 0.5 }
enum ControlAction: Codable, Equatable, Hashable { case none }   // LibraryEntry.controllerBinds
enum GamepadInput { static let keyboardMouseAvailable = true }   // LibraryEntry's per-game DirectInput choice
func wineserver_is_running() -> Int32 { 0 }
enum LibraryError: LocalizedError { case message(String) }
func env(_ name: String) -> String? { getenv(name).map { String(cString: $0) } }
'''
swift += block(lib, 'struct LibraryEntry: Codable, Identifiable') + '\n'
swift += block((root/'app/Madeira/TranslationTools.swift').read_text(encoding='utf-8'), 'enum CPUTranslationSettings') + '\n'
swift += block(lib, 'enum ControllerCompatibility') + '\n'
swift += block(lib, 'enum UnityLaunch') + '\n'
swift += block(lib, 'enum UnityStartupSync') + '\n'
swift += block(lib, 'enum ExternalGameCompatibility') + '\n'
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
expect(game.resolution == "1408x648", "new entries default to 1408x648")
expect(env("MADEIRA_SCREEN_W") == "1408" && env("MADEIRA_SCREEN_H") == "648" && env("MADEIRA_SCREEN_SRC") == "knob",
       "a direct game's resolution is exported as the session default")
game.resolution = "1560x720"; game.configureLaunch()
expect(env("MADEIRA_SCREEN_W") == "1560" && env("MADEIRA_SCREEN_H") == "720" && published == (1560, 720),
       "a screen-shape resolution is exported and published")
expect((try? game.validate()) != nil, "a screen-shape resolution validates")
game.resolution = "1280x720"; game.configureLaunch()

// A Steam game: Madeira Dock sets what starts, so its profile leaves MADEIRA_EXE alone...
var steamGame = LibraryEntry(title: "Steam game", relativePath: "Program Files (x86)/Steam/steamapps/common/Some Game", bits: 0)
steamGame.steamAppID = 4242
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
expect(env("MADEIRA_DINPUT_PAD") == "1", "Automatic controller mode publishes DirectInput beside XInput")
game.controllerMode = "xinput"; game.applyEnvironment()
expect(env("MADEIRA_DINPUT_PAD") == "0", "XInput-only mode suppresses the duplicate DirectInput view")
game.controllerMode = nil
expect(env("MADEIRA_FASTSYNC") == "auto" && env("MADEIRA_FASTSYNC_SEM") == "0",
       "no sync keys (Fastsync, the default): the game's fastsync switches are exported")
expect(LogStore.shared.lines.last == "[display-shape] resolution=1280x720 mode=fit", "the profile's display shape is logged")
// Fastsync's per-game switches: exported only when Settings chose Fastsync.
MadeiraConfig.values = ["inproc-sync": "0"]
unsetenv("MADEIRA_FASTSYNC"); unsetenv("MADEIRA_FASTSYNC_SEM"); game.applyEnvironment()
expect(env("MADEIRA_FASTSYNC") == nil && env("MADEIRA_FASTSYNC_SEM") == nil, "Wine standard sync: no fastsync switches")
MadeiraConfig.values = ["inproc-sync": "0", "env.MADEIRA_FASTSYNC": "auto"]
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
// FPS limit: 30 needs DXMT's 30 FPS cap; without it a saved 30 runs as 60.
game.fpsMode = 3; game.applyEnvironment()
expect(vsync == 3, "30 FPS applied when DXMT has the cap")
ProMotionIntent.has30Cap = false; game.applyEnvironment()
expect(vsync == 1, "a saved 30 FPS runs as 60 without DXMT's 30 FPS cap")
ProMotionIntent.has30Cap = true; game.fpsMode = 1; game.applyEnvironment()
expect(vsync == 1, "60 FPS applied")

// Validation.
expect((try? game.validate()) != nil, "a normal profile validates")
var bad = game; bad.arguments = "\"unbalanced"
expect((try? bad.validate()) == nil, "unbalanced quotes refused")
bad = game; bad.arguments = (0..<17).map { "a\($0)" }.joined(separator: " ")
expect((try? bad.validate()) == nil, "more than the bridge's 16 arguments refused")
bad.arguments = (0..<16).map { "a\($0)" }.joined(separator: " ")
expect((try? bad.validate()) != nil, "the bridge's full argument capacity validates")
bad.arguments = String(repeating: "a", count: 1024)
expect((try? bad.validate()) == nil, "a command exceeding the bridge's buffer is refused")
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
var picked = forked!; picked.controlLayout = "builtin.xbox"
let pickedBack = (try? JSONEncoder().encode(picked)).flatMap { try? JSONDecoder().decode(LibraryEntry.self, from: $0) }
expect(pickedBack?.controlLayout == "builtin.xbox", "a game remembers its controller layout")
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
exit(failed == 0 ? 0 : 1)
'''

c_src = r'''
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
'''
start = bridge.index('static uint64_t g_launch_exit')
end = bridge.index('static char *g_prefix_path')
c_src += 'void wine_launched_process_did_exit(int status);\nvoid wine_exit_status_reset(void);\nint wine_crash_exit_status(uint32_t *status);\nint wine_session_close_targets_thread(uint32_t pid, uint32_t tid);\n'
c_src += 'int wine_session_close_accept_target(uint32_t pid,uint32_t tid);\nint wine_session_close_has_exited(void);\nstatic uint32_t requested_stop;\nvoid ios_wineserver_request_game_stop(uint32_t pid) { requested_stop=pid; }\n'
c_src += bridge[start:end]
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
    expect(!wine_launched_process_has_exited() && !wine_session_close_has_exited(), "session exit flags start clear");
    expect(!wine_force_close_game_session(1), "no process is guessed before publication");
    wine_launched_process_started(24, 32);
    expect(!wine_force_close_game_session(0), "a Dock host is never used as a guessed close target");
    expect(wine_force_close_game_session(1) && requested_stop == 24, "direct game force-close is queued independently of its event pump");
    wine_exit_status_reset();
    expect(wine_session_close_accept_target(24, 32), "foreground game is selected for close");
    expect(!wine_session_close_accept_target(28, 36), "a helper cannot replace the close target");
    expect(!wine_session_close_accept_target(24, 36), "a renderer/worker cannot replace the GUI thread");
    expect(wine_session_close_targets_process(24) && !wine_session_close_targets_process(28), "force is scoped to the selected Windows PID");
    wine_session_process_did_exit(28);
    expect(!wine_session_close_has_exited(), "helper exit cannot return to the home page");
    wine_session_process_did_exit(24);
    expect(wine_session_close_has_exited(), "a Dock child exit ends an explicitly closing game session");
    wine_launched_process_did_exit(0);
    expect(!wine_crash_exit_status(&status), "clean exit: no report");
    expect(wine_launched_process_has_exited(), "a successful worker-thread exit is observed without waiting for wineserver");
    wine_launched_process_did_exit(0x40010004);
    expect(!wine_crash_exit_status(&status), "an informational status is not an error");
    wine_launched_process_did_exit((int)0xC0000005);
    expect(wine_crash_exit_status(&status) && status == 0xC0000005u, "the launched program's error status is recorded");
    expect(wine_crash_exit_status(NULL), "a NULL status pointer is allowed");
    wine_exit_status_reset();
    expect(!wine_crash_exit_status(&status), "reset clears the status");
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
        r = subprocess.run(['swift', str(sp)], capture_output=True, text=True)
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
check('__attribute__((weak))' in initial and 'ImagePathName' not in server and 'wine_process_did_' not in server,
      'the hook is weak and gets no image name')
check('madeira_exit_is_helper' not in bridge and '.exe"' not in bridge[bridge.index('static uint64_t g_launch_exit'):bridge.index('static char *g_prefix_path')],
      'no program-name list in the exit report')
check('MADEIRA_PROMOTE' not in fps and 'DisplayRateSettings' not in lib,
      'removed display-rate override cannot be enabled by saved settings')
check('if mode == 1 || mode == 3 { return 0 }' in fps, 'capped 60/30 FPS sessions release the display link')
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
