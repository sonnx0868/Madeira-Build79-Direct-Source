# Decommit native integration review

Baseline: `work/beamng/apc-context-fix/Madeira`, SHA256 `f3269d77d5e9872c0f3506570de11b9a4b707b5753d356176e1ba2691417c1f4`. Exact disassembly/import/unwind evidence and hashes accompany this note. No device operations were performed by this audit.

## Capacity and ABI

`_decommit_pages` occupies `[0x10013bfb0, 0x10013c514)`, exactly **1380 bytes**. `_free_pages` begins at the exclusive end; there is no reusable gap. Keeping the original first 32 bytes and last 28 bytes leaves 1320 bytes for the replacement body.

The architectural frame-establishing prologue is 28 bytes; the eighth original instruction is `MOV x19,x1`. Both are retained byte for byte. Frame size is `0x190`; saved pairs are x26/x25 at SP+0x140, x24/x23 at +0x150, x22/x21 at +0x160, x20/x19 at +0x170, FP/LR at +0x180. FP becomes SP+0x180. The 28-byte epilogue starts at `0x10013c4f8` and restores those exact pairs, adds0x190 to SP, and returns.

The compact unwind entry at function offset0x13bfb0 is encoding `0x0400000f`; the next entry is0x13c514. The candidate preserves all unwind bytes. Its shared state machine uses one original frame and only the saved x19..x26 plus volatile x0..x17. It does not touch platform x18, x27/x28, or callee-saved SIMD registers. Local records and Mach outputs occupy SP+0x00..0x9b; the legacy refusal message uses outgoing variadic arguments there before records exist. The save area is never reused for locals.

## Exact callable helpers and data

| Purpose | Native address | Notes |
|---|---:|---|
| `anon_mmap_fixed` | `0x10012c3a0` | args x0 base,x1 length,w2 protection,w3 flags; check MAP_FAILED=-1 |
| Alias lookup | `0x10012ae68` | `ios_jit_anon_alias_lookup(base)` returns RW address or0 |
| `bzero` stub | `0x1009e1644` | x0 base,x1 length |
| `dprintf` stub | `0x1009e17f4` | legacy alias refusal only; Darwin varargs on stack |
| Original refusal format | `0x100adf907` | original literal, original five arguments preserved |
| `mach_port_deallocate` stub | `0x1009e1cd4` | w0 task,w1 object port |
| `mach_vm_protect` stub | `0x1009e1d40` | w0 task,x1 page,x2 size,w3 setmax=0,w4 rights |
| `mach_vm_region` stub | `0x1009e1d64` | w0 task,x1 &address,x2 &size,w3 flavor9,x4 info,x5 &count,x6 &object |
| `mach_task_self_` GOT | `0x100bc0e98` | load64 pointer from GOT, **then load32 task port from that pointer** |
| Host page mask | `0x100d5d270` | derive host page size mask+1 |
| Alias count / table | `0x100d29820` / `0x100d29828` | stride32: base,end,RW,RX; ignore base0 |
| Pool ledger count / table | `0x100d5d2b8` / `0x100d5d2c0` | stride24: offset,size,PEB |
| Pool RW / RX / size | `0x101073fa8` / `0x101073fb0` / `0x101073fb8` | contiguous64-bit globals |
| `pages_vprot` | `0x100d69320` | pointer to two-level guest4KiB metadata directory |

There is no out-of-line native `set_page_vprot_bits`, `ios_pool_live_overlap`, or `ios_jit_anon_alias_overlaps` to call. All three necessary loops are reproduced inline from the pinned source/native semantics. `kernel_writewatch_register_range` is empty under this Darwin configuration; the binary has no corresponding call/helper. No new import or guessed `vm_region_64` entry is needed.

## Caller status propagation is required and correct

The original compiler knew decommit always returned0. At `0x100134a6c` the caller executes BL `_decommit_pages`, then at `0x100134a70` branches directly to `0x100134aa0`, where it overwrites the status with0 and writes output pointers. That would hide every new failure.

The single caller instruction changes **`0c000014` to `0a000014`**, redirecting the branch to the existing `0x100134a98` status path:

- `0x100134a98 MOV x24,x0`
- `0x100134a9c CBNZ w0,0x100134978` (shared unlock; no output pointer writes)
- Success falls through at0x100134aa0, writes address/size output pointers, and then unlocks.

The common unlock calls `server_leave_uninterrupted_section` at0x100134984, restores result from x24 at0x100134988, and returns normally. This is the same error path already used by `free_pages`; no new return/unwind path is added. The candidate explicitly writes W0=0 on every success before final bookkeeping and returns defined NTSTATUS values on every failure.

## Candidate and rejection risks

Candidate build artifacts are in sibling `native/`. Current core is1064 bytes including original32-byte prefix, followed by288 bytes of unreachable NOP padding and original28-byte epilogue. Only the1380-byte function interval and the caller4-byte instruction can differ from the pinned baseline; build assertions verify that and unchanged unwind data.

The replacement rejects partial pages overlapping any live alias or either pool mapping, executable current protection, inadequate maximum rights, query count/coverage failures, failed opening or exact requery, unsuccessful interior remap, nonzero readback, and failed restoration/requery. Both edge preparations occur before interior remap. Requested bytes are zeroed and read back only while the whole host page is verified R/W; restoration runs in reverse on every applicable exit. COMMITTED bits are cleared only after success. An alias-at-base keeps the historical live-ledger refusal and associated log.

Review must reject any future variant that uses an imported task-port pointer as the port value, changes save offsets without unwind metadata, drops reverse restoration, changes flags before failure is known, introduces VM_PROT_COPY/EXEC, treats MAP_FAILED as success, or omits the caller status patch. Host fixture tests cannot prove Darwin API behavior or physical game success. A kernel restoration failure is reported but cannot be made atomic: earlier zero/remap operations may already have occurred. Device regression checks remain necessary before another BeamNG launch.
