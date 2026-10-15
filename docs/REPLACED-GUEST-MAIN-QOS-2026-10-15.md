# REPLACED build 685: guest main thread scheduling

## Device evidence

Authenticated, untruncated report `870d06f5-ead0-446b-b6f8-69d561a259cf`,
sent at 10:04 on 15 October 2026: build 685 / `0b36c149`, iPad14,6/M2,
1408x648, RAW mode, thermal 0. Unity uses ClientWorkerJobs, with no
`-force-gfx-direct` argument. The valid Direct experiment on build 683 froze
under a persistent suspend while owning an FEX reader lock; it is not a
working performance configuration.

The guarded late detach skips at 10:02:45 with `no live debugger`. The earlier
request returned while the debugger still appeared attached, so the helper
did not falsely report a confirmed detach. No duplicate unhandled detach
breakpoint/JIT dump is present. Nevertheless, FPS still declines.

The new app-wide workload counters provide both fast and slow windows:

| Window ending at uptime | FPS | CPU ms/frame, all threads | Instructions/frame | Cycles/CPU ns | IPC |
| --- | ---: | ---: | ---: | ---: | ---: |
| 204346.011 | 59.4 | 36.43 | 166,356,361 | 2.7041 | 1.689 |
| 204361.263 | 50.4 | 45.43 | 208,370,199 | 2.7136 | 1.690 |
| 204366.515 | 39.2 | 69.42 | 203,984,316 | 2.1325 | 1.378 |
| 204381.765 | 30.0 | 84.00 | 215,834,309 | 2.2365 | 1.149 |
| 204397.264 | 31.6 | 83.23 | 213,924,652 | 2.0821 | 1.234 |

These are aggregate process counters, not physical GHz, game-only CPU time,
or per-core residency. The decline contains both more instructions per frame
and lower instruction throughput. Thermal=0 does not itself establish a
constant hardware clock. Slow-frame snapshots repeatedly show the unnamed
guest main thread at priority/current_priority **20**, while
UnityGfxDeviceWorker and Job.Worker 1 are **47**. Live JIT PCs and a loading
window near 100% CPU identify the busy unnamed thread; its source creation
attributes below independently specify priority 20.

## Confirmed source difference from BCD

Our `wine_process_start` still called
`pthread_attr_setschedparam(...sched_priority=20)` before creating the pthread
that executes `wine_process_thread`. Its later
`pthread_set_qos_class_self_np(USER_INTERACTIVE)` ignored the return value.
Fixed scheduling opts that pthread out of Darwin QoS; the subsequent request
can return EPERM. Our wineserver creation had already removed this pattern.

BCD test v0.1.11-pr1-test contains the guest-main correction, commit
`c2ed4216624fc542f0a9adcdaf41170a7eedfbef` / upstream PR #162. It requests
USER_INTERACTIVE through `pthread_attr_set_qos_class_np` at thread creation.
This app change was missed in the prior CPU/FEX/DXMT comparison. No FEX core
or Windows priority implementation change is required for this correction.

Primary references:

- [BCD thread-attribute change](https://github.com/llamajose/madeira-bcd/commit/c2ed4216624fc542f0a9adcdaf41170a7eedfbef).
- [Apple pthread QoS API contract](https://github.com/apple-oss-distributions/libpthread/blob/main/include/pthread/qos.h).
- [Apple pthread QoS attribute implementation](https://github.com/apple-oss-distributions/libpthread/blob/main/src/qos.c).

## Correction and verification

Request the intended QoS in the creation attributes and handle attribute
errors as launch errors. Log both self-promotion and observed QoS with
`[main-qos-v2]`. The existing slow-window priority snapshots and app workload
counters remain available to verify the deployed behavior.

`check-guest-main-qos.py` extracts the production creation block and QoS
probe into a C program, then runs actual pthread/Mach APIs on macOS. It first
reproduces the old fixed-priority thread and its failed QoS promotion. It then
requires the production pthread to start with USER_INTERACTIVE even before
self-promotion, accept that promotion, and have native priority above 20.
The IPA workflow requires this Darwin check. Windows explicitly skips it;
a Windows source check is not a substitute for the runtime result.

The QoS request does not pin a core or bypass iOS power/thermal policy. This
corrects a demonstrated scheduling mismatch, but an FPS gain and stable 60
in REPLACED are **not yet measured on the iPad**. The REPLACED strict TSO and
real-suspend compatibility settings remain an independent performance and
correctness question; changing them in the same comparison would obscure
the effect of this correction.
