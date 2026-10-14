# REPLACED build 680: sustained low FPS

Full authenticated server reports downloaded on 15 October 2026:

- `46b4a41a-93df-4def-bfc2-a2d9ade1da4f` (03:02), 60 FPS mode.
- `7310a1e1-afe0-460a-b00b-654bc00a6726` (03:09), MAX mode.

Both are build 680/source 2df4a491, iPad14,6/M2, 1408x648, thermal=0,
low-power=0, adaptive cooling disabled during gameplay, reusable runtime off.
These are separate launches, not a frame-matched replay of one camera path.

## Checks against the last proposed fixes

- Native host probe contains LRCPC2=1; FEX explicitly reports LRCPC2=true,
  AFP=false. The omitted probe from build 679 is fixed in actual execution.
- The per-game four-processor override is absent. The log's cpuReported=auto
  field names the setting; it is not a direct reading of the guest PEB.
- Liquid Metal stops before Wine/game rendering, at 03:00:14 and 03:07:25.
- The CPU block cache's real compile count settles at 40,535 while slow
  gameplay continues. A 99% CPU block-cache hit rate is not a shader hit rate.
- Shader cache hits increase with no initial misses in the 60-mode run.
  Pipeline creations average about 0.15–0.16 ms, maximum about 1.05 ms;
  the existing log does not show repeated long pipeline builds as the cause
  of every sustained 30-FPS frame.
- 60 mode reaches 55.5 FPS and later 32–35 FPS. MAX is confirmed by both
  fpsMode=0 and display_intent=120Hz; it initially reaches 71–76 FPS and
  later falls to 30–33. Changing that setting has not removed the slowdown.
- MAX-mode late Present gaps are roughly 33–36 ms, time inside Present
  mostly below 1 ms, and drawable acquisition typically about 0.1 ms.
  This rules out treating the late drop as an observed hidden 30 cap or
  a long nextDrawable wait. Work or waiting can occur earlier than Present.

## Remaining measurement gap and prepared change

The old passive CPU sampler requires 500 ms without a Present. It therefore
captures load/save freezes but never a game producing a regular frame every
33 ms. Its lack of a CPU report during this slowdown proves nothing about
CPU load. Source now samples at most 12 slow five-second windows, excluding
an explicit 30-FPS selection and zero-frame stalls. It records app CPU/I/O
and the busiest native threads without suspending threads or reading guest
memory. Thread CPU usage is labeled as a percentage rather than Mach units.

GPUStartTime/GPUEndTime describe command-buffer intervals. The old diagnostic
adds their durations; overlapping work on multiple queues must not be read
as total GPU utilization or directly multiplied into a definitive per-frame
GPU bottleneck. Source now merges a bounded completed batch of intervals,
including out-of-order completions, and reports its union/span separately
from the sum. Capacity overflow is explicit. This remains interval coverage,
not an on-chip active-unit utilization measurement.

These changes correct the missing evidence. They do not change game graphics
or prove an FPS improvement. A further performance fix must follow the
measured CPU/queue/render costs; no hardware limit or BCD equivalence is
established by these two reports alone.
