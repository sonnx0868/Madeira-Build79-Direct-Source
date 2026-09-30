# Independent shared-clock audit

Reviewer: `async_native_review`, September 9, 2026. Read-only source/binary/log investigation. No app setting, executable, package or device was changed.

## Finding

The zero elapsed values are consistent with a deliberately disabled Windows shared tick counter, not ordinary millisecond precision. This exact iOS engine publishes the KUSER_SHARED_DATA clock fields only when its **Darwin process environment** contains `MADEIRA_USD_TIME=1` before wineserver startup. The default is off and cached. Native monotonic/server timeout state and the performance-counter API use independent clock paths; there is no basis for calling all clocks broken.

The existing app already supports this setting through **`Documents/madeira-usd-time.txt`**. Its startup code reads the file, trims whitespace, calls native `setenv`, and logs activation before starting wineserver. A separate controlled clock test can therefore use the existing feature without an IPA rebuild. A Windows child-launch environment flag is not the correct demonstrated route. Full app termination/relaunch is required when enabling or disabling it because the host gate is cached and the mapping is chosen at startup.

This is actionable as a separate compatibility test; it should not interrupt the already-started BeamNG experiment or be declared the proven CEF timeout cause.

## Physical evidence

Fresh successful diagnostic logs are in `work/beamng/apc-signal-fix/device-after`. The suite reports zero elapsed milliseconds for all three children; the outside-wait test also reports zero worker and parent-write elapsed differences. A zero result for one short parent write could be clock precision, but the suite-wide pattern cannot be explained that way.

Independent native phase timestamps in `work/beamng/live-logs/2c3c1881da539ee1.log` put AsyncCancel spawn at `t+11.664s`, CEF-IPC parent spawn at `t+11.748s`, its child at `t+11.813s`, and the subsequent APC-delivery spawn at `t+11.944s`. Thus the CEF-IPC portion spans roughly 196 ms between successive suite child launches while its GetTickCount64 delta is zero. The intended shared-page update interval is at most the ordinary 16-ms idle poll timeout, with publication after event-loop activity, not a 196-ms clock quantum.

The complete startup header includes neither the app's `Shared-data clock: MADEIRA_USD_TIME=...` marker nor the opt-in `[usd-map]` creation diagnostics. Together with the frozen tick readings and default-off native gate, this strongly supports the feature being off in that run. The log does not itself reveal the cached integer or independently sample the live mapping contents; an explicit clock test remains the direct confirmation.

The successful pipe tests remain valid despite zero elapsed text. Their publication/data assertions and explicit Windows event/process waits succeeded. However, helper polling loops whose deadlines use GetTickCount64 are **not guaranteed to time out** while it is frozen. A success is still a success; an intended tick-based failure deadline is not a reliable watchdog in this state. Native explicit wait timeouts have a separate server monotonic path.

## Pinned evidence

Madeira source revision `44b4b67e116b5bc813f7a35c9ed3b7d8ad3b5416`:

| File | SHA-256 |
| --- | --- |
| `build/wineserver/fd_ios.c` | `4e70cd6559e87254ed1edcd8671e9a4a77c79838347d2782c1e95be2ab06e6e1` |
| `build/wineserver/mapping_ios.c` | `2646e99300a74328be85a7c3f79833c9b6a7326f6b611f0faf36b88494d5fc88` |
| `app/Madeira/ContentView.swift` | `8062f9def3f5c9102590191ad60aee599e1b15d5486c3720ebf1ee0ae194ed0f` |
| `build/ntdll-unix/signal_arm64_ios.c` | `9543df6cb3c827dd9e5dc67d23917b7d4afee2979ca029c1159a50cc7a275389` |

Exact native candidate inspected: `work/beamng/apc-signal-fix/Madeira`, SHA-256 `1ae878fbc6ed1f2ffcc9febda14ad55bbffdb5a6efbe8c35cd10db6822c67a7b`. Its clock-related code is unchanged by the 20-byte APC signaling correction.

PE copies inspected under the candidate's signing-stage `Payload/Madeira.app`:

