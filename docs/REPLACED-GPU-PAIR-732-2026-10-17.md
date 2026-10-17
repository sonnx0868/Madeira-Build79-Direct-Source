# REPLACED build 732: original smooth/slow comparison

Input: `25410052-9ff6-4198-b94d-5439129c4465`, supplied on Windows. Both
captures validate as complete, with two actual Present calls and four command
buffer commits. Original capture/session files are preserved.

## Verified observations

| Observation | Smooth | Slow |
| --- | ---: | ---: |
| Median FPS in final five observations before recording | 57.96 | 43.00 |
| Native capture bytes | 1,235,820,318 | 1,174,134,802 |
| Decoded command records | 8,927 | 8,957 |
| Render encoders over two frames | 130 | 128 |
| Draw calls over two frames | 2,184 | 1,886 |
| Indexed indices × instances | 1,858,122 | 1,347,342 |
| Compute dispatches over two frames | 100 | 100 |
| Compute pipeline handles used | 26 | 26 |
| GPU event/fence wait calls | 0 | 0 |
| CPU `waitUntilCompleted` calls | 3 | 3 |

The complete sequence of compute pipeline handles and dispatch grids is
identical. Its SHA-256 is
`d18abd369bb9b245076502b1156f4e6d456559e81b2f2353c370cc0bb83a7eb7`.
Input buffers/textures do change; this does not establish identical shader
branches or fragment coverage. The slower capture has fewer indexed draws
and indices. There is no increasing draw/pass/dispatch count explaining the
slowdown in these two samples.

The stable session window ending at uptime 401844.980 averages 58.3 FPS,
with main-thread P-core cycle rate 2.772 cycles/running-ns. The window ending
at 401855.490 averages 47.6 FPS, with rate 1.991. Main-thread P/E execution
is 1,582.372/9.777 ms and 1,579.880/13.261 ms respectively: it stays almost
entirely on P cores. These are per-window counter-derived rates, not an
instantaneous hardware clock read. Public thermal state is 0 and low-power
mode is false in both windows.

The text log shows bounded GPU submissions and no command-buffer errors.
Completed GPU timestamp intervals cover almost the full later reporting
windows. That interval coverage includes elapsed command-buffer execution;
it is not a hardware utilization or per-pass limiter counter.

Pipeline creation is sparse after startup. A binary archive save fails once
and a later save succeeds. This report does not establish repeated shader
compilation as the sustained bottleneck.

DXMT calls the CPU completion wait from its finish thread
(`dxmt_command_queue.cpp::WaitForFinishThread`), which retires completed work.
The API listing does not provide per-call thread timing. The presence of
these waits is not evidence that the game/encode thread synchronously waits
after every commit. Removing the retirement wait would violate resource lifetime.

## How the command inspection was verified

The Mac runner lacks the standalone `gpudebug` executable. The diagnostic
uses the installed Apple `DYCaptureArchive` reader and function enum table.
All 58 encountered opcodes resolve. Apple produces exactly one decoded line
per original command record in both files; bounds and final stream offsets
are checked before pairing labels with arguments. The generic Apple tracer
leaves function names empty, so the installed enum table supplies the labels.
These diagnostic private Xcode interfaces are never linked into Madeira.

The input was transferred as authenticated AES-GCM ciphertext through an
unpublished temporary draft asset. The key is provided only to the private
analysis job. Raw textures and environment/account data are not published.

The reserved `gttrace-dump` item is also inspected through Apple's archive
reader in both samples: it is only the 15-byte placeholder `temporary data`
plus a terminator. `MTLCaptureDescriptor` contains the capture options/path.
No embedded original-device hardware performance profile is identified.

Reproduce with downloaded private Apple-reader artifacts:

```
python3 tools/analyze-gpu-comparison.py /path/to/comparison --apple-output /path/to/apple-artifacts --output /path/to/report
```

## Causal gap and next measurement

These files establish the command structures and the counter-derived CPU
rate reduction. They do not establish why the OS reduced the rate, original
GPU performance-state transitions, or which GPU pass/limiter dominates.
The slower sample is not a controlled fixed-image replay with hardware
profiling. No root-cause FPS fix is claimed or shipped from this analysis.

`tools/profile-device-performance.py` is prepared to collect graphics,
selected-process/system telemetry and rate-limited energy observations on
the original iPad through the existing developer transport. It inspects
condition information without applying any condition. It changes no game
setting and does not launch, kill or suspend the app. Unsupported counters
and service errors remain explicit. Whole-display Core Animation FPS is
kept distinct from the game's Present FPS in the log.

At preparation time Windows usbmux enumerates zero connected devices.
Original-device profiling still requires connecting/unlocking/trusting the
iPad. No additional setting comparison is substituted for that missing
measurement. A defect identified in the shared Wine/DXMT/Metal path would
be fixed there for all games using it; this capture alone does not establish
a universal FPS improvement.

References: [Apple GPU profiling workflow](https://developer.apple.com/documentation/xcode/optimizing-gpu-performance),
[original-device Instruments measurements](https://developer.apple.com/documentation/xcode/analyzing-the-performance-of-your-metal-app).
