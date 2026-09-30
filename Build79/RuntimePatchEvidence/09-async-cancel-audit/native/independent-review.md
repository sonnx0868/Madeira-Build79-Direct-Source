# Independent review: native async cancellation correction

Reviewer: `async_native_review` subagent. Review performed September 9, 2026. The native review was initially read-only; this report is the only file added by the reviewer.

## Decision

No blocking defect was found in the reviewed native correction. The evidence supports proceeding with a controlled physical-device diagnostic. This is not a claim that the iPad test has passed, that the exact CEF teardown has been reproduced by the probe, or that BeamNG's initial UI failure is fixed.

Reviewed executable hashes:

- Baseline GuestExit-v2: `c3125a1266952a6a1fbc14928755c917654da309fec09f9642287524d49126cd`.
- Candidate: `f6ef9798133cfd487ee3decc83ec7cb0f762e2952b78b25967820a4efa1ec984`.
- Pinned `server/async.c`: `a8673f9332acab7289e1914244b25f929ee837f9571eaa3b20174430b37267ad`.

## Native implementation review

The candidate replaces only the 640-byte `_req_cancel_async` range `[0x100054dec, 0x10005506c)`. The build reports 429 differing bytes and identity outside that function. Source, assembly, emitted disassembly, reports, original request-handler disassembly, relevant pinned Wine source, and actual executable symbols were reviewed.

All helper and global virtual addresses in `native/build.py` match the pinned executable's symbol table. Structure access offsets match the original native handler: process list `0x118/0x120`, async process entry `0x60`, owning thread `0x48`, FD `0x78`, IOSB selector `0xa0`, flags `0xd0`, cancellation group `0x100`, group sync `0x48`, and group count `0x50`.

The correction retains each selected async before a callback can synchronously complete it. Group membership is attached before cancellation. The construction sentinel prevents early group destruction while the rest of the operations are being added. Retained entries are restored before releasing their temporary reference; restoration reads the current tracked-list head again after every potentially destructive release. This addresses the original lifetime hazard without skipping cancellation or cleanup.

If all operations complete during construction, no wait handle has yet been exposed: releasing the empty group and returning a zero handle is consistent with completed cancellation. If operations remain pending, the remaining group reference is completed by the existing cancellation-completion path. The original assertion against conflicting group membership is preserved.

The request wrapper preserves handle lookup and target release, object/thread/IOSB filtering, termination and system-operation exclusions, already-canceled handling, invalid-handle behavior, allocation failure status, reply values, and process-wide `STATUS_NOT_FOUND` behavior. Creating the sentinel before internal-sync allocation in the native body has no observable effect on the allocation-failure destructor, which only releases a non-null sync object.

The unchanged prologue and epilogue save and restore the same callee-saved registers and frame layout. The stack remains 16-byte aligned. The code does not repurpose Apple platform register `x18`. The reply slot at `sp+8` and temporary list at `sp+0x10..0x1f` do not overlap saved registers beginning at `sp+0x20`. Original epilogue and cold assertion-call offsets are preserved; zero padding is bypassed by an explicit branch. The compact-unwind entry inspected with `llvm-objdump --macho --unwind-info` is `0x0400001f` for `[0x54dec, 0x5506c)`, consistent with the unchanged frame.

## Independently executed checks

The existing compiled source fixtures were rerun without rebuilding or rewriting their reports:

- Fixed implementation: **81 lifecycle cases passed**, all fixture-owned objects released.
- Original implementation negative control: expected ASan heap-use-after-free, exit code 1.
- Keepalive-only negative control: expected `!async->async_cancel` assertion, terminated by SIGABRT.

ASan and UBSan remained enabled. LeakSanitizer was disabled because the host ptrace sandbox prevents its thread scan; explicit allocation accounting is not a full-process leak scan.

The reviewer also executed **110 additional complete-handler Unicorn cases**, using the existing lifecycle-helper model without rewriting its reports:

