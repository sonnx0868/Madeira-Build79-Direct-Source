# REPLACED build 679: remaining FPS drop

Report: `8d45dc5c-4c45-4320-8594-7e39bbe4fc74`, uploaded 23:23 on
2026-10-14 (Asia/Bangkok). Full 1,552,703-byte report downloaded from the
authenticated Diagnostics browser to Downloads. Not truncated. Device
iPad14,6 (M2), resolution and drawable both 1408x648.

## Observed timeline

These are averages over approximately 10-second windows, not instantaneous
minimum FPS. GPU numbers are per completed command buffer, not per frame.

| Time | FPS | RAM MB | Thermal | Average GPU ms / command buffer |
| --- | ---: | ---: | ---: | ---: |
| 23:21:56 | 57.3 | 2428 | 0 | 3.14 |
| 23:22:06 | 43.1 | 3902 | 0 | 3.65 |
| 23:22:17 | 30.0 | 6122 | 0 | 3.00 |
| 23:22:27 | 56.4 | 6022 | 0 | 7.40 |
| 23:22:59 | 45.0 | 6045 | 0 | 10.06 |
| 23:23:30 | 38.4 | 6080 | 0 | 11.98 |

- Cooling disabled: requested=1, applied=1, enabled=0; all gameplay samples
  retain fpsMode=1 and thermal=0, low-power=0. There is no observed Madeira
  thermal 30-FPS cap in this session.
- The transition to 30 FPS coincides with a scene's memory/load growth;
  the later sustained reduction coincides with increasing GPU command time.
  GPU metrics report zero command-buffer errors and up to four in flight.
- DXMT reports two command buffers per frame. Near the end its average
  interval between Present calls is 26–28 ms; time inside Present is
  0.0–0.1 ms. Drawable wait is roughly 0.03–0.05 ms. The late window's
  sampled sum of GPU command durations is approximately 24 ms/frame
  (11.98 ms * 806 buffers / roughly 403 frames); this is an estimate, not a
  frame-correlated GPU measurement or a proof of exclusive GPU causation.
- Shader/pipeline caches are active. This fresh native compiler identity
  initially misses the old cache: pipeline misses rise from 1 to 280 during
  startup and to 384 later. This can contribute scene/loading spikes; it
  does not by itself explain all sustained frame time.
- FEX's 99% CB_SUMMARY hit rate is the translated CPU block cache, not the
  shader cache. It must not be used to claim shaders are fully warmed.
- Cached CPU mappings and commitBeforePresent are confirmed active by
  native markers, so merely adding those patches again cannot fix this log.

## Confirmed remaining source wiring bug

FEX reports `ml1231 ... LRCPC2=false AFP=false (probe none)`. The app only
called madeira_publish_host_probe when the optional i386 farm existed.
This IPA has no i386 farm, but its newly integrated ARM64EC CPU feature
selector also consumes that probe. Thus the x64 route never received it.

Source follow-up publishes the existing bounded sysctl probe before
__wine_main for every launch. Upstream feature selection still requires an
explicit LRCPC2=1, preserves disablelrcpc2 and the AFP opt-in rule, and
does not enable unsupported instructions on unknown/older devices. No
FEX core source was generated or edited for this change.

This closes a missed integration step; it does not prove restored 60 FPS
or remove the measured graphics workload. The change is not in IPA 679.
Next device comparison should verify the actual probe and GPU/Present
timings in the same scene; do not label this log solely a thermal-cap bug.

## Exact BCD default comparison follow-up

Read the already verified tag v0.1.11-pr1-test, not its moving main branch:

- BCD WineProcessBridge.m line 2002 calls madeira_publish_host_probe
  unconditionally. The source follow-up now matches this wiring.
- BCD Library.swift lines 523–524 exports MADEIRA_CPU_COUNT only for an
  explicit choice. Its Wine system.c otherwise uses the host count or the
  global cpu-count setting. Our automatic Unity profile instead forced 4,
  which this report confirms as cpuReported=4 on the 8-core device.
  Source follow-up removes that automatic override; explicit per-game CPU
  choices remain intact. This is a default mismatch, not a measured proof
  that reporting 8 restores 60 FPS.
- Both native DXMT build scripts use -O2. A Debug IPA label does not mean
  these native graphics objects were compiled unoptimized.
- ResourceInitializer upload flushing, staging rename budgeting and shader
  parsed-program release remain reviewed upstream candidates. The latest
  report's initializer live/peak is 4 MB, staging ring live 72 MB with peak
  164 MB, while private textures reach 2464 MB. This does not establish a
  staging-upload backlog as the cause of its sustained FPS drop.

The estimate of GPU command time describes our current path, not the
minimum cost of running REPLACED on M2. In particular it cannot establish
that BCD would miss 60 FPS in the same scene, or that no software regression
exists. Matched BCD scene/renderer/native settings remain necessary to
attribute the remaining difference beyond the two confirmed CPU defaults.
