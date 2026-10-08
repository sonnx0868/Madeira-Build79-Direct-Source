# Gameplay performance validation

The Silksong build570 direct-route report (2026-10-07, 16:49 local) confirms
`MADEIRA_MULTI_GAME=0`, 2560x1440, CPU report 4, nominal thermals and roughly
3.1–3.3 GB footprint. In frames 8000–10944, five 64-frame batches contain a
gap over 500 ms, max 1445.5 ms; Present itself stays below 13.3 ms. These are
frame-production stalls, not a claim that GPU execution has been measured.
The app's background gap is excluded. New x64 compilations and callret resets
increase in the affected region; exact JIT/GC/I/O attribution remains unproven.

Two concrete defects are addressed in this source update:

- Both Metal and DXMT shader-cache paths failed in the report. The native cache
  resolver previously defaulted to the iOS sandbox only for WoW64. All iOS
  callers now use NSCachesDirectory. Existing per-executable keys and shader
  cache version are retained; failures still fail cleanly. `DXMT_IOS_CACHE_DIR=0`
  explicitly restores the old resolver, `DXMT_SHADER_CACHE=0` disables the disk
  shader cache, and `DXMT_CACHE_STATS=0` silences sparse hit/miss evidence.
- Normal sessions still ran a Mach read and a 24-site classifier on every
  emulated store, rescanned the alias table for forensic write counters, and
  read back new VM commits under virtual_mutex. Store timing/classification,
  alias telemetry and TLS forensic polls now require extended diagnostics;
  commit/decommit read-back and decommit census require `MADEIRA_MEMORY_CENSUS=1`.
  Only observation is gated. Actual zeroing, atomic emulation, SMC invalidation,
  Mono bridge capture, execute recovery and safe retirement are not skipped.

No FEX source changes or unsafe SMC/TSO opt-outs are included. No render
resolution or controller preference is changed. This is not a verified promise
of 60 FPS without spikes: first-use managed/JIT compilation and asset loading
can still stall.

Build **Madeira v0.1.5 source bootstrap + IPA**. Old cached native bundles are
rejected by archive markers `gameplay-observers=v1` and `sandbox-v1 reader ready`,
as well as the real ARM64 PE check for the supervisor. The cache source change
is reproduced by `patches/dxmt-gameplay-performance.patch` in clean builds.

`check-gameplay-performance.py` compiles the actual cache policy and alias
telemetry. On Codemagic it also compiles the real Objective-C cache classes and
checks shader bytes survive writer close and reader reopen. Windows cannot
verify Foundation/iOS/Metal. No synthetic test measures the game's performance.

Device test: keep the same resolution, FPS cap, scene/route and thermal state.
Run once to populate the repaired shader cache, then fully reopen and repeat.
Send both reports. Expect a ready cache and warm hits, unchanged display/input,
and fewer normal-play forensic messages. Compare foreground frame gaps over
50/100/500 ms, not only average FPS. Validate the reusable runtime separately
with native-supervisor startup, normal Quit → home → ready → another game.

## Startup and scene transitions (2026-10-08)

Shader cache writes now run on a serial utility queue. Each writer retains at
most 32 MB, including keys and per-entry overhead; excess entries are skipped
without blocking compilation. Guest keys are copied before the thunk returns,
and native dispatch data is retained until the write completes. Queue saturation
or closing the app before writes finish can leave entries to compile next time.
Cache initialization uses a nonblocking file lock; another writer cannot hold
up the game indefinitely. The reader does not wait for SQLite busy locks.

The compiler fingerprint is generated from source, build flags, SDK/toolchain
identity, linked LLVM archives and converter headers. It replaces build-time
stamps in D3D12 cache keys and namespaces DXMT's iOS shader tables. Identical
rebuilds preserve entries; compiler changes invalidate them. The first build
using this identity needs to populate new entries once.

On iOS, DXMT shader/pipeline workers default to one through four, bounded by
reported hardware concurrency, at normal priority. `env.DXMT_COMPILER_THREADS`
can explicitly select 1–8 workers; malformed values use the default. This
reduces oversubscription during load bursts; the best count still requires a
device comparison. D3D11's pipeline cache stops/joins its compiler workers
before destroying shader and pipeline task objects. Moved shader-cache guards
also transfer lock ownership exactly once.

The launch UI heartbeat now requires `MADEIRA_RUNTIME_PROFILERS=1`. Silksong and
other detected Unity/Mono games select a direct launch even if Reusable runtime
is enabled, while the managed-child exception route awaits device validation.

Bootstrap rebuilds the ARM64EC DXMT DLLs as well as the native archive. Preflight
rejects bundles missing `async-writer-v1`, `shader-compiler-content-v1` or
`compiler-workers=v1`; an old native bundle cannot deliver these changes.
`check-startup-performance.py` tests actual scheduler shutdown, worker limits,
cache memory reservation and content fingerprint generation. Codemagic also
requires the real Objective-C asynchronous write/reopen/borrowed-key test.
These host checks do not establish a Silksong frame rate or eliminate first-use
FEX/Mono compilation, asset loading, GC or Metal pipeline compilation.

