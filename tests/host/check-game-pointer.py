#!/usr/bin/env python3
"""Regressions for game pointer coordinates, mixed button streams and capture.

Source wiring runs on every host. --require-swift makes production Swift
behavior tests mandatory on Codemagic; no iOS device is simulated here.
"""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
hardware = (root / "app/Madeira/HardwareInput.swift").read_text(encoding="utf-8")
view = (root / "app/Madeira/ContentView.swift").read_text(encoding="utf-8")
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
display = (root / "app/Madeira/GuestDisplay.swift").read_text(encoding="utf-8")

absolute = hardware.split("private func postAbsolute(", 1)[1].split("private func noteDelivery", 1)[0]
assert "game.mapPoint(point)" in absolute and "ScreenMap.toScreen" not in absolute
assert "func mapPoint(" in view and "GameSurfaceLayout.map(point: p, guest: guestSize()" in view
overlay = hardware.split("final class DirectCursorOverlay", 1)[1].split("final class PadStickMouse", 1)[0]
assert "winios_screen_size(&sw, &sh)" in overlay and "static let screenW = 1024" not in overlay
buttons = hardware.split("func uikitButtons(", 1)[1].split("func uikitScroll(", 1)[0]
assert "!gcLive" not in buttons and "buttonStreams.acceptsUIKit(b)" in buttons
assert "buttonStreams.acceptsGC(b, pressed: pressed, at: t)" in hardware
assert "clickFocus.pointerRoute(" in hardware
assert "updateHardwareResponder(active: keyboard && keyboardConnected)" in hardware
assert "hardwareResponder ? hardwareInputView : super.inputView" in view
assert "guard !LibraryModel.shared.blocksGameplayTouch" in hardware
assert library.count("HardwareInput.shared.sessionUIChanged()") == 3
assert "menu || launching || quitting" in library
assert "if !base && pointerLocked" in hardware
assert "LibraryModel.shared.current != nil && !Self.desktopMode" in hardware
assert "if lockedByUs || captureGame { autoLockSuppressed = true }" in hardware
assert "locked: pointerCaptured" in hardware
assert "UIPointerLockState.didChangeNotification" in hardware
assert "let captured = state?.isLocked ?? false" in hardware
lock = hardware.split("enum PointerLock {", 1)[1].split("final class PointerHider", 1)[0]
assert "window === gameWindow" in lock
for overlay_window in ("PassthroughWindow", "ControlsWindow", "LibraryKeyboardWindow"):
    assert f"window is {overlay_window}" in lock, f"pointer lock misses {overlay_window}"
assert "window.isKeyWindow }) ?? gameWindow" not in lock
assert "owners = nextOwners" in lock and "targets != nextTargets" in lock
assert "current.childViewControllerForPointerLock" in lock
assert "viewIfLoaded?.window?.windowScene" not in lock
assert "return original(object, sel)" in lock, "unowned controllers must keep their preference"
assert "pointer-lock-target" in lock and "pointer-lock-preference" in lock
assert "UIWindow.didBecomeVisibleNotification" in hardware
assert "UIWindow.didBecomeHiddenNotification" in hardware
quit_body = library.split("func requestQuit() {", 1)[1].split("func saveCurrentProfile()", 1)[0]
assert "winios_request_session_close(0)" in quit_body
assert "winios_request_session_close(1)" in quit_body
assert "winios_post_key" not in quit_body, "Quit must not depend on an ignored Alt+F4"
assert "self.current == session" in quit_body, "a stale close timer must not reach a later session"
assert "wine_session_close_has_exited() != 0" in library
hud = library.split("struct LibraryHUD: View {", 1)[1].split("struct LibraryLiveLogs: View", 1)[0]
assert ".sheet(isPresented: $diagnosticSheet)" in hud
assert "DiagnosticUploadView(gameTitle: model.activeEntry?.title)" in hud
assert 'Label("Send diagnostic log"' in hud
controls_window = view.split("final class ControlsWindow: UIWindow", 1)[1].split("enum TouchControlsHost", 1)[0]
assert "if ControlPresetsModel.enabled, let root" not in controls_window
assert "library.quitting" in controls_window
print("PASS: game mapping, button ownership, focus/menu release and multi-window lock wiring")

compiler = shutil.which("swiftc")
if not compiler:
    if "--require-swift" in sys.argv:
        raise SystemExit("swiftc is required for the game pointer behavior tests")
    print("SKIP: Swift behavior tests need swiftc; Codemagic requires them")
    raise SystemExit(0)

