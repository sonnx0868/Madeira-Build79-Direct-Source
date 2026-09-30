# ARM64EC ntdll static TLS isolation

The installed Wine ARM64EC `ntdll.dll` has its own 96-byte static TLS template, but the loader's special initial-ntdll path does not register that directory. Its initial TLS index is zero, which Madeira explicitly allocates to the main executable. Wine's exception/unwind code therefore accesses the executable's TLS block instead of its own.

This is now independently reproduced on the iPad by the parent's small, owned x64 executable: normally caught software exceptions overwrite its static TLS. Ordinary system queries and a stack-backtrace call do not cause that overwrite. This is an engine correctness issue; no game allocator/assertion bypass is proposed.

## Exact evidence

The shipped ARM64EC DLL has SHA256 `9d2ab14aa5a0d6148df05ccdbb704c10e1b990cae54340dde8a52e278a5e24ef`, 1,572,864 bytes. Its Git blob is `3e3703f30de8e1dc54ef9af8fce705b9738fba18`, exactly the DLL committed in Madeira `44b4b67e116b5bc813f7a35c9ed3b7d8ad3b5416`. The TLS directory is RVA `0xb6780`: raw template `0x100000..0x100060`, index RVA `0xdcb88` initially zero. It is a standalone ARM64EC PE. The raw COFF Machine field is AMD64/0x8664 in both shipped and rebuilt files; LLVM recognizes its ARM64EC hybrid metadata and presents the inferred ARM64EC format. No header conversion or machine-field mismatch is claimed.

Pinned `signal_arm64ec.c:111` declares `static __thread struct ec_stack_window ec_win`. Actual shipped instructions at RVA `0x5e170` load `TEB[0x58][*(DWORD *)(image+0xdcb88)]`, add `0x20`, zero 64 bytes, then write the window fields. The two bad BeamNG regions match these fields exactly:

| TLS offset | ntdll writer field | Captured BeamNG misuse |
|---|---|---|
| `0x20` | guest allocation low | outside the affected scope vector |
| `0x28` | guest allocation high | interpreted as vector begin; equals native `StackBase` |
| `0x30` | emulator stack low | interpreted as vector end; equals emulator allocation low |
| `0x38` | emulator stack high | overwrites vector capacity/state |
| `0x40` | last frame | overwrites scope state |
| `0x48` | last PC | overwrites the game's initialization flag |
| `0x50` | 32-bit steps | low word of invalid vector pointer: 12 |
| `0x54` | 32-bit budget | high word of invalid vector pointer: 65,536 |

The worker's `0x000100000000000c` is thus exactly **12 unwind steps and a 65,536-step budget**. An earlier observation that the same bytes could resemble `SYSTEM_CPU_INFORMATION` was a coincidence, superseded by this exact layout and on-device SEH reproduction. Native system-query disassembly is correct about its supplied destination and 12-byte length; it is not the identified writer.

Bounded shipped instructions are preserved at `../legacy/crash-audit/ntdll-ec-window-writer.asm`. The complete original disassembly is `ntdll-ec-text.asm` there. The parent's immutable device proof is in `work/beamng/cpu-tls-probe/captures/capture-ad0b9caa2c66484ea1e382154e4d76df/cpu-tls-probe.log` and the user-facing `outputs/CPU-TLS-IPAD-BEFORE-FIX.log/json`. The parent reports four threads started, 24 completed queries, three caught exceptions; the final worker timed out, so this is not a claim all four completed.

## Source change

`ntdll-static-tls.patch` changes only `dlls/ntdll/loader.c` from Wine `7817e220384e895651f868ba4d97affcf21b3816`:

1. Add an ARM64EC helper distinguishing an absent/empty initial-image TLS directory from failure to register a required one.
2. Keep the main executable's existing first allocation, but fail the current process on required allocation failure instead of allowing another module to take its slot.
3. Register the already loaded ntdll through its existing loader entry immediately after the main image and before `load_arm64ec_module()`. No numeric TLS slot is hardcoded. Existing `alloc_thread_tls()`, callback handling and cleanup initialize and maintain each module's independent storage.
4. Mirror the assigned index DWORD into the executing process-owned PE data copy using Madeira's existing naked `xlate_ios_jit` helper, as the loader already does for delay-IAT writes. Reject unexpected NULL translation or a failed volatile readback, and log the original/translated addresses and index. Broad ntdll protection/synchronization is deliberately avoided.
5. Correct the existing BOOL allocator's heap-failure `return -1` to `FALSE`; otherwise a required failure would falsely appear successful.

`build_ntdll_module()` creates a single-module loader node before this point. The new lookup uses the same `CONTAINING_RECORD(node->Modules.Flink, ...)` pattern as existing process-attach code. The existing loader critical section is held. An executable with no TLS reserves nothing; ntdll may then legitimately receive the first real slot. New processes initialize their own loader table; new threads receive copies through the existing `alloc_thread_tls()` path.

