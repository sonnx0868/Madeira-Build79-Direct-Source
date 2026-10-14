# Tester logs: Celeste and Fading Afternoon

Both supplied reports identify source 075b6a23, build 666. They do not
validate the newly published build 679. No device gameplay was reproduced
on this Windows host.

## Celeste (Epic)

- PE machine 0x14c; Wine image flags 0x6 = ILOnly + dynamically relocated,
  without ComPlusNativeReady. Wine's mapping parser sets NativeReady for
  PE32 ILOnly only when the CLR header does not require 32 bits.
- The log explicitly says the bundle has no i386-windows. The bridge
  consequently launches the ARM64EC farm with no 32-bit guest window.
- build_wow64_parameters tries to allocate 0x3876 bytes below 2 GB. iOS
  refuses the host mapping (STATUS_NO_MEMORY/c0000017). The next line is
  assert(!status) in env_ios.c, ending the shared application process.
- This is a missing 32-bit runtime path, before graphics initialization;
  resolution and shader caching cannot fix this launch.

Source follow-up: classify native x86 and CLR 32BITREQUIRED before direct
launch; reject an incomplete WoW64 bundle with an actionable error. Keep
AnyCPU and 32BITPREFERRED-only assemblies eligible for the native CLR.
Refresh Epic entry architecture/graphics/engine from its installed EXE
instead of retaining bits=0. This prevents this direct-launch app abort;
it does **not** supply the missing Wine/FEX 32-bit runtime or prove Celeste
playable. Child processes started by a separate launcher still require
native loader protection and the complete runtime.

## Fading Afternoon (v2024.01.11)

The report loads a different game package from the user's Vietnamese
local installation. Its first fatal fault is d3d11.dll+0x2f98c, not the
Steamworks achievement function from the older local report. It receives
Steam stats and creates its game window before the fatal fault.

The verified build 666 native dependency archive provides the matching
d3d11.dll. llvm-objdump identifies RVA 0x2f98c as
dxmt::MTLD3D11SwapChain<false>::Present:

```
original: aa1f03e3 140001d7    // ARM64: mov x3,xzr; branch Present1
at fault: ef17e7e9 140001be    // first five bytes overwritten
```

Fault bytes `e9 e7 17 ef be` form an x64 relative-jump detour. The ARM64EC
entry thunk calls x9=0x14a8fe98c (the patched Present), with its return
address at d3d11.dll+0x1cb50c. ARM64 executes the injected x64 bytes and
raises C000001D. This establishes incompatible modification of an ARM64EC
entry point; it does not identify which library installed the detour.

An x64 overlay/hook is a likely source, requiring confirmation from the
tester's actual steam_api64.dll and other injected modules. Disabling
Valve's gameoverlayrenderer DLL alone would not suppress an overlay
implemented inside a different local DLL.

Older Goldberg source supports steam_settings/disable_overlay.txt:
https://github.com/kameralarda/goldberg_emulator/blob/main/dll/settings_parser.cpp
Newer forks changed this configuration. Do not create/rewrite this file
blindly for every game or claim that an unidentified DLL obeys it. The
local user's active steam_api64.dll contains configs.overlay.ini and
enable_experimental_overlay, but no disable_overlay.txt marker, so it
cannot stand in for the tester's different binary.

Next evidence: tester's Celeste.exe and Fading steam_api64.dll, plus a
build 679 session to separate previously fixed paths from these remaining
compatibility gaps. No additional game binary was patched from these logs.
