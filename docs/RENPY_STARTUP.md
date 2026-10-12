# CrossRealms startup failures in build 612

Report: `6fe47584-2ab6-4dfe-a6e0-71ac60fdc0df`, CrossRealms 0.4.1,
Madeira 0.1.6 build612 (`e036bc3`), iPad14,6 / iPadOS27.0.1.
Launch at 19:29:01 on 2026-10-12 (UTC+07); returned to library at 19:30:08.
All recorded display samples have FPS0 / drawable0x0. No game frame appeared.
Physical footprint reaches roughly 1.76GB; the log does not show an OOM kill.

## Evidence and changes

### Relocation directory read outside the copied image

The first native fault is `NtProtectVirtualMemory+0x71c`, reading
`0x7027004000`. The native archive built609 and used by IPA612 was inspected:
that offset is `ldrh w3,[x0],#2` in the DIR64 relocation entry loop. The old
loop checked neither the directory against image size nor each block against
the remaining directory. Its block-size register contains `0xaa1503e2`.

`ios_pe_relocations.h` replaces this walk with checked directory/block/entry
ranges, 64-bit RVA arithmetic, unaligned reads/writes, and validation before
any fixup. Malformed tables and fixups that rewrite the directory are skipped;
ordinary protection changes still return their original status. This prevents
the unsafe read; it does not establish why that image's relocation metadata
was invalid in the first place.

### ARM64 shell helpers execute ARM64EC ntdll

Python starts `C:\windows\system32\cmd.exe /c "ver"`. These helpers report
machine `0xaa64`, with no CHPE CPU area, but the loader clones the session's
ARM64EC ntdll. Their PCs repeatedly fault at child ntdll RVA `0x612b0`, addr0.
The ntdll DLL inspected locally was SHA256-verified byte-for-byte against IPA612.
The instructions immediately before that fault load `[x18+0x1788]`, then read
a byte through that pointer. This is the CHPE CPU area that native children
intentionally do not have. Six helper TIDs collectively log 6,135 repeated
`setup_exception for SEGV` messages.

The existing private EC-ntdll loader is generalized to select the child's
machine. Pure ARM64 children of an EC session now load the bundled native
ntdll; AMD64 children of a native session keep the existing private EC path.
CHPE redirection/bitmap initialization is restricted to the AMD64 case.
Same-architecture and WoW64 paths retain their existing selection.

### Worker exit incorrectly becomes process exit

TID005c performs `RtlExitUserThread` / self `NtTerminateThread` with code0.
The Unix side immediately takes `exit_process`, closes the process socket, and
wineserver kills PID0020's still-present TIDs0024 and0048. The code in
`exit_thread` reaches that branch only when its shared `nb_threads` drops to0.
That counter assumes one Windows process per Unix process, while Madeira's
children are additional pthreads in the same Mach task and their main threads
do not take the normal thread-creation increment path.

`ios_process_threads.h` tracks registrations/reservations per PID+PEB+TID,
including every process's main thread. Reserve and attach deduplicate; create
failures cancel reservations; exits remove only their owner's thread. An
unknown exit or incomplete registry never means zero. Explicit process exit
retires the owner's records. Non-iOS builds keep Wine's original counter.
This counter is independent of the optional reusable runtime's liveness
registry and does not decide when JIT memory may be reused.

## Verification and remaining work

`check-renpy-startup.py` compiles the production portable helpers and checks
both cross-architecture directions, six child exits without consuming parent
counts, duplicate/unknown exits, concurrent reservations, PID reuse, saturation,
valid unaligned DIR64 fixups and malformed/overflowing metadata. Windows execution
passes. macOS repeats the fixtures with ASan/UBSan early in bootstrap.

New native markers: `private-child-ntdll-v1`, `process-threads-v1`,
`bounded-reloc-sync-v1`. Cached old native bundles are rejected. A full bootstrap
is required; no new IPA is built or published at the time of this source change.

The earlier source613 change reports a clean exit before the first frame and
attaches recent Ren'Py `log.txt`/`traceback.txt` from the selected game's folder.
Build612 predates that capture and this report has zero companion logs. Real
CrossRealms launch/graphics compatibility must still be verified on the iPad;
these fixes are not a measured guarantee that the game now launches.