There is an inherited timing limitation: registration is before FEX initialization, but a thread whose `ThreadLocalStoragePointer` is still NULL receives actual storage at the later existing `alloc_thread_tls()` call. This patch does not reorder that early FEX/bootstrap interval or claim its callbacks already have TLS. It corrects the demonstrated overlap once normal thread storage is initialized. No callback or stack-walk safeguard is removed.

## Build and verification

`build.py` replays the exact local Wine-generated PE build commands into this directory, using Clang/LLD 22.1.8 and Wine's own Windows headers. Original and patched standalone ARM64EC DLLs are built from the same source and objects; only `loader.c` differs. Command and direct-file hashes key the owned object cache. `config-audit.json` checks all 36 ntdll PE translation units: none includes the Linux-generated `config.h`; target macros are ARM64EC/Win64, with no `__APPLE__`, `__linux__` or `FEX_IOS_HOST`. No native Darwin/ELF Wine object is linked.

The full ARM64X target cannot link the pinned fork's unrelated native-ARM64 `xlate_ios_jit` reference in this local configuration. That failure is preserved in `build/original/ntdll-arm64x.dll.log`; no stub was substituted. Madeira uses the successfully linked standalone ARM64EC form, so the candidate contains no native ARM64 implementation. The unmodified source emits existing format/declaration-after-statement warnings; those categories retain warning severity while other patched-loader warnings are errors.

Frozen candidates (debug information retained):

- Original rebuild: `build/original/ntdll.dll`, 4,259,840 bytes, SHA256 `fb15bf5faa29907f65ea3c8d344983fe6b709457aabc812f1910c024e71c8c2c`.
- Patched rebuild: `build/patched/ntdll.dll`, 4,259,840 bytes, SHA256 `8708f6d7222cb6ecbc5fecb5fd702a72b2f730f2ba98f8c5e81887713ab17ba5`.

`python3 work/beamng/ntdll-tls-fix/test_loader.py` extracts the actual three allocation routines from the patched source and passes **1,737 strict ASan/UBSan checks**, including independent main/ntdll blocks, existing/new threads, raw-copy and zero-fill contents, no-TLS executables, required main/ntdll allocation failures, distinct executing-index mirroring, NULL-translation rejection and partial thread-initialization cleanup. Caller-order checks verify required failures are consumed before proceeding. Host thread/heap services are mocked: these checks do not claim ARM64EC execution or game success.

`build-report.json`, complete compile/link logs, and `*.metadata.txt` preserve outputs. Independent review of final SHA8708f6d7…ab17ba5 passed: all 1,469 export names/ordinals/RVAs, all 516 Nt/Zw syscall entry bytes/IDs/RVAs, all 1,200 EC redirect sources and the 96-byte TLS contract match the shipped image; every redirect target lies in a published native executable range. Static/delay import directories remain absent. The peer independently reran all 1,737 allocation checks. Full results are under `../legacy/canvas-audit/tls-loader-review/`. This clears the identified source/ABI blockers for a controlled device test; it is not a successful runtime result. No installed DLL, IPA, native engine or game file was edited here. The parent will first repeat the isolated canary/exception test with the candidate, then evaluate actual games and keep rollback available.

Primary source: [Wine loader](https://github.com/willfaust/wine/blob/7817e220384e895651f868ba4d97affcf21b3816/dlls/ntdll/loader.c), [ARM64EC exception implementation](https://github.com/willfaust/wine/blob/7817e220384e895651f868ba4d97affcf21b3816/dlls/ntdll/signal_arm64ec.c), [shipped Madeira DLL](https://github.com/margooey/Madeira-actions/blob/44b4b67e116b5bc813f7a35c9ed3b7d8ad3b5416/app/Madeira/arm64ec-windows/ntdll.dll).

## Translation-hook startup proof

The exact pinned native source is saved as `evidence/loader_ios.c` (Git blob `95c30e0c44e21792db19e89e53052ae9cd61854f`, 129,403 bytes). Session loader lines 2071–2079 install and mirror `p_ios_jit_translate_addr`; child EC loader lines 2382–2384 do the same before returning the child's `LdrInitializeThunk` entry. The child's forward success is silent in runtime logs because that source branch logs only a missing export. Thus the existing hook is available before the new initial-image registration. Its mapped-owner selection is unchanged. Source `signal_arm64ec.c` explicitly skips ntdll during the later broad metadata refresh; this is further reason to use the narrow DWORD update instead of relying on a later general sync.

The source-provenance audit verified all 153 original ntdll source/header dependencies against the exact pinned Wine archive. The rebuilt original is a comparison artifact, not a byte-for-byte recreation of the prebuilt release; compiler/debug layout differences remain visible in the metadata.
