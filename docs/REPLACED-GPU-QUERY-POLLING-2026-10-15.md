# REPLACED 697: thermal decline and repeated GPU query polling

## Reports

- `1b86351d-d082-4b90-bab3-ecfa27cb0e5d`, 12:49 +07: user reports
  charging while playing. P average falls from 3.15 to about 1.2 GHz;
  after a minute virtually all game work is on E at about 1.6 GHz.
  Public thermal state becomes serious at 12:44:38.
- `b84e95a8-d42a-4917-9d04-aaeb0941e57c`, 12:55 +07: user reports
  unplugging for this run. P still declines and the app reaches serious
  at 12:55:08, about 48 seconds after launch. Charging cannot explain the
  entire problem. The logs do not measure charger power or die temperature.

Both are complete build-697 reports. Producer pacing is active; the old
330 FPS RAW loading burst is gone, replaced by about 120 FPS. Mode is 0,
adaptive cooling is off, low power off, and there is no app 30 FPS cap.
Vector TSO remains off; other memory settings and the native core are unchanged.

For nine earlier 695 windows and nine 697 windows with the same 26+2 render
passes / 12+22 clears, CPU instructions/frame average 98.6 versus 100.8 M,
but CPU time/frame increases from 36.1 to 53.0 ms. E average rate is 2.41
versus 1.59 GHz. Main-thread instruction throughput falls almost exactly in
proportion to that rate. These are workload-signature matches, not proof of
identical gameplay or measured on-glass FPS: 695 RAW deliberately counted
skipped presents, which 697 no longer does.

## Existing counters expose a specific source of wasted CPU

In the simple loading screen, build 697 repeatedly reports
`event_stall_max=73155..77999` with only 6+0 render passes. These are maxima
of the actual per-frame `event_stall` field. `d3d11_context_imm.cpp::GetData`
increments it whenever a GPU query returns S_FALSE on its flushing path.
It is not a count of shader compilations or critical-section waits.

The UnityGfxDeviceWorker concurrently consumes about 91% of a CPU core.
Against 695's comparable loading phase, reducing counted frames from about
320 to 120 reduces total process instruction rate only from about 15.7 to
15.1 billion/sec. Graphics-thread instructions per frame rise from about
34 to 109 M. Pacing one producer while the caller retries GPU queries can
therefore retain most of the CPU work. This explains why the 697 pacing
change alone does not adequately reduce startup power.

The counter includes query kinds other than Event. It proves excessive
pending-query polling; existing logs do not enumerate every query kind.
Unity frame-completion Event queries are the first targeted path. The next
device log must show query-park-v1 and a reduced event_stall count to establish
the fix is active and reaches the dominant polling path.

## Implemented correction

After the existing 64 fast Event-query checks, retain the established flush
behavior and park on that query's exact Metal shared-event sequence for at
most 1 ms. The existing native wait wakes when the GPU signals; this is not
an unconditional millisecond sleep. Flush's local device-lock scope ends
before the wait. Pending queries remain S_FALSE, including when the wait
times out. A subsequent GetData must observe the real GPU watermark before
returning S_OK. Already-ready, invalid, ordinary short polls and unrelated
query paths keep their behavior. Issuing a query again resets its threshold
and event sequence.

Only the ARM64EC D3D11 frontend is rebuilt. The existing Metal wait ABI is
already present in winemetal.dll / the native renderer. No FEX core changes,
new periodic profiler, clock override, thermal cap or launch option is used.

## Verification and limits

The host fixture extracts and executes the actual GetData method, EventQuery
state machine and queue wait wrapper. Tests cover short DONOTFLUSH polls,
the existing stalled flush path, exact target, a one-ms timeout, genuine
completion, invalid arguments, status-only polls and reissue. Real ARM64EC
frontend compilation and the linked DLL/import/source receipt are required
before publishing. Device FPS, power and clock improvement are unmeasured
until the resulting build is exercised; thermal protection can still reduce
performance, and non-Event query polling may require separate work.

Primary API contracts:
[Microsoft GetData](https://learn.microsoft.com/en-us/windows/desktop/api/d3d11/nf-d3d11-id3d11devicecontext-getdata),
[Apple Metal shared-event wait](https://developer.apple.com/documentation/metal/mtlsharedevent/4322276-wait),
[Apple device temperature management](https://support.apple.com/en-us/118431).
