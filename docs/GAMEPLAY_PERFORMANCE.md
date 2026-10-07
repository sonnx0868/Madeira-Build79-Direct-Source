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

Build **Madeira v0.1.4 source bootstrap + IPA**. Old cached native bundles are
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