| PE | SHA-256 |
| --- | --- |
| `arm64ec-windows/kernelbase.dll` | `ee8bf461cf881b0b04a22696fa8f6397f1eb877dfb52944c2cbec4d25987f718` |
| `aarch64-windows/kernelbase.dll` | `cfe61575b101e7d59303d34aa57bd169a178aef433d9d694eaa340400693971d` |
| `arm64ec-windows/kernel32.dll` | `91ff5ec8cf5b72ef081f93ee7f16069be15101c9f39b6c3d65b5445e10f5c69b` |
| `arm64ec-windows/winmm.dll` | `7df6bb3f17f930e644bb83db38caf859fc8ea0b50564fe00ca5635edca13a7e5` |
| `arm64ec-windows/ntdll.dll` | `e43ae5a9bc90a02c3c01e5a451e9ab7f5570b2735b93215284868a279254c27f` |

These are exact packaged candidates plus the root-provided fresh runtime. This audit did not independently pull the installed app files from the iPad.

## Writers, readers and the host setting

`fd_ios.c:408–416` caches `getenv("MADEIRA_USD_TIME")` as true only when the first byte is `'1'`. Native `_ios_usd_time_enabled` is `[0x10006268c,0x1000626d0)`, with cache at `0x100c1acd8`.

`_set_current_time`, `[0x1000627d0,0x100062910)`, always updates native current time from `gettimeofday` and monotonic time from `_monotonic_counter`. The global native values are at `0x10107d600` and `0x10107d608`. Only its shared-page publication is behind the gate and non-null server alias at `0x101073cf0`. Native `0x100062884` skips the publication block when disabled. When enabled it stores SystemTime, InterruptTime, TickCount and the deprecated tick field, using the expected ordered high2/low/high1 sequence. There are no logging or timezone-library calls inside the periodic publication block.

The ARM64EC kernelbase GetTickCount64 export at RVA `0x82260` is an x64 entry thunk into its ARM64 body at RVA `0x682c0`. That body directly reads the low/high1/high2 fields at canonical KUSER_SHARED_DATA offsets `0x320/0x324/0x328`, retrying inconsistent high words. The aarch64 PE body at RVA `0x76fa4` does the same. Neither API computes a fresh native clock. GetTickCount and QueryInterruptTime likewise read the shared fields.

iOS reserves the canonical low address `0x7ffe0000`, so the native Wine USD is allocated elsewhere and remapped to the shared section. Existing iOS Mach/signal handling redirects canonical USD reads to that real allocation; native `NtGetTickCount` at `0x1001194e4` directly uses the real pointer at `0x100c3cdf8`. Native `virtual_map_user_shared_data` at `0x10013624c` opens the same named section and maps its FD shared/read-only at the allocated guest address. The server's writable alias and the client/guest view are separate virtual addresses backed by the same section. Freezing server publication therefore freezes a valid guest view; an invalid guest pointer is not needed to explain the symptom.

The existing app setting is independently present in the exact native executable: the filename is referenced at `0x10002197c`, UTF-8 file reading follows, and native `setenv` is called at `0x100021a9c`. The activation log string is referenced at `0x100021b24`. These instructions are inside `runWineFullSequence`'s startup closure and precede `startWineserver`. The matching pinned Swift source is lines 1936–1949.

Operational details for the root's later controlled test: the file belongs in the app's **Documents root**, adjacent to the `wine` directory, not under `wine/drive_c`. Contents `1\n` enable it. Contents `0\n`, or removing it before a fresh app launch, restore the default. Removing the file in an already-running app does not unset an existing host environment value or clear the cached gate. A child-process `SetEnvironmentVariable` or CreateProcess environment block is not shown to affect the already-running native server and must not be used as a substitute.

## Why this remains opt-in

The comments describe concrete earlier investigations, not a blanket iOS prohibition:

* The oldest skipped-write explanation blamed library validation. The later author rejects that explanation for a shared data-only mapping; executable-library validation does not establish this page's failure.
* A writable server alias once landed at `0x7038000000`, the guest allocation window's lower bound, then a later store wedged after an initial successful write. The source treats reclaimed/reprotected guest VA as a likely explanation. This is a historical observation, not a proven current-device failure.
* Earlier clock trials kept logging or alias-reporting in the timed server loop. Those paths did file I/O and could perturb or block the loop under test. The present periodic path removes them; creation-time reporting remains.
* The complete opt-in also changes alias placement. `mapping_ios.c:1533–1594` keeps the old PROT_WRITE shared mapping exactly when disabled. Enabled, it tries non-fixed hints 0, 8 GiB, 12 GiB and 16 GiB, rejecting aliases inside `[0x7038000000,0x8000000000)`. It requests RW, applies RW protection, reports the creation mapping, then publishes the alias. If no alias is found outside that band, it logs the failure and leaves publication unavailable. The present native `_create_user_data_mapping` at `[0x10006f3dc,0x10006f658)` matches these branches.
* TimeZoneBias is deliberately left at initialization because timezone conversion uses shared libc locks/static storage in this all-in-one process. Enabling the tick writer is not a timezone-correction test.

