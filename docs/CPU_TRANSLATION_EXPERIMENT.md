# CPU execution research and first prototype

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
- `FEX/AGENTS.md` forbids AI-generated contributions to that project. This
  prototype changes no FEX source or pin, memory-ordering/SMC rule, executable
  page protection or game binary.

## Candidate architecture changes

| Candidate | Expected benefit | Required correctness work | Status |
| --- | --- | --- | --- |
| Coarse native work / API batching | Amortize x64↔ARM64EC entry/exit over many operations | Exact results, call ordering, resource lifetime, immediate-return APIs, fallback | Standalone prototype built |
| Relocatable persistent CPU code cache | Reduce cold startup/recompilation of previously seen code | Module/content identity, CPU features, ABI/config identity, relocations, unwind/PC maps, SMC invalidation, iOS RX/RW mapping | Windows integration incomplete in pinned FEX; proposal only |
| Profile-guided multi-block compilation | Spend expensive optimization on measured hot paths | Safe code replacement, flags/register equivalence, exceptions, indirect branches, compile-work scheduling | Proposal only; not implemented here |

A persistent code cache mainly changes warm startup. It does not automatically
improve the quality of the generated instructions or eliminate the first
translation of dynamically generated Mono/V8 code. Likewise, the native
packet prototype does not imply that arbitrary closed-source game logic can
be moved to ARM64 without porting or verified translation.

## The runnable experiment

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
