# Hardware keyboard, mouse and trackpad

A Bluetooth or USB keyboard and mouse (or an iPad trackpad) reach Windows
programs as real key and mouse events: held keys with their own up events,
modifiers as keys, mouse motion, five mouse buttons and the wheel. The mouse
has a visible cursor, the program's own, and input reaches the program only
while it has focus. Everything is in `app/Madeira/HardwareInput.swift`, with the
cursor report in `app/Madeira/Winios/WiniosCursor.c` and
`build/win32u-unix/driver_ios.c`; controllers stay with `GamepadInput.swift`
(see [CONTROLLERS.md](CONTROLLERS.md)).

## Keyboard

`GCKeyboard` is the primary path and delivers one callback per physical key
transition. The game view also accepts UIKit physical-key presses as a
fallback for OS/device combinations whose `GCKeyboard.keyboardInput` is nil or
incomplete. Both paths use the USB HID usage and are de-duplicated before Wine,
so a key never arrives twice. UIKit text events remain separate and are used
only for keys the physical map does not understand.

During play, the game view becomes first responder while a hardware keyboard
is connected, with an empty input view so no software keyboard opens. Native
menus, launch screens and other text fields keep their own keyboard focus.

- The key's USB HID usage is mapped to a Windows virtual key. Modifiers keep
  their side (`VK_LSHIFT`, `VK_RCONTROL`, ...); the wineserver derives the
  generic `VK_SHIFT`/`VK_CONTROL`/`VK_MENU` state from them. Numpad keys are
  sent as `VK_NUMPAD0`-`VK_NUMPAD9`/`VK_DECIMAL` (Num Lock on).
- Common physical keys carry their PC/AT set-1 scan code and E0 bit directly
  from the HID usage, so Win32 messages, Raw Input and DirectInput all see the
  same physical key. This preserves distinctions a virtual key alone loses,
  especially main Enter versus numpad Enter and dedicated arrows versus the
  numpad. Uncommon locale-specific keys fall back to Wine's active-layout
  mapping (`winios_drv_post_key` in `build/win32u-unix/driver_ios.c`).
- While a hardware keyboard is connected, the text bridge (the keyboard button,
  `UIKeyInput` on the game view) ignores typed text, because the same presses
  already arrived raw.
- Known layout-fallback differences, all pinned by the host test: the Japanese
  yen key shares the backslash key; keypad `=`, keypad comma,
  Select and the Japanese/Korean IME keys have no scan code in the US layout.
  Execute, Stop, Again, Undo, Cut, Copy, Paste and Find have no Windows virtual
  key and are not sent (the unmapped usage is logged).
- The iPad keyboard's **Globe/Language** key is normalized from Apple's raw
  usage 669 to USB Escape, so Windows games receive `VK_ESCAPE`/scan `0x01`.
  This applies when iPadOS delivers the key to Madeira. If Globe still opens
  the system language/emoji UI, set **Settings > General > Keyboard > Hardware
  Keyboard > Modifier Keys > Globe Key > Escape** on the iPad. The resulting
  physical Escape uses the same raw-key bridge; Madeira cannot remap a system
  key that iPadOS does not deliver to the app.

## Focus

GameController reports the keyboard and mouse to the app whatever the user is
doing in it. Input reaches the program only while it has focus:

- **Always required:** the app is active and in the foreground, the game view
  is on screen, and nothing is presented over the app (a sheet, an alert, a
  picker, a menu shown as a view controller).
  Madeira's own in-game menu and launch screen also suspend hardware input
  and release pointer lock immediately.
- **Keyboard:** no other text input is first responder, for example a text
  field in the app. The game view's own text bridge counts as the program's.