The enabled alias path still performs a creation-time write/readback diagnostic and periodic stores. Its old failure rationale means the correct next action is an observed, reversible setting A/B, not silently declaring it safe everywhere or changing the default without physical validation. The alias exclusion band is hardcoded; the current 896-MB pool matches the comments, but this audit does not validate every alternate allocator layout.

## Separate clock paths and CEF relevance

Native `NtQueryPerformanceCounter`, `[0x10011880c,0x100118890)`, calls `mach_continuous_time` and applies `mach_timebase_info` to return 100-ns units, with frequency 10,000,000. Native `_monotonic_counter` at `0x10008b924` uses the same independent time source. Native `NtQuerySystemTime`, `[0x100118748,0x10011880c)`, uses `clock_gettime(CLOCK_REALTIME)` with a gettimeofday fallback. These exact functions do not read the frozen USD fields. The source's broad comment saying QPC uses clock_gettime is less precise than this actual Darwin binary.

The current ARM64EC `RtlQueryPerformanceCounter` body at ntdll RVA `0x6fbd0` calls NtQueryPerformanceCounter; frequency at RVA `0x6fbec` is 10,000,000. `winmm.timeGetTime` imports kernel32's implementation; the actual kernel32 body at RVA `0x25740` calls QueryPerformanceCounter and QueryPerformanceFrequency and converts to milliseconds. Therefore **timeGetTime is also not this build's frozen GetTickCount implementation**.

Pinned Chromium `72.0.3626.121/base/time/time_win.cc` SHA-256 `26fb36c951062e6f75e4f4e9134e3d54f4d456493d6c34587890284c994f6ce8` selects QPC when its CPU/frequency checks permit, otherwise its rollover-protected multimedia clock. That fallback uses timeGetTime. Both routes ultimately have an independent QPC source in this Wine build. This lowers confidence that the already-observed CEF timeout is specifically a frozen TimeTicks clock; it does not rule out another CEF, game or middleware component calling GetTickCount64 or reading shared fields directly. No such decisive callsite has been established by this audit.

Existing package resource `arm64ec-windows/clocktest-x64.exe` is present, SHA-256 `1a3e20a534b90aab9022340d001a15ac66aa0804f07647f62d4ef2db49498707`. The app's existing **x64 clock test** action selects it; source comments describe a one-second comparison of tick, wall, unbiased interrupt and QPC clocks. Its imports confirm GetTickCount64, GetSystemTimeAsFileTime, QueryUnbiasedInterruptTime, QueryPerformanceCounter/Frequency and Sleep. This audit verified presence/imports, not this diagnostic's complete source or pass criteria. It can serve as the next existing observation route after the root reviews its results, or the suite's elapsed values can give a narrower before/after check. No new test executable is necessary merely to flip the setting.

## Limits and next useful evidence

No device setting or test was run in this audit. A useful later validation records the app activation marker, `[usd-map]` creation outcome, advancing tick and interrupt values over a controlled sleep, independent QPC/wall-clock deltas, and app responsiveness during normal Wine startup. Preserve the known-good pipe diagnostic results while comparing. A successful tick test establishes that shared-clock publication works for that run; it is not a claim that BeamNG is in-game or that clock publication explains its remaining startup failure.

Primary links: [Madeira clock publication](https://github.com/margooey/Madeira-actions/blob/44b4b67e116b5bc813f7a35c9ed3b7d8ad3b5416/build/wineserver/fd_ios.c), [gated alias allocation](https://github.com/margooey/Madeira-actions/blob/44b4b67e116b5bc813f7a35c9ed3b7d8ad3b5416/build/wineserver/mapping_ios.c), [existing native app setting](https://github.com/margooey/Madeira-actions/blob/44b4b67e116b5bc813f7a35c9ed3b7d8ad3b5416/app/Madeira/ContentView.swift), [Chromium 72 clock selection](https://github.com/chromium/chromium/blob/72.0.3626.121/base/time/time_win.cc).
