# REPLACED transition measurements after build 687

The two build-687 reports are `5f6c5266` (vector TSO on) and `04bbb979`
(off, verified by FEX's effective configuration). Late FPS remains about
29–31 in both. The second run reaches thermal serious, so it is not a
temperature-matched measurement of the setting's isolated effect. Neither
run identifies physical per-level CPU clock/residency or exact wait owners.

## Collector

REPLACED automatically enables `ReplacedMetrics.h` through DiagnosticEvents.
`env.MADEIRA_REPLACED_METRICS=0` disables it. It does not modify game/runtime
settings. Five-second windows include loading, fast gameplay and slow
gameplay; at most 120 windows and 128 registered Wine threads are retained.

- `replaced-cpu-v1`: process rusage v6, CPU time on performance/efficiency
  levels, cycles and instructions, running-time-weighted average GHz/IPC,
  runnable time, estimated CPU energy/power, thermal and low-power state.
- `replaced-thread-v1`: native and Windows thread IDs, process ID, names,
  per-level running time, cycles/running-time ratios, instructions and native
  priority. Only the selected game process's eight busiest registered threads
  are printed. Helper-process work still appears in the process-wide total.
- `replaced-sync-v1`: the game's existing PE ntdll `ios_xp_nt` DATA export,
  read through bounded Mach VM reads. Reports completed contended
  critical-section wait time, counts, histograms and address-wait call counts.
- `replaced-sync-lock-v1`: up to three frequently sampled critical-section
  addresses from the existing ring. Reports truncation and the owner/current
  state when sampled; this is not proof that this owner caused a past wait.
- `replaced-metrics-cost-v1`: collector wall-clock overhead and registered,
  valid, unavailable and dropped thread counts.

The existing native thread-attach callback supplies IDs and PEBs even when
Reusable runtime is off. Registry/measurement work uses a private serial
queue. Retained Mach rights are released on session end or replacement.
No thread suspension, affinity changes, guest writes, GPU waits, FEX edits or
new native binary rebuilds are involved.

## Interpretation limits

Average GHz is cycles divided by running time for the named performance level,
not a readout of instantaneous frequency. Per-thread level names are only
identified as P/E when the OS reports their mapping. Unsupported hardware
counters, denied queries, short results and resets are explicit; zero
unsupported counters must not be interpreted as a frequency collapse or
all-E residency. CPU energy fields are kernel estimates and exclude GPU
energy. A thread's time off CPU is not automatically synchronization time.

The synchronization measurements cover **completed critical-section waits**,
not every event/semaphore/condition/Metal wait. A long wait can start in one
window and complete in another. The sum across parallel threads can exceed
wall time. Counters come from the existing measured PE code; an unavailable
export is logged as unavailable, never as zero wait time.

## Verification

`check-replaced-metrics.py` executes production counter math and the actual
Darwin collector/export resolver. Checks include timebase units, work versus
rate, counter rollback and missing counters, the exact 40-byte Darwin
threadcounts records, supported/denied/truncated results, malformed remote
addresses/exports, duplicate attach and lifecycle reset. The test resolves a
synthetic PE DATA export without executing PE code. The cached native bundle
was separately inspected and contains `ios_xp_nt` in ARM64EC ntdll at RVA
`0xd4150`. Host tests do not establish device FPS or physical clock values.

## Same-release library correction

Continue Playing includes recent manually imported EXE entries and store
entries, with up to twelve horizontally scrolling cards instead of three.
Stale details/import saves preserve the newest recorded play date. Private
games, temporary installers, Desktop and uninstalled Steam entries are
excluded. Settings / Appearance adds a persistent Show Continue Playing
toggle. Old dates already overwritten cannot be reconstructed; playing the
game records its date again.

## Celeste investigation

The supplied Celeste 1.4.0.0 EXE is PE i386, CLR flags `0x3` (ILONLY plus
32BITREQUIRED), and imports mscoree. Its FMOD/FMOD Studio libraries are also
i386. Build 687 contains the ARM64 WoW64-side modules but its i386-windows
directory has only `.gitkeep`; the 32-bit Windows DLL farm is missing.
The displayed message is a pre-launch compatibility rejection. Clearing the
EXE's required-32 flag would leave its native dependencies incompatible.
This app/measurement change does not add or claim a working Celeste runtime.
