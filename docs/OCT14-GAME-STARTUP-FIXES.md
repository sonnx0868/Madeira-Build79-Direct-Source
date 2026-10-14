# October 14 startup and input fixes

Evidence: Primal Planet log b6be0ad9, build 647: first visible frame after 25.012 s,
1408x648 Zink/MoltenVK on M2, normal thermal state, 19.3–31.7 FPS during the
measured gameplay interval. More than 16,000 fixed shared-page timer reads were
serviced by Mach exceptions. No measured FPS gain from the new build yet.

- Native kernelbase GetTickCount/GetTickCount64 use the existing native QPC
  boot clock. The Unix QPC and wineserver TickCount both use mach_continuous_time.
  32-bit wrap, milliseconds and sleep progression are retained. The x86 path
  remains unchanged. Both native PE architectures must be rebuilt.
- Automatic Godot controller mode uses XInput once; the explicit combined
  XInput/DirectInput choice remains available. Touch/physical merged snapshots
  feed the same device. This addresses the duplicate device hypothesis; device
  menu and touch operation still need confirmation.
- Party Animals log a54a899f: native MSVCP140 with game-local x64 VCRUNTIME140
  jumps to MSVCP140's MZ header. Prefer the bundled CRT family for this launch.
  The same game later runs out of its 560 MB pool loading phonon.dll.
- Import only the two-region split-pool paths from BCD d0d1349c4dcf9c2f45df8efef253197b4368736d,
  credited to its authors. No FEX change, pure-x64 no-copy, Social Club launcher
  or poisoned-range salvage. Check allocation/warmer bounds across the native
  hole, conservative rollback and same-distance writable aliases. Reject a
  failed hole protection or overlapping alias. Secondary allocation rounds
  down to the remaining physical pool budget. Automatic opt-in is limited to
  Party Animals on devices with at least 8 GB RAM; pool-split cfg still wins.
- Preserve authorized live game sessions when moving to the home screen;
  app-switcher snapshot remains covered. Leaving the library with no live game
  relocks its private shelf, and Quit still returns home and renews JIT.
- Regular-width sidebar owns its alignment, fixed icon column and wider layout.
  Search lives in the content column; compact layout retains native nav search.

Host tests verify allocator boundaries and timer conversion. Mac CI must prove
native compilation, policy behavior and the app build. No game/device outcome
is implied by compilation; no 60 FPS guarantee is made.

Steam/Dock profiles are installation folders, so runtime detection now handles
both directory and executable inputs. GameMaker/Godot/Unity isolation and the
Godot input policy apply to Steam entries as well as imported games.

## Approved illustration and shelf revision (build intentionally deferred)

User requests: match the approved preview, include the scenic sidebar/passcode
background, keep the original native search appearance, and do not relock an
already-authorized private game while switching to StikDebug for JIT.

Asset: app/Madeira/Assets.xcassets/LibraryLandscape.imageset/LibraryLandscape.png.
Generated with built-in ImageGen. Prompt: cinematic dusk mountain lake, evergreen
forest silhouettes, tiny warm cottage, deep charcoal/midnight blue/muted teal,
empty dark upper sky, no text, logo, UI or characters. One static 1536x1024
illustration is reused by sidebar/passcode without animation or network fetch.

The regular shelf uses a unified installed-game gallery and a separate Steam
management/download area. Hidden entries are filtered before search/counts.
Continue-playing uses three artwork cards with independent working Play actions.
Header offers native search, add, options and grid/list. Sidebar has aligned icon
columns, cyan selection rail and illustrated lower half. Compact layout keeps
native navigation search. Both placements use the original ReportingSearchBar,
UISearchController and delegate (including keyboard dismissal and squeeze).

Background policy keeps existing access only while a game is live or while an
already-unlocked game is waiting for JIT; a pending JIT request never grants
access. Otherwise the shelf locks. The switcher remains covered. Quit locks it.

User explicitly requested deferring builds on Oct 14. Bitrise #35/source650 was
aborted after native clock/pool compilation; no finished IPA/native package or
update was published. Latest UI needs future Xcode/device validation. Do not
launch another build until the user finishes their additional feature requests.

Local verification of the final UI revision: tree-sitter Swift reports no new
syntax errors in Library, LibraryAccess or SteamGames; existing grammar gaps
in baseline files are unchanged. The C frontend exit/close regression passes.
The PNG asset is 1536x1024, 1,871,718 bytes, with a valid asset-catalog entry.
These checks do not type-check UIKit/SwiftUI or render the real iPad screen.
