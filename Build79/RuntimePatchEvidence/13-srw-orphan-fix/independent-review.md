# Independent SRW orphan-fix review

The prepared candidate passes the focused exact ARM64 regression. No blocker was found for this four-byte change. Physical game stability is still unverified.

## Exact binary and control flow

- Baseline: `Madeira-original`, SHA-256 `6e09addbcea3478bb2fb4c6989703351849f0aff160fe3d4c49eb8e3ed5697a8`.
- Candidate: `Madeira`, SHA-256 `91f4bc3acb90ed4dafa7fbf7cea5087f8c32ae9044990a6f785b116567e43d90`.
- Independently parsed the ARM64 FAT Mach-O mappings. Only file offsets `0x120728`–`0x12072b` differ. At VA `0x10011c728`, `d6fcff97` (BL `0x10011ba80`, `_ios_srw_reap_exclusive`) becomes `1f2003d5` (NOP).
- The following instructions are unchanged: `STR xzr,[x20]` and the branch to clearing the suspicion count. Neither consumes the removed call's return value, flags, or caller-saved registers. The enclosing function restores its saved LR normally. The exact candidate preserves the separately qualified dead-owner caller at `0x1000fd418`, and that instruction still targets `0x10011ba80`.
- All bytes outside the single instruction are identical, including function prologues, epilogues, exception/unwind sections, and the reaper itself. This establishes unchanged unwind metadata; it is not a fresh semantic certification of the whole binary's unwind tables.

## Reproducible native regression

Command:

```text
work/beamng/fex-runtime-teb-fix/test-venv/bin/python work/beamng/srw-orphan-fix/test_native.py
```

The explicitly requested retry completed with exit 0: **12 distinct cases, 24 executions, PASS**. Results and input/script hashes are saved in `native-test-report.json`.

The harness executes every instruction of `_ios_orphan_check` (`0x10011c290`–`0x10011c7c7`) reached by each test. Baseline negative controls also execute the unchanged native reaper, including its actual LDAXR/STLXR owner-clear loop, diagnostics, empty wake-bucket traversal and return. The only mocked helpers are `msync` and `dprintf`; both deliberately clobber caller-saved integer/vector registers and condition flags. The native wake queue is empty, so this does not perform any real OS alert or scheduling.

The baseline reproduces the captured word change `00010009 → 00000008` with four unstamped waiters in one census. Three-waiter and five-waiter cases likewise demonstrate a live modeled owner being erased. The candidate makes no reaper call and retains the original coherent ownership word. It still counts diagnostics per waiter, including five strikes in the five-waiter case; the patch deliberately does not change that behavior.

Controls cover scalar and vector stamp lookup (including a scalar tail), clearing an existing suspicion in the last slot for stamped/released/incoherent locks, an exclusive lock with no queue, fewer than three waiters, failed `msync`, and waiter entries in the last three of the 512 slots. Every execution checks exact SP restoration, 16-byte stack alignment, preserved x18–x29, restored LR, preserved d8–d15, bounded stack writes, adjacent lock canaries, and unchanged waiter/stamp inputs.

The separate source fixture report was also reviewed: ten distinct cases / twenty executions pass under ASan and UBSan, including actual source release/try-acquire behavior and the corresponding owner underflow. That source fixture complements the native test; neither substitutes for physical concurrent execution.

## Why missing stamps cannot establish owner death

The retained source in `work/ipad-ui/audit/signal_arm64_ios.c:915` collects selected FEX lock stamps from live registered threads' TEBs at offsets `0x16e8` and `0x16f8`. Reads can fail, the set is capped at 64 entries, and the stamp protocol names particular FEX locks. It is not a complete owner registry for every Wine SRW lock. The PE SRW acquire/release instrumentation (`work/vulkan/build-audit/wine-source/dlls/ntdll/sync.c:742`) records events in a separate 512-entry ring; that ring is not the stamp set being tested here.

`sync.original.c:3839` iterates individual waiter records. For every matching waiter, it increments the same lock's strike counter. Thus three matching waiter records can satisfy “three strikes” in one census. The comment describing three consecutive monitor cycles is inaccurate. The coherence guard proves only a possible lock-word shape, not ownership provenance or owner death. Removing the anonymous no-stamp reaper call is therefore justified independently of the graphics workload.

The separate dead-thread path (`signal_arm64_ios.c:735`–769) uses different evidence and the stored FEX mutex stamp. Its caller remains byte-identical. This review does not certify that entire older death-detection policy.

## Captured freeze and confidence

Frozen `work/beamng/graphics-freeze/madeira-log.txt` has SHA-256 `c173d0af48aa523ee046949106b8f52033b95cb68c2f07cac1646d956e2bf058`.

At lines 315518–315520 the monitor records the same lock, `0x703f913050`, with word `00010009`, four waiters and three consecutive strikes. Line 315521 explicitly records the synthetic `dead_teb=0xdead` reaper changing that word to `00000008`. At lines 315532–315533, threads `00ec` and `008c` report exclusive and shared releases on that same lock with their expected ownership missing. The retained PE shared-release code logs the missing owner and nevertheless decrements the 16-bit owner field. The later `ffff0010` word and eight waiters at lock+2 are consistent with this exact underflow and a poisoned lock, while the presentation count stops advancing.

This provides a concrete recorded mutation, matching native reproduction, and matching source failure mechanism. It strongly supports the unsafe orphan reaper as a cause of this freeze. It does not reconstruct every concurrent owner handoff or prove that no independent graphics issue remains. Shader work and the graphics preset change may increase contention; their timing alone is not proof of a shader compiler failure. Earlier Metal background errors occurred far before the final presentation count and do not explain this immediate lock-word corruption.

The candidate removes this unsafe mutation without changing graphics settings, resolution, the lock representation, or ordinary release/wake operations. No device action or candidate-binary modification was performed by this review.