- 24 cases: both thread-only settings, synchronous/deferred completion, and 0, 1, 2, 24, 48, or 96 operations.
- 6 cases: earlier-operation completion, completion during restoration, and future-operation completion, with both thread-only settings.
- 16 cases: invalid handles, group allocation failure, sync allocation failure, and handle allocation failure, with both thread-only settings and both completion modes.
- 64 deterministic randomized cases with seed `682527`: 32 operations each, mixed completion, object/thread/IOSB filters, terminated and system-operation exclusions.

All 110 passed the existing model's ABI, request-status, reply, cancellation-selection, group, list-integrity, and allocation-balance assertions. These supplement the builder's recorded 148-case run. They are not independent implementations of the helper model and must not be represented as physical native execution.

## Windows diagnostic review

Also reviewed `AsyncCancel.c`, `build_probe.py`, emitted PE headers/imports/unwind information, shortcut construction, and the existing host-Wine report.

Reviewed hashes:

- `AsyncCancel.c`: `2981463785f462ad8f8b203f52d07f7566e0d8b79f0db3cd0934d53201dbfb51`.
- `build_probe.py`: `2740590ffd9a8b7a274a581f19853e101192647e4f22a0f33d9413d6b9460519`.
- `MadeiraAsyncCancel.exe`: `84136602707fa8dba09ae0d437ba935bf4e3f254b046464d519910d77214feea`.
- Shortcut: `a5180f251910fc9357f04f1a6ea2d09d1548bea1e5c09f40e5e5c344bef620b8`.

No blocking ABI or test-validity defect was found for this diagnostic's intended scope. It is an AMD64 PE32+ executable with the expected kernel32 imports and unwind records. Its explicit `OVERLAPPED` representation has the correct 32-byte size and event offset 24 for Win64; pointer-sized internal fields, DWORD fields, HANDLEs, WCHARs, and imported function declarations are consistent with the target ABI. The program exits through `ExitProcess` and does not rely on an absent CRT initializer.

The named pipe uses duplex access and overlapped server I/O. Every canceled read must first report `ERROR_IO_PENDING`; each completion must signal its event and report `ERROR_OPERATION_ABORTED`. Forty-eight rounds rotate through `CancelIo`, `CancelIoEx(handle, NULL)`, and four IOSB-specific `CancelIoEx` requests. Each round subsequently transfers and verifies a byte using the same pipe. The existing host-Wine report records all 48 rounds passing: 192 canceled reads and 48 subsequent byte transfers. The reviewer inspected that report but did not rerun the host-Wine probe.

Pinned ntdll source confirms that `NtCancelIoFile` sends `cancel_async` with `only_thread=TRUE`, while `NtCancelIoFileEx` sends it with `only_thread=FALSE` and the optional IOSB selector. Thus the three modes exercise the intended group and process/IOSB paths. Since all reads originate from one live thread, the probe does not establish exclusion of operations from a different thread or reproduce a dead-thread APC completion.

The shortcut target is `C:\MadeiraDiagnostics\BeamNG034\MadeiraAsyncCancel.exe`; deployment must use that location. The log is recreated beside the executable. The pipe name contains the process ID, and the test does not open game files or target unrelated processes.

Diagnostic limitations: three-second event deadlines may produce a timeout under exceptional emulation stalls; the failure label is meaningful, but `GetLastError()` after a wait timeout may be stale and must not be treated as the wait's cause. A blank console alone is not a pass; the fresh diagnostic log supplies the result. The probe does not prove the exact dead-thread CEF crash trigger or full concurrent CEF teardown.

## Remaining validation boundaries

The native Unicorn tests instrument object/FD helpers; the source fixtures extract real production completion/APC bodies into reduced surrounding infrastructure. Neither executes the whole native server on an iPad. This review does not verify final signing, the installed signed payload, real device behavior, or the separate initial CEF startup/painting failure. The root agent owns packaging, installation verification, and physical-device testing.
