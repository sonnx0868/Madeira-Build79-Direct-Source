# Sundered and Celeste: build 701 reports

## Reports read in full

- `1d428122-d6c8-4ea8-9113-3ba4335c4192`: Sundered, local installation,
  iPad14,6, build 701, 511,422 bytes, not truncated. This is a different
  package/launch route from the tester's Epic build-697 report.
- `56432743-a224-42f6-a76b-e49aeb2d0cea`: Celeste 1.4.0.0 Vietnamese local
  installation, iPad14,6, build 701, 150,695 bytes, not truncated.

### Celeste

The WoW64 window and i386 runtime now initialize. `mscoree` reads the request
for `.NETFramework,Version=v4.5`, then explicitly reports
`CLRRuntimeInfo_GetRuntimeHost Wine Mono is not installed`. Exit is
`0xffffffff`, before presentation. This supersedes the old build-666 missing
i386-farm failure; the i386 farm alone is not a managed runtime.

The pinned Wine source expects Wine Mono **11.0.0**. Its unmodified official
runtime is staged at build time from the release tarball, SHA-256
`0cd723aa28897f7d7d2702eed6c72e4262255980e212bc2b3c94bfa234abe5fd`.
The 2,923 files include both x86 and x64 Mono DLLs and the managed class
libraries. The build verifies the archive, the expected Wine version, file
hashes and i386 immediate imports against the existing DLL farm.

At prefix setup, `C:\windows\mono\mono-2.0` links to the bundled runtime,
which the existing `mscoree::get_mono_path_local` already understands. App
reinstall refreshes only the app's old link. A physical/user-linked runtime
is preserved. No game EXE flags or game DLLs are changed.

All copyright/license files were copied from the verified complete source
archive into `legal/THIRD-PARTY-NOTICES-WineMono.txt`; it includes the exact
binary and corresponding-source URLs and SHA-256 values.

A Windows-native x86 fixture loaded this exact Mono DLL, initialized the CLR,
opened a newly compiled 32BITREQUIRED C# EXE, printed
`MADEIRA_MONO_MANAGED32_OK`, and returned the expected managed exit value 42
(harness success 0). This verifies the package's real CLR execution on
Windows, not Wine/FEX/iPad execution or Celeste gameplay. The isolated local
fixture is in `E:/CodexTemp/madeira-mono-smoke-20261015`.

### Sundered

This run starts through the native supervisor: child pid 0028. Its Unity
player is embedded in the EXE (`player_win.exe` export identity); Mono is in
`Sundered_Data/Mono`. The preceding detector only recognized a top-level
UnityPlayer.dll/Mono directory. Consequently it missed the existing policy
that keeps Unity/Mono out of the unvalidated reusable-child route.

Unlike the earlier Epic report, there are **zero presented frames** here.
The x64 unwind reaches `player_win.exe+0xa10916` / `+0x9849d3`, then
ExitProcess(1). The log also records the existing session-thread/child-window
dispatcher warning. A Mono backpatch access violation is handled earlier;
it does not establish an unhandled native crash. A failed native probe of
`mscorlib.dll.dll` is not sufficient proof that `mscorlib.dll` is missing.
No Unity companion log is present in this report.

The detector now recognizes the selected EXE's `_Data` layout, requiring a
UnityEngine assembly plus Mono or Unity player-data markers. This activates
the existing isolated launch route without requiring a global Reusable
toggle change. CLR EXEs likewise select an isolated launch, avoiding the
child-window errors seen before Celeste's Mono lookup. Ordinary native games
retain the prior route. These are concrete routing/dependency corrections;
device gameplay and the exact Unity-level exit reason remain unverified.
