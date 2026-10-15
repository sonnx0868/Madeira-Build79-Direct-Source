# REPLACED build 695 and producer pacing correction

## New evidence

Report `879dd495-fc0f-42fc-b5aa-bb77e481607f`, uploaded 12:08 +07 on
2026-10-15, is the complete current-session log from build 695 / `fd9583c`.
It retains 1408x648, RAW mode 2, effective vector TSO off, other memory
ordering settings unchanged, nominal thermal state and low-power mode off.

The fixed live DATA reader works in all 49 windows. During the FPS decline,
completed critical-section waits total about 0.2–6.8 ms per five seconds,
across the measured process. This does not identify critical sections as
the sustained bottleneck. Other wait types are not timed by this counter.

CPU P-level average rate falls from 2.8–3.0 GHz to 1.2 GHz around 46–56
seconds. CPU energy estimates reach about 196 J at 46 seconds. The loading
interval renders 320–327 synthetic presents/sec and peaks at an estimated
6.2 W of CPU power. These figures support reducing wasted startup work;
they do not prove a specific thermal sensor, fixed energy threshold or
private OS power policy.

Later the game returns to 64–68 FPS with P rate still near 1.19 GHz. Near
the end it falls to 40–55 as CPU instructions per frame and render passes
increase. Low P clock alone does not determine FPS, and neither the Game
Mode metadata nor a scheduling-priority override fixes all this workload.

## Two concrete faults in the old presentation path

1. RAW skips acquiring a drawable until **18 ms after the previous acquire**.
   Every skipped frame has already simulated/rendered, increments the FPS
   counter, and never reaches a drawable. At a stable 60 game presents/sec,
   16.67 ms is less than 18 ms, so the gate accepts alternate frames: about
   30 visible submissions/sec while CPU/GPU still process 60. Faster loading
   renders hundreds of discarded frames. This is source-derived behavior,
   not a measured on-glass FPS count from this report.
2. The normal 60/30 modes only ask Metal for afterMinimumDuration. That
   constrains display scheduling, without a precise producer deadline. A
   frame that misses the display interval can be rounded to another vblank.
   The existing native producer limiter is off by default; its single global
   deadline would also couple independent producers if enabled wholesale.

## Implemented correction

- Pace at native nextDrawable with monotonic absolute deadlines. Native
  mach_wait_until parks the producer; no CPU spin or relative sleep ladder.
- A frame already late proceeds immediately. Do not pay an extra 16.67 ms
  after a 17–20 ms frame, or bank catch-up bursts after a stall.
- Keep per-producer, per-surface deadline state and reset on a live mode /
  display change. Distinct game windows do not share a global deadline.
- Present normally after that deadline; do not also apply a second
  afterMinimumDuration constraint.
- Retire RAW mailbox skipping and synthetic skipped-frame counts. Saved
  mode 2 becomes Display maximum; normal choices are 30, 60 and the panel
  maximum. The CPU/GPU producer respects the physical panel maximum.
- Preserve the existing explicit MADEIRA_PRESENT_LIMITER=0 compatibility
  opt-out. Ordinary iOS sessions use the corrected path automatically.
- Rebuild only winemetal_unix.o and compare every archive object against
  the previous bundle, including duplicate LLVM object filenames. All
  other Wine/FEX/PE/renderer objects are preserved.

This addresses a demonstrated source fault and unnecessary work. It does
not override the device's power controller, prove why it chooses a lower
frequency, or guarantee 60 FPS for a frame exceeding the CPU/GPU budget.
Do not change TSO, real-suspend or quality settings for this comparison.

## Verification

The host fixture compiles the actual patched production pacer. It covers
30/60/panel rates, 17/20 ms slow frames without 30 FPS rounding, interrupted
and failed waits, resume, mode changes and independent surfaces. On Darwin
it also executes the actual mach_wait_until path and checks elapsed versus
CPU time to reject a busy-spin implementation. The complete ordered patch
pipeline is applied twice to clean pinned sources. A receipt binds source
hashes and all archive members to the producing commit. Device frame rate,
power and stability still need validation with the resulting IPA.

Primary platform guidance:
[Apple: graphics performance and sustained versus burst performance](https://developer.apple.com/documentation/metal/improving-your-games-graphics-performance-and-settings),
[Apple: CPU scheduling and avoiding polling](https://developer.apple.com/documentation/xcode/scheduling-cpu-work-efficiently).
