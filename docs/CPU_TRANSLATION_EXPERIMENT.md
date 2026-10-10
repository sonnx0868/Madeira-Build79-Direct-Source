# CPU translation tools and experimental backend cache

Target for a first device comparison: the DX11/Legacy route. No GTA V file,
device gameplay trace or frame-rate baseline is present in this experiment.
The prototype is not a replacement CPU emulator and does not accelerate a
game unless that game/library actually uses a future integrated batch path.

## What the current source establishes

- Wine and DXMT already use ARM64EC. Replacing every small function with a
  native equivalent is not automatically faster: the boundary has register,
  stack, callback and exception work. Existing VC runtime exemptions in
  `WineProcessBridge.m` also document exception-path failures, so replacing
  arbitrary runtime DLLs would mix performance and compatibility changes.
- The pinned FEX Windows `Common/ImageTracker.cpp` maps AOT files but still
  contains TODOs for `CodeCache::LoadCache`, `RegisterMappedCodeBuffer` and
  `EnableLoadedSection`. Enabling `ENABLECODECACHINGWIP` alone does not enable
  reuse of translated ARM64 game code in this Windows path.
- Madeira now carries a fork integration in `build/fex-arm64ec/backend-cache.patch`
  and `madeira_code_cache.h`. Bootstrap applies it locally and rebuilds the
  ARM64EC translator. The upstream source pin stays unchanged. These generated
  integration changes must not be submitted as upstream FEX contributions:
  `FEX/AGENTS.md` prohibits AI-generated contributions there.

## Candidate architecture changes

| Candidate | Expected benefit | Required correctness work | Status |
| --- | --- | --- | --- |
| Coarse native work / API batching | Amortize x64↔ARM64EC entry/exit over many operations | Exact results, call ordering, resource lifetime, immediate-return APIs, fallback | Built-in CPU comparison; game API facade still requires a separate implementation |
| Persistent ARM64 backend cache | Avoid ARM64 emission for verified, identical optimized IR | Translator/config/CPU identity, literal repairs, debug/PC maps, unchanged SMC and JIT publication | Implemented as an optional Madeira integration; device validation pending |
| Profile-guided multi-block compilation | Spend expensive optimization on measured hot paths | Safe code replacement, flags/register equivalence, exceptions, indirect branches, compile-work scheduling | Instruction budget controls and emission/cache telemetry implemented; execution profiles and tiered recompilation remain unimplemented |

A persistent code cache mainly changes warm startup. It does not automatically
improve the quality of the generated instructions or eliminate the first
translation of dynamically generated Mono/V8 code. Likewise, the native
packet prototype does not imply that arbitrary closed-source game logic can
be moved to ARM64 without porting or verified translation.

## The runnable experiment

Settings › Send diagnostic log › Run CPU translation comparison stages the
bundled tools into `drive_c/Madeira/Tools/translation-lab-v1`. It uses the existing
launch gates/JIT flow and is a transient library session. There is no manual
ZIP extraction, new reusable-engine bypass, or replacement of game DLLs.

`build/translation-lab` compiles identical synthetic integer work into an x64
EXE and an ARM64EC DLL. It compares 65,536 helper calls, 1,024 calls with chunks
of 64, and one complete native batch against the local x64 implementation.
Every result must equal the local checksum. Native PE validation requires
CHPE metadata, native code ranges and generated entry thunks, not just a
machine-name label.

The Windows x64 control run verifies algorithm equality and loading. It uses
an x64 control DLL and measures no ARM64/FEX overhead. Actual crossover
measurements require running the packaged native helper through Madeira on
the iPad. Results are stored in `C:\madeira-translation-lab.txt`, and new
diagnostics collectors include that known file only within the report's
session time window.

## The implemented CPU cache

Game details › Compatibility & performance offers Off (default), Verify
generated code, and Reuse verified code. CPU experiments require a separate
engine; restart Madeira between launches. The built-in comparison selects
Reuse, which learns and verifies unknown blocks before permitting later reuse.
Clear CPU code cache removes the current build's cache and disable marker;
restart Madeira first so there is no existing in-memory store.

