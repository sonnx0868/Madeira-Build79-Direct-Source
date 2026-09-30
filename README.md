# Madeira

Run Windows PC games on a non-jailbroken iPhone.

Madeira combines [Wine](https://www.winehq.org/) (ARM64EC),
[FEX-Emu](https://github.com/FEX-Emu/FEX) for x86-64 → ARM64 translation, and
[DXMT](https://github.com/3Shain/DXMT) for D3D11 → Metal, running as a single
Mach process on iOS with wineserver as a thread rather than a separate process.

## Status

Thumper and ULTRAKILL are playable. Marvel Cosmic Invasion has reached
gameplay, though a run has also ended in an unexplained termination and its
controls are not yet reliable. Others reach gameplay at low frame rates. This
is a research project, not a product: expect rough edges, per-title quirks and
breaking changes.

The iOS target currently supports **64-bit Windows PE** executables (x86-64
through FEX/ARM64EC, plus ARM64 PE). It does not ship a working 32-bit x86
address space, and its accelerated graphics path is D3D11 through DXMT. Plain
OpenGL, D3D12/Vulkan, anti-cheat and DRM-heavy programs are not universal
compatibility targets; an `.exe` suffix alone does not imply that a program can
run.

## Using the game library

1. Enable JIT with StikDebug from Madeira's setup card.
2. Choose **Add → game folder**. A complete folder is strongly preferred over
   one `.exe`, especially for Ren'Py (which also needs `game/`, `renpy/`,
   Python/SDL DLLs and assets).
3. Madeira copies the game into `C:\Games`, reads the PE architecture, detects
   common Ren'Py layouts and chooses a 64-bit executable. If a folder contains
   several executables, use **Edit** to select the correct one and adjust quoted
   command-line arguments.
4. Press Play. One Wine/FEX session is supported per app process; relaunch
   Madeira before switching games so the process-lifetime JIT pool is not
   duplicated.

On iPad, a USB/Bluetooth hardware keyboard and mouse/trackpad are detected
automatically while a game is running. Madeira forwards held keys, modifiers,
arrows, F1–F12, left/right/middle clicks, relative movement and scrolling to
Wine. iPadOS-reserved shortcuts such as Command-H remain system shortcuts.

## Requirements

- A non-jailbroken iPhone. Development has been on an A15 (iPhone 13 Pro).
- JIT, which on iOS requires a debugger to attach —
  [StikDebug](https://github.com/0-Blu/StikJIT) is what this project uses.
- An Apple ID for signing. A free account works; its provisioning profiles
  expire after 7 days, so the app must be rebuilt and reinstalled weekly. The
  app's container survives reinstall, so prefixes and saves are preserved.

Because JIT requires debugger attach, this app cannot be distributed through the
App Store. It is installed by sideloading.

## Building

The build is split across several chains — the unix-side Wine libraries, the
ARM64EC PE modules, FEX, DXMT and the iOS app itself. `build/*/build.sh` covers
the native pieces; the app is built with `xcodebuild`.

```sh
git clone --recurse-submodules <this repo>
```

Note that `FEX`, `wine` and `research/dxmt` are submodules pointing at forks
containing the iOS work; upstream clones will not build here.

The root repository pins the exact public component revisions recorded in
`Build79/PROVENANCE.json`. A small set of release-materialized changes is kept
as auditable root patches instead of pointing the submodules at unpushed local
commits. After cloning, run:

```sh
git submodule update --init --recursive
bash scripts/apply-lineage-patches.sh
```

This source snapshot does not contain every generated/static or redistributable
binary. On macOS, run the read-only preflight first:

```sh
bash scripts/check-ios-build.sh
```

It reports missing FEX/Wine/DXMT archives, PE resource trees and Microsoft VC++
runtime files before Xcode reaches an opaque linker error. Follow the relevant
`build/*/build.sh` stages and `tools/fetch-vcruntime.md`, then select your own
Apple Development team in Xcode. Madeira requires development signing and JIT;
it is not an App Store/distribution-signing build.

## License

**GPL-3.0-or-later** — see [`LICENSE`](LICENSE). Derivatives that are
distributed must remain open source.

### Upstream licenses vs. this project's forks

Those are the licenses of the **upstream projects**: Wine and GnuTLS
LGPL-2.1-or-later, GMP and Nettle LGPL-3.0-or-later, FEX-Emu and DXMT MIT,
rpmalloc 0BSD. Their texts are in [`LICENSES/`](LICENSES), and upstream code
remains available under them **from upstream**.

**The forks used here are not licensed identically to their upstreams.** Each
carries its own `LICENSE-MADEIRA.md` saying exactly what applies:

| Fork | Terms |
|---|---|
| [`wine`](https://github.com/willfaust/wine) | relicensed to **GPL-3.0-or-later** under LGPL-2.1 §3 |
| [`FEX`](https://github.com/willfaust/FEX), [`dxmt`](https://github.com/willfaust/dxmt) | upstream MIT preserved; modifications **GPL-3.0-or-later** |
| [`rpmalloc`](https://github.com/willfaust/rpmalloc) | upstream 0BSD preserved; Will Faust's modifications **GPL-3.0-or-later** |

This is not retroactive: those forks were public beforehand, so anything
already obtained under a permissive license stays available under it.

[`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md) has the per-component
breakdown. Note in particular that the Microsoft Visual C++ runtime DLLs are
not distributed here and must be supplied yourself — see
[`tools/fetch-vcruntime.md`](tools/fetch-vcruntime.md).

## A note on upstream contributions

The forks here contain substantial AI-assisted work. FEX-Emu's contribution
policy states that AI must not be used to generate code for contributions to
that project, so **do not submit AI-generated changes from this fork upstream**.
The MIT license permits the fork itself; the policy governs contributions back.
Check each upstream's contribution policy before proposing changes to it.
