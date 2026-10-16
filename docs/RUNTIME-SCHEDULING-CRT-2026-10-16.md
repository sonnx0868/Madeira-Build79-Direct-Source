# Timed sleeps and builtin CRT selection, 16 October 2026

## Evidence and attribution

The current REPLACED report `95f90a6f-d90a-4412-af87-b6f29fb60215`
is build716 at1920x1080. It retains requested/applied60, nominal public
thermal state, low power off and adaptive cooling off. Game threads retain
base priority47, but the main thread is frequently sampled with current
priority0. Almost all late CPU execution is on E cores. The report does
not expose the private performance-controller recommendation and cannot
establish its cause. Time spent blocked can also produce a current-priority
snapshot of0; that field alone does not prove a scheduler-yield call.

Source inspection identifies an avoidable scheduler interaction:
`NtDelayExecution` calls `NtYieldExecution` even for a nonzero timed sleep,
before parking in `select`. That reaches Darwin `sched_yield`. Apple's
[CPU job scheduling guidance](https://developer.apple.com/videos/play/tech-talks/110147/)
explains that yield can depress current priority to0 and delay execution
for up to10ms. A timed park already relinquishes the CPU. Adding that
independent depression is unnecessary for the requested nonzero sleep.
The same Wine path exists in the BCD reference; this is a shared-runtime
correction, not evidence of a new BCD-only fix. It does not pin P cores,
change thermal policy, set a new FPS cap or prove sustained60FPS.

The Party Animals Off report `de1283e8-69e7-4409-958a-5b332fbc6c00`
and On report `5e857933-c254-4a2b-8ba3-02d30375bcd7` both contain the same
early MSVCP140 header jump. Turning suspension On did not introduce that
fault. The Off run later stops presenting and reports a worker waiting
over60s for a `_Lockit` critical section owned by the game main thread.
The source filename `msvcp90/misc.c` is shared CRT implementation, not
proof that this lock belongs to the VC90 DLL.

Both traces load bundled hybrid MSVCP140 followed by game-local native
VCRUNTIME140 (image size0x1b000). The existing bridge requests all three
CRT overrides as builtin, but `loader_ios.c::load_builtin` honours only
the disabled load order and otherwise unconditionally keeps the selected
file. Thus `=b` never replaced the native DLL. Disassembly of the actual
shipped MSVCP140 identifies the first failing call as
`_Yarn_char_op_assign_cstr -> #memcpy`. The native fallback uses an exit
thunk address at module RVA0 and reaches the MZ header. Subsequent locale
cleanup faults at `locale_dtor+0x40` on a null receiver. The later lock
stall may be a consequence of interrupted initialization, but these logs
do not establish the complete lock ownership cycle or explain30599.

## Corrections

- iOS nonzero timed sleeps park directly. Sleep(0) still calls
  NtYieldExecution. Alertable waits/APCs, infinite waits, remaining timeout
  calculation and non-iOS semantics are preserved.
- Explicit builtin and builtin-then-native preferences now use only the
  app-staged PE farm for the calling architecture. The existing
  `virtual_map_builtin_module` validates the Wine-builtin flag and machine;
  no `.so` search or file replacement is introduced. Missing builtin-only
  files fail; builtin-then-native falls back on ordinary missing/unsupported
  results. Access errors are not silently converted into successful loads.
  Default/native choices and already selected Wine builtins retain their
  existing path.
- Rebuild exactly `loader.o` and `sync.o` in libntdll_unix.a and require a
  source/object/archive receipt. Every other member, including the signal
  handler, remains byte-identical. FEX, PE DLLs, renderer and wineserver
  are outside this rebuild.

## Verification

The production function/header fixture passes on Windows: architecture
farms, load-order choices, absent/rejected files, handle cleanup, timed
and zero waits, absolute/relative/expired deadlines, interruptions,
alertable/APC and infinite semantics. CI additionally requires actual
Darwin timed waits to park without spin, retain QoS and make no synthetic
yield calls. Native compilation/linking and artifact receipts are mandatory.
Device FPS/core residency and Party Animals matchmaking/loading outcomes
remain unverified until this native build is exercised on the iPad.
