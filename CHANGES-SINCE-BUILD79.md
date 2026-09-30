# Changes since the lineage-verified Build 79 snapshot

The original `SHA256SUMS` and `Build79/PROVENANCE.json` remain immutable
release evidence. This development branch adds:

- a SwiftUI game library and Files-based folder/EXE importer;
- PE architecture validation and selectable executable profiles;
- Ren'Py detection with ANGLE2/software renderer defaults;
- quoted launch arguments, per-title working directory and Steam identity;
- guided, bounded StikDebug/JIT startup and explicit launch states;
- physical iPad keyboard, mouse and trackpad input through GameController;
- input queue coalescing that preserves key/button release edges;
- Wine/wineserver atomic lifecycle, socket rollback and owned TLS cleanup;
- portable Xcode references, Codemagic workflows and native-dependency
  packaging/verification scripts.

The materialized FEX, Wine and DXMT deltas are preserved in `patches/` and are
applied to the exact public commits with `scripts/apply-lineage-patches.sh`.
