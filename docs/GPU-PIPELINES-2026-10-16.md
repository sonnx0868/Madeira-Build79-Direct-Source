# GPU pipeline work after build 721

## Device evidence

The REPLACED report `590e17f7` requests/applies 60 FPS at 1920x1080. The
main thread is no longer sampled at priority zero. It falls from about 51 FPS
while running on P cores to about 31 FPS on E cores after the public thermal
state reaches serious. GPU command-buffer intervals are nearly continuous
before the transition. That is interval coverage, not hardware GPU utilization.
Pipeline creation is sparse/short in this report; compilation does not explain
all the sustained slowdown.

Party Animals reports `b23766da` and `9a9a12cf` describe the same session,
which starts already at serious thermal pressure. The render surface is
800x600, despite the 1920x1080 monitor. It enters gameplay but eventually
runs at about 11-14 FPS. Pipeline creation includes a 780.60 ms outlier.
Some binary-archive saves fail and some succeed. No per-core measurements
in these Party reports establish which cores ran the game.

## Implemented changes

- Memoize at most 128 fully described ordinary Metal pipelines per device
  and game cache scope. Reuse complete successful PSOs and share concurrent
  identical builds. Reflection, explicit archives, unsupported/private
  descriptors and caller opt-outs retain their original paths. Native errors
  and exceptions wake waiters without permanently caching failure. The native
  compiler and archive operations execute outside the bookkeeping condition.
- Keep the descriptor's functions alive with each cached entry. Tracked
  functions use their library/content/constants identity; untracked functions
  use object identity only for this process, never for disk warmup recipes.
- Give each writer an independent archive shard and unique temporary path.
  Preserve reading old canonical archives. Atomically replace complete files,
  refresh immutable lookup snapshots after a successful save, read at most
  four archives/64 MB, and retain at most eight owned shards/64 MB per identity.
  Symbolic links and unrelated names are excluded. File I/O stays on the
  background writer. A cold miss still creates the requested pipeline.
- Auto enables the existing bounded, archive-hit-only warmup for detected
  Unity titles. Explicit game/global choices win. It neither compiles missing
  shaders in replay nor predicts shaders absent from previous sessions.
- Enable the existing dead color-store optimization on iOS ARM64EC under a
  stricter shared predicate: exact same base view, full matching nonzero
  dimensions/array coverage, no resolve, and a plain Store action. Preserve
  read/write tracking, dependencies, depth/stencil behavior and the explicit
  `DXMT_CLEAR_DISCARD_STORE=0` opt-out.

## Build and verification

The GPU refresh rebuilds only `pipeline_cache_ios.o`,
`pipeline_warmup_ios.o` and ARM64EC `d3d11.dll`. The archive receipt compares
every object occurrence, including duplicate LLVM member names. It preserves
the earlier producer-pacing object's provenance while recording the changed
container. Query provenance is refreshed and the D3D11 exports must match.
Wine/FEX/scheduler/controller binaries are preserved from the verified 721
native bundle. Cached bundles without these compiled changes are rejected.

Host checks cover patch ordering/idempotence, full-clear guards, warmup
identity/replay/fallback, and production Objective-C concurrency, retry,
reentry, live archive lookup and shard bounds. Native Objective-C checks
require macOS; a Windows source-contract check does not count as their pass.

No iPad FPS/temperature measurement of these changes has been made yet.
Reducing repeated compilation and avoidable memory stores may help, but does
not establish that either game can sustain 60 FPS or avoid thermal throttling.

## Tester logs supplied during this work

Two actual files from the supplied public transfer were downloaded:
`Celeste_2026-10-16_13-57-39-541.txt` and
`Fading Afternoon_2026-10-16_13-53-43-468.txt` (build 721).

Celeste reports slot 0 connected with neutral values during its initial
XInput probes. The transport deliberately prints only the first eight neutral
queries, so their absence later does not prove polling stopped. There are no
host publish/lifecycle records in this per-game file. It also contains
repeated access faults in anonymous generated code, not a confirmed fatal
game exit. Input-only failure versus a stalled game loop is unresolved.

Fading Afternoon exits with C000001D. The shipped 721 D3D11 RVA 0x2fcf0 is
`MTLD3D11SwapChain<false>::Present` and begins with ARM64 bytes
`e3 03 1f aa d7`. The failing process contains `e9 83 24 2f be` instead:
an x64 relative detour to 0x71bad32178, executed as ARM64 through an EC
entry thunk. This confirms incompatible runtime code modification. The
writer of that detour is not identified by this log. The exact tester DLLs
are needed before attributing it to an overlay or changing its behavior.
No game DLL or DRM path is modified by the GPU work above.
