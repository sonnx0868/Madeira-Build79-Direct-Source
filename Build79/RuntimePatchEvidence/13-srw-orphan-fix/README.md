# Anonymous SRW orphan mutation correction

The graphics-freeze capture shows an anonymous game SRW being forcibly cleared by Madeira's missing-stamp heuristic, followed by invalid releases, owner-count underflow to 65535, and a prolonged main/worker stall. Detailed evidence and limitations are in `../graphics-freeze/native-review.md`.

`build.py` pins the current Decommit-v1 native and the original Wine source. It changes one four-byte ARM64 call to NOP, preserves a local baseline, verifies the entire byte delta and unchanged unwind metadata, and writes a separate source correction copy/patch. It does not edit the shared source tree, build an IPA, or contact a device.

- Baseline native: `6e09addbcea3478bb2fb4c6989703351849f0aff160fe3d4c49eb8e3ed5697a8`.
- Candidate native: `91f4bc3acb90ed4dafa7fbf7cea5087f8c32ae9044990a6f785b116567e43d90`.
- Site: VA `0x10011c728`, file offset `0x120728`, `d6fcff97` → `1f2003d5`.
- Removed action: speculative `ios_srw_reap_exclusive(lock, 0xDEAD)` from `ios_orphan_check`.
- Retained: diagnostics, detector metadata reset, normal SRW acquire/release/wake, unrelated explicit-dead-owner reaper call, and all other native code.

The detector still counts strikes per waiter and uses historical diagnostic wording. Those observations no longer authorize mutation after this change. Do not infer a dead owner from the remaining diagnostic lines. This correction prevents new speculative clears; it cannot repair a lock already corrupted in a running game.

Run the source tests with `python3 work/beamng/srw-orphan-fix/test_host.py`. The runner extracts the exact original/corrected detector and reaper, plus pinned Wine SRW release/try-acquire implementations. The fixture uses a real mapped low-address lock, real `msync`, and atomic compare-exchange. It controls an adversarial interleaving: an owner remains inside its critical section while the detector runs and a second actor attempts the real try-acquire. The baseline permits overlapping exclusive ownership in one census with three waiters; the candidate preserves the owner until ordinary release and wake. Controls cover five waiters, matching/unrelated stamps, fewer/no waiters, shared ownership, incoherent lock shape, and the observed 0→65535 release underflow.

`host-test-report.json`: **10 distinct cases, 20 executions** across baseline/candidate, all pass their respective expected outcomes under ASan and UBSan. LeakSanitizer is disabled because the sandbox's ptrace prevents its thread enumeration. The fixture is deterministic, not an OS-scheduled stress test. Wake functions are instrumented; no claim of Darwin Mach delivery or game stability follows from it. The independent agent owns the native ARM64 harness and review artifacts.

No device fix is claimed. The intended physical test uses a fresh app/game process at the same persisted settings and 896 MiB pool setting. This bounded correction does not explain the earlier 1152 MiB invalid-instruction trial, nor guarantee that all graphics workloads fit device memory or run without further issues.
