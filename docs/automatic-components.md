# Automatic game components

Adding an EXE schedules preparation. Play waits for the same preparation task
before requesting JIT; downloaded games are checked at Play too. Concurrent
requests share work, with serialized installation transactions. Preparation
does not execute a Windows installer or consume a Wine session.

## Resolution and installation

The bounded PE scan includes imports, delay imports and matching-architecture
DLLs/plugins. It resolves each import against its module folder, EXE folder,
prefix system directory and the correct bundled Wine architecture. A recursive
filename match elsewhere in a game does not satisfy a dependency. Wrong local
architectures are reported and preserved.

Only missing components in the bundled pinned catalogue are downloaded:
Microsoft VC++ 2005/2008/2010/2012/2013/14, x86 and x64, and the June 2010
DirectX helper libraries. Existing bundled Wine implementations are reused.
Managed CLR games use the already verified, bundled Wine Mono 11.0.0 via the
existing offline prefix placement. A separate modern .NET Desktop requirement
is reported; it is not silently treated as satisfied by Mono.

Packages use immutable official Microsoft URLs, HTTPS with a redirect host
allowlist, exact download sizes and SHA-256 pins. All package signatures were
verified as Microsoft Corporation when generating this catalogue. Runtime DLLs
and VC80/90 assembly manifests also have individual uncompressed SHA-256 pins.
The app extracts matching files without running setup or processing MSI custom
actions. The VC80 embedded contiguous CABs are selected only for the pinned
packages audited by tools/runtime-catalog.py. The entire uncompressed output
is verified, including machine type; CAB member names never become paths.

Native Wine/core graphics DLLs, API-set DLLs and game-owned libraries are not
downloaded from arbitrary DLL websites. Missing game files and unsupported
components remain visible as findings, and do not prevent trying the game.
Network/integrity/install failures stop preparation with a retryable error.

Installation is app-local and absent-only. Required dependencies inside the
same runtime package are installed together; post-install scanning detects any
remaining known gap. On failure, only newly created matching-hash files are
rolled back. Existing game files are never replaced. A runtime may still need
an API or registration that Wine does not implement; file readiness is not a
promise that every Windows game will run.

## Cache

Documents/RuntimeCache/packages stores verified, shared extracted payloads;
the large installer is discarded after extraction. Documents/RuntimeCache/games
stores per-executable receipts, using hashed paths as filenames. Cache checks
use file/directory size, inode and modification time, catalogue hash, bundle
location, plus hashes of files installed by Madeira. Changing game files or
adding/removing a DLL invalidates the receipt. App reinstall/resource location
changes invalidate it too. A valid receipt skips PE rescanning and downloading.
Game details offers a forced recheck. A cached warning is invalidated by changes
in the watched directories, so unsupported dependencies do not force a full
scan on every launch. Truncated scans are never considered reusable complete
checks.

Receipts contain local paths for invalidation; they stay on the device and are
not included in diagnostics. Diagnostic lines contain only counts/cache state.

## Verification

tests/host/check-runtime-preparation.py compiles the production CAB adapter and
production Swift core. Synthetic PE/CAB fixtures test archive corruption,
traversal containment, extraction bounds, existing-file preservation, transitive
runtime installation across families, concurrent request sharing, post-check, cache reuse, deletion repair, wrong-machine
preservation and directory invalidation. No fixture EXE is executed. Mac CI
requires the Swift portion; the native portion also runs on Windows.

Maintainers can regenerate the catalogue using tools/runtime-catalog.py with
the reviewed package metadata, packages and a host build of the CAB decoder.
The libmspack LGPL sources and notices are included in the source and app.
