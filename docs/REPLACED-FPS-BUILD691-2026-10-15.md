# REPLACED build 691: measured frequency and residency decline

## Report

- Diagnostics ID: `32c971ab-4ff7-4368-9244-cd60b5f46630`, uploaded
  2026-10-15 11:35 +07, current session, not truncated.
- File: 3,581,656 bytes; SHA256
  `7499e3cf1e70564dd4998d0882c5e4d04f34a71ab1763becae4c2e883c52b721`.
- App 691 / `43b9aa9f3ec83945497d6837e551ab4fd1160477`, iPad14,6,
  iPadOS 27.0.1, 1408x648, RAW mode 2, Present SyncInterval 0.
- Vector TSO is effectively false. Scalar TSO and memcpy/set TSO remain
  true, half barriers false, real suspend enabled. Guest main creation QoS
  succeeds; main/render/encode priorities remain 47 during the decline.

## What the new counters establish

All 110 five-second windows return valid hardware counters and identify
level 0 as Performance and level 1 as Efficiency. The figures below are
cycles divided by running time on the respective level, not instantaneous
clock registers. Startup/loading and gameplay have different workloads.

| Seconds after launch | FPS in window | P average GHz | P running ms | E running ms | Thermal / low power |
| --- | ---: | ---: | ---: | ---: | --- |
| 36.0 | 155.8 | 2.890 | 12555 | 1019 | nominal / off |
| 41.0 | 98.9 | 2.806 | 9186 | 869 | nominal / off |
| 46.0 | 45.8 | 1.457 | 5373 | 6825 | nominal / off |
| 51.3 | 38.7 | 1.224 | 2522 | 9962 | nominal / off |
| 61.3 | 33.0 | 1.274 | 4017 | 8532 | nominal / off |
| 332.5 | 38.6 | unavailable (no P execution) | 0 | 11223 | nominal / off |

The P-frequency change also occurs in each of the main, Unity graphics and
DXMT encode threads, so this is not an aggregate ratio distorted solely by
moving between core types. Their native IDs remain unchanged. At 330–560
seconds only 0.154% of measured process CPU time is on P cores; the game
threads are also almost entirely on E cores. A P GHz value of -1 in those
windows means no P execution to measure, not a zero-frequency processor.

This confirms a real loss of CPU execution speed/residency accompanying
the FPS decline while the public thermal state remains nominal. It does
not identify the private OS policy responsible or establish a fixed energy
budget. Estimated CPU energy reaches 168 J around 41 seconds; that is not
proof of the ~250 J threshold described in BCD's separate RDR2 experiments.

RAW renders the early loading screen at 330–332 presents/sec on a 120 Hz
panel. Estimated CPU power is 4.7–5.3 W during the busy startup interval.
Avoiding those surplus loading frames is a concrete energy-saving candidate;
this report alone does not show that a 60 FPS limit sustains 60 in gameplay.
Do not silently override the player's saved FPS mode.

## Other limits and exclusions

- App adaptive cooling stays disabled; no app-imposed 30 cap appears.
  Late Present calls take about 0.0–0.1 ms; frames arrive 26–29 ms apart.
- Shader cache starts with hits and no shader translation misses. Pipeline
  native creation averages 0.22 ms, maximum 2.22 ms at the last report. These
  sparse events do not explain the several-minute sustained decline.
- Completed GPU interval coverage is roughly 77–81% late, with about
  21–22 ms of completed GPU intervals per frame (two buffers/frame). This
  is not an on-chip utilization or GPU-clock measurement. CPU slowdown is
  established, but GPU work also exceeds a 16.67 ms budget in this trace;
  simply raising CPU scheduling priority cannot promise 60 FPS.
- Collector cost averages 6.17 ms every five seconds, maximum 13.24 ms;
  no thread suspension is used. This is far too little average work to
  explain the sustained factor-of-two decline, though sampling can perturb
  individual frames.
- Every synchronization report says `counter-export-not-ready`. There is
  **no measured lock-wait duration** in this report; unavailable is not zero.

## Evidence-supported corrections

1. BCD's tested tag contains `LSSupportsGameMode=true`; our 691 plist has
   `GCSupportsGameMode` and the games category but lacks the newer key.
   Add the documented key while preserving the old one. This requests OS
   Game Mode support; it neither proves activation nor overrides CPU clocks.
2. Correct passive sync reads to translate the PE DATA export through the
   existing native `ios_jit_translate_addr_for_owner(export, game_peb)`.
   ARM64EC executes from a JIT copy and updates that copy's data; the old
   collector reads the stale original mapping. `server_ios.c` already uses
   this owner-aware translation. Keep bounded VM reads and validate magic /
   counter frequency before accepting the live address. A host regression
   fixture must keep the original PE counters zero and the live counters
   separate, including a mismatched-owner rejection.

Neither correction has an on-device 60 FPS validation yet. Do not claim the
performance problem is fixed or infer a new per-game TSO setting from this
trace. Do not add the Sustained Execution entitlement as a speed boost:
Apple documents that it limits burst performance to the sustainable level
from launch, and it also needs provisioning support.

## Primary references

- [Apple XNU Recount: counter and energy semantics](https://github.com/apple-oss-distributions/xnu/blob/main/doc/observability/recount.md)
- [Apple WWDC25: LSSupportsGameMode and sustained execution](https://developer.apple.com/videos/play/wwdc2025/209/)
- [Apple sustained-execution entitlement](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.developer.sustained-execution)
- BCD tag `v0.1.11-pr1-test`, `app/Madeira/Info.plist` and Wine `sync.c`.