- The authoritative x86 decoder, IR generation, optimization and register
  allocation still run. The entire IR and node buffers are compared exactly;
  the hash is only an index. This is a backend cache, not the incomplete
  Windows AOT loader and not a complete removal of translation work.
- Keys include a build/compiler/emitter identity, host CPU features, backend
  configuration, exact guest entry/size, register allocation and constant
  branch targets' current ARM64EC bitmap classification. A changed binary,
  ASLR address, IR, CPU or configuration misses and uses the compiler.
- First emission learns a normalized record. A second independently emitted
  identical record promotes it. Verify always emits fresh code. Reuse accepts
  promoted records, restores metadata, repairs the dispatcher linker literal,
  and enters the original temporary-buffer migration and instruction flush.
  Guest addresses stay exact; this version does not rebase cached game code.
- Delivery/unpublished compiles, trap-flag stepping, AOT generation and native
  thunk relocations are excluded. No TSO, SMC, exception, code-page tracking,
  JIT alias or executable-page protection rule is disabled.
- A mismatch uses fresh code, disables the store and writes a persistent disable
  marker. Truncated/corrupt/oversized files are rejected. Windows takes one
  writer share lock; another process can read but cannot interleave writes.
  Contended cache/registry locks skip caching and keep the normal compiler
  available; a force-terminated holder cannot park another game thread there.
- Files and in-memory data are bounded to 48 MiB per store and 16,384 records.
  Stores do sequential record writes without additional game threads and do
  not depend on CRT exit destructors flushing a buffer. File
  loading, hashing, copying and writes have costs; reduced stutter is not yet
  demonstrated. New app builds use separate file identities.
- `[cpu-cache]` logs attempts, actual reused records, rejection counts and
  average/maximum emission-plus-cache time. That duration excludes frontend
  IR work and the later JIT migration/flush. It is not an execution-hotness
  counter or frame-time measurement.

The optional 512/2,048/5,000 instruction budgets change the existing FEX
multi-block compiler's size limit. Automatic preserves its 5,000-instruction
default and configuration. Smaller blocks can reduce individual compilation
pauses while increasing dispatch overhead. No automatic tuning decision is
made without device measurements.

## Verification gates and practical limits

- Production C++ cache tests: restart/promotion, mismatch disable, concurrent
  writes, corrupt/truncated/oversized files, failed writers and fuzzed record
  length/offset decoding. macOS runs with AddressSanitizer/UBSan.
- Production Swift staging and environment tests: trusted bundle receipt,
  symlink containment, report selection and per-game settings reset.
- The ARM64EC DLL is rebuilt from pinned FEX, including this actual hook;
  native preflight rejects old translators. Reusable native bundles include
  the new translator and built-in tools.
- Windows x64 smoke validates the packet algorithm. Cross-compilation validates
  compilation/linking, not ARM64 execution. iPad correctness, warm-cache hit
  rate, loading time, p95/p99 frame time and GTA V FPS remain device tests.

## Decision after the device run

If coarse batches materially beat local translated work and fine-grained
native calls, investigate an x64 command-recording facade and ARM64EC replay
for a limited set of high-frequency library APIs. For Direct3D, a viable
implementation must preserve COM identity/refcounts, queued resource
lifetime, update/map ordering, immediate queries and synchronization. Unknown
operations must flush and call the existing implementation. A working facade
then needs graphics correctness and frame-time comparisons in actual games.

If batches do not win, do not ship a new facade merely because it reduces the
number of transitions. Profile cold compilation, data loading and generated
CPU code instead. Use the same device, scene, resolution/FPS setting and
thermal state; compare p95/p99 frame times and loading times as well as FPS.

References: [Microsoft ARM64EC ABI](https://learn.microsoft.com/en-us/windows/arm/arm64ec-abi),
[FEX ARM64EC architecture](https://wiki.fex-emu.com/index.php/Development%3AARM64EC).
