# Madeira Build 79 — lineage-verified source

This tree was reconstructed only from the two Build 79 artifacts preserved in
the same release workspace:

- `Madeira.ipa` — SHA-256
  `5c2a8d7aff37bfc6a0bd7e80ae52cfef4744b88c717ef77e6284dc36c77b41b9`
- `Madeira-Build-79-Source.tar.gz` — SHA-256
  `1779d8657e67ec7d5316057969fa985333555686fd3511ac10a2309793afed3c`

No file was imported from another Madeira checkout, dated workspace, fork, or
community source package. The component snapshots materialized from the Build
79 source archive are:

| Component | Pinned revision |
| --- | --- |
| Madeira | `97e2ce26e6dc9e4a38976f3b5deb9272d64558eb` |
| FEX | `053c385ecc9090702e4959a1d96752ea918a6110` |
| Wine | `7817e220384e895651f868ba4d97affcf21b3816` |
| DXMT baseline | `b4b89f0a5a1752da3982a7b6c5575506024bf253` |

`research/dxmt` is the complete DX11-optimization source embedded in Build 79,
not a DXMT tree taken from another checkout. `Build79/ExactEmbeddedSource`
preserves the release's own source and historical build/repack scripts exactly.
Those fixed-offset scripts are evidence of how the published IPA was assembled;
they are not a portable source-build API.

## Source changes materialized in this tree

The following Build 79 lineage changes have been applied to their source files:

- final ntdll TLS/module-loader source from Build 78;
- FEX CPUID metadata indexing and runtime TEB lookup fixes;
- Wine iOS native-TSD bitmap correction;
- abrupt guest-exit host-survival correction;
- fd-cache exit, async cancellation, APC signal/context, decommit-edge, and
  final SRW orphan fixes;
- FEX Windows `VirtualProtect` return/`OldProtect` contract correction;
- the complete Build 76 DX11 optimization source used by Builds 76–79;
- Build 79's `Madeira`, `0.34.7`, build `79`, and extended-gamepad Info.plist
  metadata;
- Build 79's Metal 3.1 / iOS 17 shader source and compatibility artifact are
  retained under `Build79/ExactEmbeddedSource/ShaderCompatibility`.

The final UI, resolution, audio, touch, and controller implementation is under
`Build79/SourceOverlay`. The untouched copy used by the release is under
`Build79/ExactEmbeddedSource/ResolutionSource`.

## Important build boundary

This is the corrected corresponding-source tree, not a claim that the original
Build 79 IPA can be reproduced bit-for-bit on Linux. The app and native runtime
require Apple's iOS SDK and Xcode/macOS. The release's historical Python scripts
start from checkpoint IPAs and contain exact executable offsets; do not use
those scripts on a newly linked executable. Build source files through Xcode and
the existing `build/*/build.sh` chains instead.

The fixed-offset scripts are intentionally kept only in
`Build79/ExactEmbeddedSource` so the original release remains auditable. They
must not be copied into a new build pipeline as if their addresses were stable.

## Verification

`SHA256SUMS` covers the original lineage-reconstructed source bundle except
itself. It is intentionally retained as release evidence; it does **not** cover
the launcher, input, CI, and runtime-safety changes made after reconstruction.
Use Git commit hashes for the current development tree and do not regenerate
this historical manifest in place.
`Build79/PROVENANCE.json` records the authoritative inputs, pinned component
revisions, and applied source patches. Run:

```sh
sha256sum -c SHA256SUMS
```

against an untouched Build 79 source archive when auditing that historical
artifact. See `CHANGES-SINCE-BUILD79.md` for the current development delta.