pure = hardware.split("// MARK: - Pure input mapping", 1)[1].split("// MARK: - Device glue", 1)[0]
geometry = display.split("/// The session default of the guest's virtual monitor.", 1)[0]
checks = r'''
// Clicks align with the rendered surface at 1440p, after a mode change, and
// with Fit/Fill/Stretch/Aspect. Test points also go well beyond old 1024x768.
let bounds = CGRect(x: 10, y: 20, width: 1366, height: 1024)
for guest in [CGSize(width: 2560, height: 1440), CGSize(width: 1920, height: 1080)] {
    for mode in DisplayMode.allCases {
        let aspect = CGSize(width: 1408, height: 648)
        let rect = GameSurfaceLayout.rect(guest: guest, aspect: aspect, bounds: bounds, mode: mode)
        for fractions in [(0.25, 0.5), (0.75, 0.25), (0.9, 0.9)] {
            let point = CGPoint(x: rect.minX + rect.width * fractions.0,
                                y: rect.minY + rect.height * fractions.1)
            let mapped = GameSurfaceLayout.map(point: point, guest: guest, aspect: aspect, bounds: bounds, mode: mode)
            assert(abs(mapped.x - guest.width * fractions.0) < 0.001)
            assert(abs(mapped.y - guest.height * fractions.1) < 0.001)
        }
    }
}

// Drive the actual stream selector and held-edge tracker. Movement is
// independent: an M3-like mouse may report motion/left via GC and right via UI.
struct MouseHarness {
    var streams = MouseButtonStreams()
    var held: Set<MouseButton> = []
    var posted = HeldEdges<MouseButton>()
    var edges: [String] = []
    mutating func flush() {
        let changes = posted.update(held)
        edges += changes.up.map { "\($0.rawValue) up" }
        edges += changes.down.map { "\($0.rawValue) down" }
    }
    mutating func ui(_ want: Set<MouseButton>, at t: Double) {
        streams.noteUIKit(want, at: t)
        for b in MouseButton.allCases where streams.acceptsUIKit(b) {
            if want.contains(b) { held.insert(b) } else { held.remove(b) }
        }
        flush()
    }
    mutating func gc(_ b: MouseButton, _ down: Bool, at t: Double) {
        if streams.acceptsGC(b, pressed: down, at: t) {
            if down { held.insert(b) } else { held.remove(b) }
            flush()
        }
    }
}
var mixed = MouseHarness()
mixed.gc(.left, true, at: 1); mixed.ui([.left], at: 1.01)
mixed.gc(.left, false, at: 1.02); mixed.ui([], at: 1.03)
mixed.ui([.right], at: 2); mixed.ui([], at: 2.01)
assert(mixed.edges == ["0 down", "0 up", "1 down", "1 up"])

// UIKit can win the first click. Every crossing order still produces one
// down/up, even if GC UP precedes (or replaces a missing) UIKit UP.
for releaseGCFirst in [false, true] {
    var mouse = MouseHarness()
    mouse.ui([.left], at: 1); mouse.gc(.left, true, at: 1.01)
    if releaseGCFirst {
        mouse.gc(.left, false, at: 1.02); mouse.ui([], at: 1.03)
    } else {
        mouse.ui([], at: 1.02); mouse.gc(.left, false, at: 1.03)
    }
    mouse.gc(.left, true, at: 2); mouse.gc(.left, false, at: 2.01)
    assert(mouse.edges == ["0 down", "0 up", "0 down", "0 up"])
}
var late = MouseHarness()
late.ui([.right], at: 1); late.ui([], at: 1.01)
late.gc(.right, true, at: 1.02); late.gc(.right, false, at: 1.03)
assert(late.edges == ["1 down", "1 up"])
var missingUp = MouseHarness()
missingUp.ui([.left], at: 1); missingUp.gc(.left, true, at: 1.01)
missingUp.gc(.left, false, at: 1.02)
assert(missingUp.posted.down.isEmpty)

// A hover ending as the button goes down does not lose the click. Wait for
// the actual UIKit hit target; an outside click/old finger tap stays outside.
var focus = ClickFocus()
assert(focus.pointerRoute(buttonAt: 1, now: 1, pointerOver: false) == nil)
focus.touchBegan(onGame: true, at: 1.01)
assert(focus.pointerRoute(buttonAt: 1, now: 1.061, pointerOver: false) == true)
focus.touchBegan(onGame: false, at: 2.01)
assert(focus.pointerRoute(buttonAt: 2, now: 2.061, pointerOver: true) == false)
assert(focus.pointerRoute(buttonAt: 10, now: 10.061, pointerOver: false) == false)
assert(focus.pointerRoute(buttonAt: 10, now: 10, pointerOver: true) == true)

func capture(_ locked: Bool = false, focused: Bool = true, over: Bool = true,
             motion: Double = 0.1, game: Bool = true) -> AutoLock.Action {
    AutoLock.decide(locked: locked, lockedByUs: locked, cursorShown: true, hiddenFor: 0,
                    pointerOver: over, focused: focused, sinceReport: 5, sinceMotion: motion,
                    captureGame: game)
}
assert(capture() == .lock)                                // visible game cursor is contained
assert(capture(true, over: false, motion: 5) == .none)      // no hover/quiet reports while locked
assert(capture(true, focused: false) == .unlock)           // menu, launch screen or background
assert(capture(true, game: false) == .unlock)              // game session ended
assert(capture(focused: false) == .none)
assert(capture(over: false) == .none)
assert(capture(motion: 5) == .none)
assert(capture(game: false) == .none)                      // no visible-cursor lock on desktop
// A denied/pending iPadOS lock must retain absolute hover routing. Once the
// OS confirms capture (hover disappears), raw relative motion carries input.
assert(PointerPolicy.route(focused: true, hover: true, locked: false,
                           cursorShown: true, absoluteAllowed: true) == .absolute)
assert(PointerPolicy.route(focused: true, hover: false, locked: true,
                           cursorShown: true, absoluteAllowed: true) == .relative)
let escape = HardwareKeyMap.stroke(forHIDUsage: HardwareKeyMap.canonicalPressUsage(669))!
assert(escape.vk == 0x1b && escape.scan == 0x01 && !escape.extended)
print("PASS: production pointer geometry, single click edges, hit focus, capture/release and Globe mapping")
'''
with tempfile.TemporaryDirectory(prefix="madeira-game-pointer-") as folder:
    source = Path(folder) / "main.swift"
    binary = Path(folder) / "check"
    source.write_text("import Foundation\n#if canImport(CoreGraphics)\nimport CoreGraphics\n#endif\n" + pure + geometry + checks, encoding="utf-8")
    subprocess.run([compiler, str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
