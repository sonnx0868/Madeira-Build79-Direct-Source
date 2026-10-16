# SDL host controller transport and installed-game runtime checks

## Existing Celeste 721 evidence

The supplied tester file `Celeste_2026-10-16_13-57-39-541.txt` runs Epic
Celeste, i386/WoW64, on iPhone18,2. Both a paired controller and the touch
slot are reserved before Wine starts. SDL2, DirectInput8 and XInput1_4 load.
The first seven host queries include a connected, neutral slot 0. Later
capabilities/active-button reads are not recorded. The initial neutral-query
logger stops after eight calls; its silence alone cannot prove polling stops.
Host publish logging also had a process-wide first-64 budget that could be
consumed before this game's log began; it is reset at session reservation now.

The shipped i386 XInput DLL has the host capabilities query, not only GetState.
Its actual disassembly dispatches op 0x100 to the host. Wine's WoW64 thunk
preserves the scalar selector and translates the output pointer. No evidence
here supports replacing the controller DLL or downloading another game DLL.

The log has failed HID-device opens and a hidden SDL helper window. Those
do not prove the precise point of input loss. SMC faults are handled and occur
during initial translation, so they are not a confirmed fatal game exception.
There is no post-startup frame-rate record establishing whether the game loop
continues. These limits must not be restated as proof of a frozen input pump,
bad iOS controller hardware, a missing runtime, or lost SDL focus.

## Shared correction

For detected SDL2 games using Madeira's builtin host XInput transport, choose
the supported XInput backend and avoid Windows HIDAPI, RawInput, WGI and
duplicate DirectInput enumeration. Detect SDL2 beside the launch program or
one directory below, using the existing bounded runtime probe. Set these as
defaults only; explicit user SDL hints and native-XInput choices are preserved.
Remove defaults owned by this policy before the next game. Other engines keep
their previous route. No game EXE, DLL or saved controller map is changed.

Permit joystick events while the guest Windows window lacks focus: this is
focus inside Wine, not permission to keep playing in the iOS background. The
existing GameController/touch publisher still clears holds and publishes a
neutral state when Madeira resigns active. Disable SDL's separate Windows
joystick detection thread; normal SDL pumping polls the in-process snapshot.
Record the resolved Boolean hints once after log redirection so the per-game
file can establish which route actually reached the guest.

This addresses two verified SDL failure modes of a host-only controller. It is
a compatibility correction/candidate for the reported Celeste failure; the old
log does not establish which of them occurred in that exact game session.

## Checks and limits

`check-sdl-xinput-integration.py` runs the official SDL2 2.32.10 x86 DLL
on Windows with the production `WiniosGamepad.c` snapshot and actual
`ios_gamepad_query`. A test-only XInput API forwards SDL's calls to this
transport; it is never included in the app. A hidden window makes guest
focus meaningful. The actual policy defaults are parsed from production Swift.

- With RawInput enabled, SDL suppresses the host-only XInput controller.
- With guest background events disabled, SDL enumerates the controller but
  filters its button state while the hidden window lacks input focus.
- With the profile, SDL sees one early connected controller and correctly
  reads buttons, d-pad, analog input and releases from the shared snapshot.

The downloaded official SDL runtime is pinned to SHA-256
`68c78590d1997122c30c992ec79857d32e5e976a0f17c0a69a2e29eab0101d70`.
It is a reference DLL, not claimed to be the tester's exact SDL binary.
This test does not exercise iOS GameController, UIKit gestures, Mach/FEX,
Mono, or the full Celeste loop. Swift host checks additionally exercise actual
policy precedence and cleanup; existing physical/touch merge and lifecycle
checks remain required in CI. Celeste device gameplay must still be validated.

## Check & repair screenshot

The scanner used the current prefix's system32/syswow64 before the app's
architecture-specific farms. A stale DLL staged for a prior session could
therefore be reported as a wrong-architecture game DLL, even though launch
restages the correct bundled runtime. Prefer a verified matching farm when
checking the Windows directory. Keep game-local shadowing errors and preserve
the original files. If the app really lacks a matching farm, the dependency
remains unresolved. A wrong Windows DLL is not described as a bad game file.

Watch the selected farm files and directories, plus the bundle directory, for
cache invalidation. Advance the receipt schema so old false warnings are not
reused. The screenshot lacks resolved paths, so it cannot alone establish
which stale prefix entry triggered that particular report. Regression cases
cover stale i386 Windows DLLs before a 64-bit launch, ARM64EC/native farms,
missing farm fallback, preserved local architecture conflicts and cache repair.

## Primary SDL references

- https://wiki.libsdl.org/SDL2/SDL_HINT_JOYSTICK_HIDAPI
- https://wiki.libsdl.org/SDL2/SDL_HINT_JOYSTICK_RAWINPUT
- https://wiki.libsdl.org/SDL2/SDL_HINT_JOYSTICK_WGI
- https://wiki.libsdl.org/SDL2/SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS
- https://github.com/libsdl-org/SDL/blob/SDL2/src/joystick/windows/SDL_xinputjoystick.c
- https://github.com/libsdl-org/SDL/releases/tag/release-2.32.10
