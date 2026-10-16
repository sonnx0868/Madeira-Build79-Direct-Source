# Hookable ARM64EC swapchain entries

## Evidence and scope

Tester 721 Fading Afternoon crashes after an x64 E9 detour overwrites the
ARM64 implementation of `MTLD3D11SwapChain<false>::Present`. The original
COM vtable exposes the native implementation directly. An x64 hook installer
can therefore obtain an ARM64 patch target from an otherwise valid COM call.
The exact writer of the detour is not identified. Its DLL is useful for
attribution and reproduction, but not required to correct this ABI contract.

Microsoft documents `hybrid_patchable` for functions whose address must be
an x64 fast-forward sequence (FFS). An unmodified FFS can redirect efficiently
to its ARM64EC body; a patched FFS contains x64 instructions on an x64 code
range instead of overwriting native instructions. Clang supplies the matching
native dispatch thunk and the compiler/linker supply hybrid metadata.

The parent-owned DXMT patch applies the production ABI annotation to Present,
Present1, ResizeBuffers and ResizeBuffers1 on iOS ARM64EC only. These are shared
swapchain operations commonly intercepted together; no game name or DLL identity
selects the change. Rendering, resolution, swapchain creation and synchronization
logic remain intact. No hook is disabled, no third-party DLL is replaced, and
no game authentication/ownership/DRM path is changed. FEX source is untouched.

## Verification

The host fixture uses the pinned genuine ARM64EC compiler, not an x64 control
labelled ARM64EC. It verifies the actual vtable entries, x64 code-map ranges,
canonical FFS bytes, native implementation/dispatch symbols and redirection
metadata. The pre-fix build and a vtable redirected straight to an ARM64 body
must fail the same gate. The compiled production D3D11 is checked for all four
methods in every emitted swapchain template, and recorded in
`build/dxmt-pe/swapchain-hooks.json`. D3D11 export names must be preserved.

Only the EC D3D11 frontend needs rebuilding. The new receipt also updates the
PE identity in the existing GPU receipt while preserving its native-object
producer and archive hash. A newly rebuilt native archive must be independently
restamped by its own producer. Cached bundles without the compiled hookable
entries are rejected.

These are static compiler/PE and build checks. They do not execute a third-party
hook, emulate the complete mixed-ISA call cycle, or establish that the tester's
game is now playable. The tester must validate the new build. A hook that follows
past a valid x64 FFS into the native body may still be incompatible; that case
would need additional evidence. Celeste input is a separate unresolved issue.

## Primary references

- https://learn.microsoft.com/en-us/windows/arm/arm64ec-abi#fast-forward-sequences
- https://clang.llvm.org/docs/AttributeReference.html#hybrid-patchable
