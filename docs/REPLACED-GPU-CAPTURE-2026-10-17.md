# REPLACED GPU comparison

The full build 725 report `a19973b6-3d5a-4bec-af0d-b4c9d3cf3791`
shows slower frames in parts of the session, increasing completed GPU buffer
intervals and Present waits, with nominal public thermal state, no low-power
mode, no adaptive 30 FPS cap, and mostly P-core main-thread execution. It
does not identify an individual expensive pass, a hardware GPU clock, a
fixed-scene comparison or an unbounded queue. Cache saves both succeed and
fail; compilation is not established as the sustained cause.

## Concrete capture path

In the running REPLACED game, open Madeira's Session menu and select
**Record FPS drop**. It resumes gameplay, waits five seconds, then requires
five stable one-second observations at 50 FPS or more for the reference.
Keep the same scene, resolution and FPS mode. There is no automatic shader,
quality, scheduling, synchronization or FPS setting change.

The reference records two complete presentation frames. After a 30-second
recovery interval, five consecutive observations below 80% of the reference
request the low FPS sample. **Capture low FPS now** is a manual alternative
after the reference: it closes the menu and waits five seconds before capture.
Menu samples, background time and long timer gaps are excluded. Changing the
FPS mode or leaving the foreground stops the comparison. No baseline is
silently taken from a 30 FPS scene or an initial loading screen: the user
starts recording after entering the scene of interest.

If the scene never reaches 50 FPS, **Capture reference now** records the
current scene after five seconds and marks the reference as manually selected.
Its actual pre-capture FPS remains in the manifest; it is not claimed to be
a 60 FPS sample.

The local native DXMT bridge marks a presentation buffer, waits for that
buffer's commit, then starts MTLCaptureManager **before creating the next
command buffer on that queue**. Every recorded frame must end in a committed
presentation buffer created during capture. Old buffers, unrelated queues,
and stale generations cannot complete a recorded frame. The capture stops
after the second frame, at 32 created buffers, or at a 15-second timeout.
Cancellation/limits are marked incomplete. A capture already active at start
is rejected. The watchdog cannot preempt a blocked Metal system call.
Ordinary inactive hooks return after an atomic check. Native queue/resource
ordering and producer pacing remain intact.

`MetalCaptureEnabled=true` enables Apple's public programmatic GPU trace
support in this diagnostic build. Apple's framework instrumentation has a
small CPU cost even when not recording. Recording itself can pause the game
and consumes storage; it is opt-in. The 2 GB free-space preflight is a minimum,
not a hard bound on the size of a two-frame capture. No guest code is modified.

## Export and analysis

The native `.gputrace` documents, manifest and bounded session log remain in
**Files → Madeira → GPUTraces → comparison UUID**. The Session menu and the
diagnostics screen offer **Prepare GPU comparison ZIP**, then **Share GPU
comparison ZIP**. Packaging uses a background file coordinator and does not
load the trace into a Swift Data buffer. The captured session log is saved
when capture finishes, so a later game/session cannot replace it on export.
The existing 20 MB text log uploader cannot carry GPU trace binaries.

After extracting the ZIP, validate both complete samples:

```
python3 tools/analyze-gpu-comparison.py /path/to/comparison --output /path/to/report
```

On a Mac with Apple's `gpudebug`, add `--replay` for the command/performance
inventories. Explore the actions advertised by that tool to compare passes,
draws, shader pipelines, render-target formats, dimensions, load/store actions
and dependencies. Replay timings on another GPU do not establish original
iPad timings or its clock. A programmatic workload capture is not an Instruments
Metal System Trace and does not by itself identify the OS power governor.

No REPLACED trace has yet been recorded on the user's iPad with this new path.
There is no claimed FPS correction until the two actual samples are analyzed.
Host tests execute the real capture state policy, FPS detector and Objective-C
controller with a fake Metal destination, plus the ordered source patch
pipeline. Device capture and replay remain separate required validation.
On a macOS runner with capture-capable Metal, an additional real API canary
records two offscreen clear workloads and checks the resulting document with
Apple's `gpudebug` if available. A runner without GPU capture reports an
explicit skip. This canary does not substitute for the actual iPad trace.

## Primary references

- [Apple: programmatic capture and MetalCaptureEnabled](https://developer.apple.com/documentation/xcode/capturing-a-metal-workload-programmatically)
- [Apple: capture manager records buffers created and committed inside capture](https://developer.apple.com/documentation/metal/mtlcapturemanager)
- [Apple: GPU debugging from the command line](https://developer.apple.com/documentation/xcode/debugging-with-interactive-command-line-tools)
- [Apple: autonomous GPU trace investigation](https://developer.apple.com/documentation/xcode/investigating-gpu-issues-with-ai-agents)
