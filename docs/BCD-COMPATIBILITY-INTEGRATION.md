# Reviewed BCD compatibility integration

Base: Madeira build 630, Wine 4f5b197, FEX 26859e1. Compared BCD stable
26f1edd and test df19c3c. The reporter's working BCD version is unknown.

## Included

- Wine upstream b7996f867cb7b5c28b66e358a342a62c6966308c by meshoklv:
  leave the ARM64EC syscall callback before the execute-request early return.
  Subsequent protect, allocate and free notifications must reach the emulator.
  Stored as an idempotent parent patch; rebuild ARM64EC ntdll, not only native
  Unix ntdll. This preserves syscall status and the existing notification order.
- Enable Wine's existing image-map callback guard before all game launches.
  Clearing the stuck callback flag must not expose the old FEX interval lock
  to reentrant emulator heap commits during loader notifications. This uses
  existing Wine behavior without generating or modifying FEX core code. Global
  cfg values and the explicit Dock opt-out still win. A production loader
  fixture exercises nested heap allocation and outer callback ownership.
- BCD 22e2b23's 1024 trampoline slots in the already reserved 16 KB page.
  No page budget increase, slot recycling or JIT allocator change. The existing
  fallback beyond 1024 lifetime allocations remains; a bounded diagnostic exposes
  that limitation. This reduces slot collisions, but does not eliminate exhaustion.
- BCD test's Mesa patch 0007: transform feedback overflow queries require the
  actual Vulkan extension; depth clamp also recognizes Vulkan's core feature.
  No forced unsupported extension. Rebuild Mesa against the existing pinned
  MoltenVK version and invalidate the backend identity through its file digest.
- XAudio2 2.8/2.9 built from the same Wine source and bundled FAudio implementation
  as 2.7. Validate direct exports and required DLL dependencies. No global DLL
  override or replacement of the existing 2.7 interface.

## Verification and distribution

The callback regression executes the actual production wrapper across success,
failure, current process, foreign process, the 64-request logging boundary,
recursive entry and bulk protection suppression. The trampoline regression
checks all slot addresses, unique TEB storage, instructions, boundary guards and
the finite-page fallback. Mac CI builds the native inputs and the complete app.

Native receipts bind the rebuilt ARM64EC ntdll to its patched source and binary
hash; Mesa's receipt binds its binary to all patch contents. Cached builds fail
if either source or artifact is stale. The native ZIP includes the new PE modules
and receipts. A successful host/CI build is not device or game validation.

## Candidates intentionally left for separate work

AVX emulation, pure-x64 no-copy, split JIT pools and graphics COM entry patching
affect executable mapping, invalidation or instruction behavior. They are not
part of this integration. Updating every Wine/FEX pin simultaneously would also
introduce unrelated clock, Mono, controller and memory changes.

Existing per-game swap/suspension, shader cache, learned pipeline warmup, input
mapping and thermal pacing remain. There is no automatic preset for Primal Planet
or Silver Pines without an exact executable/log and a known working comparison.
No FPS improvement or universal game compatibility is claimed.
