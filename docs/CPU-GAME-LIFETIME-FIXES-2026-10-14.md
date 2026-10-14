# CPU, game startup/input and log lifetime fixes

The tester confirmed Madeira 666 versus BCD `v0.1.11-pr1-test`, both at
1408x648 for Primal Planet. The matching BCD IPA was already downloaded and
its asset hash verified. Device logs from the tester are pending.

## Prepared source changes

- FEX: import the exact upstream Windows/iOS CPUFeatures implementation at
  `be778d7ba5b98bee0405fd1d6a50d305a0807df0`, including
  `aaf73a9d7ca25c694e7ddce02d2c246c47cee5dd`. Publish LRCPC2 in the app's
  sysctl probe. ARM64EC uses immediate-offset TSO instructions only after a
  positive answer, defaults AFP off, and honors missing CPU features. The
  patch also imports the upstream non-TLS IR capture marker: compiling code
  must not clear bytes in the game's TLS. These are verbatim upstream source
  deltas, applied by the build integration; no local FEX algorithms were
  authored and the user's dirty submodule checkout was left intact.
- Party Animals: generic guest reserves must exclude the published FEX host
  arena. Log `5c42d251` shows large reserves consuming ~7.7GB of that band,
  followed by failed thread-state/FEX container allocations and a host BRK.
  Generic spill allocation now uses only intervals outside the arena, preserves
  retry inputs on failure and retains existing dead-thread reclamation.
  FEX's explicitly bounded requests keep their dedicated allocation route.
- Touch-only Primal/Godot: reserve a configured touch controller before game
  enumeration, even if its overlay is shown later. Once enabled, its slot
  remains connected for the session. Import Wine's builtin-XInput selection
  so game-local Microsoft XInput DLLs do not bypass the host snapshot;
  `MADEIRA_NATIVE_XINPUT=1` retains an explicit native choice.
- Fading Afternoon: the available 649 log `9da5468c` is a NULL Steam UserStats
  interface dereference in `Steamworks_x64.dll+0x19a0d`, inside exported
  `steam_get_achievement`, after successful rendering. It is distinct from
  the earlier reusable swapchain failure. The supplied DLL's instructions
  match the log, and its Steam settings provide App ID 1687000. Imported game
  identity is now read from bounded `steam_appid.txt` metadata rather than
  using the old Thumper fallback. The upstream TLS fix removes a separate
  runtime corruption hazard. These changes do not prove the Steam interface
  failure is fully resolved; the tester's matching 666 log/device run is needed.
- Game logs/lifetime: register the selected game PID before the first window,
  match its executable (or install directory if metadata is absent), ignore
  unrelated launcher/helper exits and observe zero/error exits separately
  from a live Steam/server. The library closes the session on that PID's exit.
  Capture an exact raw-log end offset before async IO, retain the final exit
  report, truncate any queued post-session bytes and persist the end boundary
  in recovery journals. Files keep the existing game-name/date/time naming.

## Checks and limits

Windows host fixtures passed actual upstream CPU feature selection for both
module configurations, unknown/negative probes, opt-outs, the global IR marker,
builtin-XInput selection and the production spill allocator with 24
Party-sized reservations. The complete ordered Wine patch pipeline applies and
repeats with the existing loader/compatibility patches.

The Mac verification workflow exercises Objective-C process matching, Swift
touch arbitration, raw log boundaries/recovery and folder identity, and
compiles the production Wine bridge against the iOS SDK. It creates a fixture
simulator app, not a release IPA. Final linked translators/Wine/Metal archives
and real-device gameplay have not been verified in this source-only stage.
Native compatibility checks reject caches lacking the CPU, builtin controller,
guest-arena isolation and process-lifetime changes. No release/update is
authorized by this verification step; the user requested a changes list before IPA building.
