# Applied Build 79 lineage patches

The source tree is materialized rather than a chain of IPA mutations.

| Order | Source target | Direct-lineage source |
| --- | --- | --- |
| 1 | `wine/dlls/ntdll/loader.c` | Build 79 `RuntimeFixes/ntdll/loader.c` |
| 2 | `FEX/FEXCore/Source/Interface/Core/CPUID.{cpp,h}` | R5 patch 02 |
| 3 | `FEX/Source/Windows/ARM64EC/Module.S` | R5 patch 05 runtime TEB |
| 4 | `build/ntdll-unix/virtual_ios.c` | R5 patches 06 and 12 |
| 5 | `build/ntdll-unix/process_ios.c` | R5 patch 07 semantic equivalent |
| 6 | `build/ntdll-unix/server_ios.c` | R5 patch 08 |
| 7 | `wine/server/async.c` | R5 patch 09 |
| 8 | `build/wineserver/mach_ios.c` | R5 patch 10 |
| 9 | `build/ntdll-unix/signal_arm64_ios.c` | R5 patch 11 |
| 10 | `wine/dlls/ntdll/unix/sync.c` | R5 final patch 13 |
| 11 | `FEX/FEXCore/include/FEXCore/Utils/AllocatorHooks.h` | Build 79 protection-contract patch |
| 12 | `research/dxmt` | Build 79 embedded DX11 optimization source |
| 13 | `app/Madeira/Info.plist` | Build 79 release identity and extended-gamepad metadata |

Patch 05's earlier SRW change and patch 13 are byte-identical. The final patch
was applied exactly once to avoid a duplicate/conflicting hunk.

The release's fixed-address UI patchers were not run against this newly linked
source tree. Their portable C/Objective-C/Swift source is preserved under
`Build79/SourceOverlay`; the original scripts remain under
`Build79/ExactEmbeddedSource` for audit only.