## Retrieved device reports (2026-10-08)

The authenticated log server is accessible through the owner's in-app browser
session. Three full, untruncated Silksong reports were downloaded and inspected:

| Report | Build | What it establishes |
| --- | --- | --- |
| Current direct-launch report | 572 | Direct launch, working shader cache, substantial startup/initial scene stalls |
| Previous direct-launch report | 570 | Previous direct-route comparison with cache-open failures |
| Previous reusable-runtime report | 570 | Reusable route fails around Mono Finalizer exception unwinding and leaves live peers |

The build572 report is 4,371,851 bytes, from `iPad14,6`, iPadOS 27.0.1,
15,709 MB physical RAM. Launch configuration is 2560x1440, reported CPU count 4,
Reusable off, swap 2048 MB, video-memory report 6144 MB, `inproc-sync=0` and no
explicit fastsync setting. Under the current configuration rules that selects
Wine standard sync. A controlled Fastsync comparison is still needed. The
Unity profile's new launch-specific choice is visible in game details and
`[startup-sync]`; explicit env/per-game opt-outs and Madsync take priority.

Observed launch is 00:03:43.724 local, and game-visible-present is 00:04:36.733:
53.009 seconds. In early 64-frame batches, the maximum inter-Present intervals
include 12,839.7 ms at frame 64, 30,333.4 ms at frame 832, 6,613.1 ms at frame
1664 and 10,890.4 ms at frame 1728. Corresponding maximum time inside Present
is 19.9/14.3/15.1/15.2 ms. These establish frame-production gaps outside Present,
not the GPU's execution time and not an isolated attribution to shader compile.

Both native cache-directory resolution and reader initialization succeed in
this run. No CacheReader/CacheWriter failure is logged. Sparse cache counters
show 0 hits/1 miss, then 0 hits/64 misses; this is a cold-cache run, not evidence
that a second launch cannot retrieve persisted entries. After the initial load,
many 10-second samples stay around 59.9–60.0 FPS with nominal thermals.

FEX reports real compilations increasing during initial scene stalls and
252,249 cumulatively near the end; the late overall translation-cache hit rate
is 99%. Call/return-predictor reset counters reach 44,032, predominantly from
the core invalidation path. The reported cumulative byte total counts ranges
reset, not bytes proven written or resident. A reset cannot safely be skipped
merely because it shares a code-buffer generation: Mono can invalidate code
within that generation. Attribution to translation, managed GC, synchronization,
asset I/O and shader/pipeline compilation still needs a controlled comparison.

An 8,489,740.6 ms interval occurs across app inactivity/backgrounding and is
excluded from gameplay hitch conclusions. Late Metal errors explicitly report
GPU submission while the app is in the background; they do not establish the
cause of the initial foreground stalls. The reusable report is from build570;
there is no reusable-enabled build572 report in these three downloads.

The uploaded build572 IPA predates the current uncommitted compiler-worker,
asynchronous-writer and content-fingerprint changes. Its measurements cannot
validate those changes. Rebuild the full source bootstrap and compare the same
foreground route on cold and warm launches before claiming a speed gain.

## Changes targeting the initial load

The build572 run creates large file-backed swap extents during the initial
scene load while its footprint stays around 2.8–3.4 GB on the 16 GB device.
The Unity smoothness profile now enables pressure-only backing for classic
swap coverage: future fresh allocations stay anonymous with ample headroom.
Launch publishes `os_proc_available_memory()` before Wine allocations, and the
native footprint monitor refreshes an atomic snapshot outside virtual_mutex.
Allocation decisions issue no memory-query syscall and never copy/remap live
data. Low/unknown headroom retains backing; the 2 GB margin also accounts for
the incoming allocation. Explicit wider coverage and a pressure-policy off
setting retain the previous behavior. Preflight requires `swap-pressure-v1`.
Fresh eligible anonymous commits debit the snapshot between monitor samples,
so allocation bursts cannot repeatedly spend the same headroom. Ineligible
FEX/predictor allocations do not debit that estimate.

The profile selects Fastsync auto for the logged Wine standard configuration.
This avoids defaulting this optimized game launch to server-based event waits.
Madsync, explicit global Fastsync settings, per-game off, profile off and the
`MADEIRA_UNITY_STARTUP_SYNC=0` escape hatch take priority. Semaphore acceleration
stays separately opt-in. `[startup-sync]` records the actual launch request.

`check-silksong-startup.py` compiles the actual C pressure policy and, on CI,
the actual Swift sync decision. It tests ample/low/unknown headroom, integer
boundaries and the opt-outs; it does not measure paging, FPS or Mono/FEX compile
time. These changes reduce identified avoidable startup work; the 53-second
delay has not yet been reproduced on a device running the new code.