- **Mouse:** the pointer is over the game view (iPad, where UIKit reports the
  pointer's position); or, where it does not (iPhone AssistiveTouch), the last
  click or tap in the app's window landed on the game view; or pointer lock is
  on; or a button that went down on the game view is still held (a drag that
  leaves the view keeps the mouse until the button is released).
- **Right stick controls mouse:** the "always required" part.

On iPhone, AssistiveTouch turns a click into a tap at its cursor that arrives
within milliseconds of the GCMouse button report, before or after it. A button
press waits up to 60 ms for that tap and goes to the program only if the tap
landed on the game view, so a click on the app's own buttons does not also
click in the program.

When the program loses focus, everything it was told is held is released: a
key-up for each held key and a button-up for each held button. A key or button
pressed while the program did not have focus stays away from it until it is
released, even if focus comes back first. Focus is re-evaluated on each key and
button, when the pointer enters or leaves the game view, on taps, when the app
changes state, when a text field or text view starts or ends editing, when a
window becomes or stops being key, and four times a second while a keyboard,
mouse or controller is attached. Everything held is also released on a
device disconnect or app deactivation. A memory warning deliberately does not
change input state, because it can arrive in the middle of play.

Not detected: a menu that is neither presented as a view controller nor takes
first responder. Upstream's developer layout has none.

## Mouse and trackpad

Two paths carry the mouse; whichever delivers a delta first wins and is logged:

- **GCMouse** (iPad; iPhone with AssistiveTouch): raw HID deltas on a dedicated
  high-priority queue.
- **UIKit indirect pointer** (a pointer that GameController enumerates but that
  never reports): hover, pointer drag and scroll recognisers on the game view,
  and indirect-pointer touches read as buttons from `UIEvent.buttonMask`. This
  needs `UIApplicationSupportsIndirectInputEvents` in `Info.plist`, which this
  change adds. It is best effort: iOS clamps the unlocked pointer at the screen
  edge and has no API to re-centre it.

How motion reaches the program depends on the program's cursor:

- **The program shows a cursor, and UIKit reports the pointer's position**
  (iPad, pointer not locked; the desktop session always counts as showing
  one): the program's cursor is put where the pointer is on the game view
  (absolute moves), mapped exactly as a touch is. The drawn cursor therefore
  sits under the hidden iOS pointer and never drifts away from it, and it stops
  at the edge where the pointer leaves the game view.
- **Otherwise** (the program hides its cursor, as mouse-look does; the pointer
  is locked; iPhone): motion is sent **relative** (`MOUSEEVENTF_MOVE` without
  `ABSOLUTE`). The wineserver adds it to its own cursor and raw input receives
  the delta before any `ClipCursor` clamping, so mouse-look never stalls at a
  screen edge.

Buttons are left, right, middle and the two side buttons
(`XBUTTON1`/`XBUTTON2`); continuous scrolling is summed into wheel notches of
120, vertical and horizontal.

Button ownership is selected separately for each button. A working GCMouse
motion stream does not disable UIKit clicks; a button moves to GCMouse once
that button's callbacks arrive. The first overlapping UIKit/GCMouse click is
merged into one down/up pair. If hover ends as a button goes down, the bridge
waits up to 60 ms for the click's actual hit target rather than dropping it.

Absolute pointer positions use the game view's touch mapping, including the
live virtual monitor resolution and Fit/Fill/Stretch/Aspect. The cursor overlay
also reads the live monitor size, instead of assuming a 1024x768 screen.

Mouse samples and key/button edges share the iOS-to-Wine input queue. Adjacent
pure mouse moves are coalesced (relative deltas are summed; absolute motion
keeps the latest position), while keys, buttons and wheel events retain exact
ordering. The queue is 1024 entries, and if it ever fills, a key or button edge
displaces an old motion sample instead of being dropped. This prevents a
high-polling-rate mouse from losing a key-up or button-up and leaving an input
stuck. Per-event queue logging is removed from the hot path; aggregate queue
statistics are logged periodically.

**Gain:** relative mouse motion has its own sensitivity (`sensMouse`, default
1.0 = the device's deltas unchanged).

**iPhone:** pointer devices are routed only through AssistiveTouch (Settings >
Accessibility > Touch > AssistiveTouch > On, then Devices). If a mouse is
enumerated but reports nothing for 10 s, the app shows this path once. With
AssistiveTouch, a click also arrives as a synthesised finger touch at the
AssistiveTouch cursor, which would snap the program's cursor there. While the
mouse is in use (a GCMouse delta in the last 2 s), touches on the game view
with no contact patch, or within 50 ms of a GCMouse button change, are
dropped. The first 30 classifications are logged. Set
`"ignoreTouchesWithMouse": false` in `Documents/madeira-input.json` to turn the
filter off. The on-screen touch controls are not filtered. AssistiveTouch's own
cursor cannot be hidden by an app; the program's drawn cursor is the one that
shows where a click lands.

## Cursor

The iOS pointer is hidden over the game view; what shows is the program's own
cursor:

- **Desktop session** (`MADEIRA_DESKTOP=1`): the compositor draws it, as
  before. On iPad it follows the pointer; with relative motion it follows the
  motion, and the touch trackpad continues from the same position.
- **A program on the game view** (no virtual desktop): nothing drew a Windows
  cursor before, because the finger is the pointer there. Now, while the mouse
  is in use, the app draws the program's cursor over the game view:
  - the driver sends the program's cursor image and hotspot (the same
    extraction as for the desktop compositor), whether the program hides its
    cursor (`ShowCursor(FALSE)`, `SetCursor(NULL)`), and where Wine's cursor is
    after every posted move and after `SetCursorPos`. It changes nothing the
    program sees, and does nothing until the app has enabled it;
  - the cursor is hidden whenever the program hides its own (mouse-look, or a
    program that draws its own cursor), while the mouse does not have focus,
    and after a finger touches the game view, until the mouse moves again;
  - until a program has reported a cursor, none is drawn.

## Pointer lock (iPad)

Pointer lock hides and pins the iPadOS pointer, so GCMouse deltas keep
arriving at the screen edges and the pointer cannot wander onto the app's own
buttons. iPadOS honours it only while Madeira is full screen, and only on the
GCMouse path (on the UIKit path locking would stop pointer delivery, so it is
refused). iPhone has no lockable pointer and shows no lock control.

The requested preference and actual iPadOS capture are tracked separately.
Input routing uses `UIScene.pointerLockState.isLocked`, observed through
`UIPointerLockState.didChangeNotification`. `[hwinput] pointer-lock-state`
reports requested/captured, availability, activation, owner count,
AssistiveTouch status and scene/screen size;
`requested=1 captured=0` means iPadOS has not granted the lock. The lock button
shows a waiting glyph until capture is confirmed. The game window and Madeira's
non-key joystick/controls/HUD/keyboard overlay windows share the preference;
updating only the key window can leave a higher overlay root requesting no lock.
Roots and their explicit pointer-lock delegates are refreshed after window,
controller or scene changes. Ownership comes from the window, not a delegate's
view (which may not be attached yet). Unregistered controller instances and
system windows keep their original preferences.

`[hwinput] pointer-lock-target` identifies each controller sent an update,
including its window level and key status. `pointer-lock-preference value=1`
means that controller's getter was actually consulted with a lock preference;
it is still **not** proof of capture. Only `pointer-lock-state ... captured=1`
confirms iPadOS containment. A fullscreen active scene with preference queries
but `captured=0` needs further device investigation, not a cursor-coordinate clamp.

- **Library games:** as soon as raw GCMouse motion is confirmed and the
  mouse is moving over the game view, capture also applies to games with a
  visible cursor. It stays captured in the game's own menus. Madeira's menu,
  launch screen, backgrounding, disconnect and ending the session release it.
  Press **Ctrl+Alt+P**, or touch the lock button, to release it by hand; manual
  release keeps automatic capture off for the rest of that game session.
- **Other direct programs:** while a program on the game view hides its cursor (for half a
  second, so a program about to show one does not lock), the pointer is over
  the game view and the mouse is moving, the pointer is locked, as a PC game
  captures the mouse. The lock is released as soon as the program shows a
  cursor or when the program loses focus. A quiet cursor-report stream does
  not unlock: many games stop changing cursor state during mouse-look, and
  releasing there would let iPadOS reclaim motion at a screen edge.
  `MADEIRA_POINTER_AUTOLOCK=0` turns this off.
- **By hand:** the lock button in the pointer settings or **Ctrl+Alt+P** on the
  keyboard (P is not sent to the program). Touch keeps working while locked, so
  the button is always a way out. Releasing an automatic lock by hand keeps it
  off for the library session, or until the cursor next changes visibility
  for another direct program.
- The lock is released when the mouse disconnects.

## Right stick controls mouse

Optional and off by default (`padRightStickMouse`). While on and a controller
is connected, its right stick moves the mouse at a velocity (360 counts per
second at full deflection, scaled by the Relative pointer sensitivity; 15%
dead zone), in every pointer mode. It is meant for programs with no controller
support: a program that reads the controller itself (XInput or DirectInput)
already gets the right stick, and moving the mouse too would turn the camera
twice. XInput keeps receiving the stick either way.

## Settings

In the developer layout, open the pointer panel (the cursor button). Below the
key row it then shows, for attached devices only:

- the mouse sensitivity slider and, on iPad, the pointer lock button;
- the **Right stick controls mouse** toggle when a controller is connected.

The values are stored in `Documents/madeira-input.json` with the other pointer
settings: `sensMouse`, `padRightStickMouse`, `ignoreTouchesWithMouse`.

## Switches

Set these with `env.NAME = value` in `Documents/madeira.cfg`, or in the process
environment.

| Switch | Default | Effect |
| --- | --- | --- |
| `MADEIRA_HWINPUT` | on | `0`: no keyboard/mouse bridging, pointer recognisers, touch filter, drawn cursor, lock or right-stick mouse; the app behaves as before this change (the Info.plist key stays). |
| `MADEIRA_INPUT_FOCUS` | on | `0`: keyboard and mouse reach the program whatever has focus in the app (still not while the app is inactive). |
| `MADEIRA_DIRECT_CURSOR` | on | `0`: no cursor is drawn on the game view, motion stays relative there and the pointer never locks by itself; the driver reports nothing. |
| `MADEIRA_POINTER_ABSOLUTE` | on | `0`: the program's cursor never follows the pointer's position; all motion is relative. |
| `MADEIRA_POINTER_LOCK` | on | `0`: never lock the pointer; no lock button, Ctrl+Alt+P is sent to the program. |
| `MADEIRA_POINTER_AUTOLOCK` | on | `0`: lock only by hand (Ctrl+Alt+P or the lock button). |
| `MADEIRA_NAV_KEYS_E0` | on | `0`: the driver's previous extended-key flags for the navigation keys. |

## Logs

`[hwinput]` lines: the switches at start; keyboard and mouse connects and
disconnects with the device inventory (repeated 10, 30 and 90 s after start,
because stderr becomes the session log only when Wine starts); the mouse path;
keyboard focus changes, and mouse focus changes caused by the app's state;
the program showing or hiding its cursor; pointer lock changes with their
reason; unmapped HID usages; touch classifications; the right-stick mouse
turning on and off. With diagnostics on (the ladybug), also every mouse focus
change, pointer route changes, the first raw deltas, a 10 s delivery-rate line
and a 1 Hz activity line while input is moving or held. The driver logs each
new cursor image (`[winios] cursor set`), as in the desktop session.
The first 100 input edges also show the source, hit-focus decision, pending
clicks and posted button flags/position. The activity line includes whether
the game view is first responder. A delivered Globe press logs
`Globe -> Escape` even with extended logging off; no typed characters are
recorded in these new traces.
`[winios-input-q]` periodically reports coalesced motion and any queue drops;
`dropped-critical` should remain zero.

## Validation

```sh
python3 tests/host/check-hardware-input.py      # needs swiftc, cc and the wine submodule (or WINE_SRC)
python3 tests/host/check-nav-keys.py
python3 tests/host/check-game-pointer.py --require-swift
```

`check-hardware-input.py` compiles the pure part of `HardwareInput.swift` (key
map, physical scan codes, mouse button edges, held-key diffing, motion carry, wheel notches, the
AssistiveTouch classifier, the desktop cursor clamp, the right-stick velocity,
the view-to-screen mapping against `mapTouch`'s arithmetic, focus gating of
held keys, click focus, the pointer route and the automatic lock) and the
driver's fallback extended-key function, takes every mapped HID usage through
the physical or active-layout path, and compares the result with the scan code
and E0 prefix a PC keyboard sends. It compiles `WiniosCursor.c` and exercises it, including from four
threads at once. It also checks the driver's reports, the focus gating on every
path to the program, the app wiring, the switches and this document.

Device checklist:

- typing and held keys (WASD with Shift) in a program, with and without a
  text field of the app focused;
- the cursor on the game view: visible in a program's menus with its own
  image, hidden in mouse-look, under the pointer on iPad;
- mouse-look with the automatic lock on iPad in full screen, and its release
  when the program shows a cursor;
- clicking the app's own buttons with the mouse (the program gets nothing), on
  iPad and on iPhone with AssistiveTouch;
- the app going to the background with a key and a button held (both released);
- the wheel and all five buttons; Ctrl+Alt+P and the lock button;
- the iPhone AssistiveTouch hint and click filter;
- the right-stick mouse on and off;
- each switch at its non-default value.

For the TrainStationTycoon regression, test left/right clicks at 2560x1440,
drag across the surface edges, open Madeira's menu while a key/button is held,
then resume. Test Globe/Escape and background/foreground once with diagnostics
enabled. Host tests verify the production geometry and event policies; device
tests are still needed for iPadOS pointer lock and system-key delivery.
