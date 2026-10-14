# BCD performance comparison, 14 October 2026

## Exact comparison

Read-only BCD checkouts: stable `v0.1.10-bcd` at
`26f1edd7d816683e3bd4f56fe30161caa307d38f`, test `v0.1.11-pr1-test` at
`df19c3c3f460e30a2d3def44c4c2cc0e953d12c8`. Both pin FEX `3bec2ac`,
Wine `257f271`, DXMT `db546ee`. The tester's actual BCD IPA/config remains unknown.
Our deployed build 666 consumes native inputs built from source 663; its pins
are FEX `26859e1`, Wine `4f5b197`, DXMT `35a4db1`, with parent integration patches.
Checking source alone does not prove which code a downloaded IPA contains.

The published IPAs were also downloaded without executing them and their
GitHub asset SHA-256 digests verified:

- BCD 0.1.10: `17aaeaa211925e68dc8c5934d86b6da65ab1e79104e62e6689fd2427e9c5c403`.
- BCD 0.1.11 test: `cd5ab4135ec9f4b9f7daac533d38cac5c5966f993bedf3320c7c1ed61f9c3af6`.
- Madeira 666: `43b2f2a10c04bd93b5163689ed699d87289647acf202a936943c3bec75bc2bf2`.

Both BCD IPAs contain `commitBeforePresent=` in ARM64EC d3d11.dll,
`ml1178` in the linked native app and `ml1231`/`LRCPC2=` in xtajit64.dll.
Our published 666 contains none of those markers. Thus these are actual
packaged differences, although markers do not prove their runtime setting or
the size of any FPS gain. BCD 0.1.10 has no `winios-gl` native app marker;
the 0.1.11 test does. Full binary hashes/markers are in the local evidence
`E:/CodexTemp/madeira-bcd-performance-20261014/ipa-comparison.json`.

## Findings and decisions

| Area | BCD implementation | Our status and action |
| --- | --- | --- |
| D3D11 frame submission | DXMT `4443a2e` commits a nonempty draw chunk before recording present, allowing rendering during drawable wait. Costs an extra command buffer per frame. | Missing in 666. Imported as an ordered parent patch; `d3d11.commitBeforePresent=0` restores old ordering. ARM64EC swapchain compiles in an isolated patched checkout. Device gains unmeasured. |
| CPU-written Metal memory | DXMT `4f025e0` uses cached CPU mappings for buffers/textures/heap allocations and alignment queries; avoids expensive release stores into write-combined pages. | Missing in 666. Adapted for local iOS only with atomic publication of the setting. Remote/macOS paths retain their flags. `MADEIRA_WRITE_COMBINED=1` restores the original cache mode. Concurrent production helper tests preserve all storage and hazard flags. |
| Translated CPU code | BCD FEX enables LRCPC2 only with a positive host probe and defaults AFP off for ARM64EC, reducing address arithmetic and FPCR transitions. | Absent from our pinned ARM64EC implementation. Its Windows iOS feature path also bypasses generic host-feature overrides, so simply exporting `FEX_HOSTFEATURES` cannot implement this. FEX core was not edited: its AGENTS.md prohibits AI-generated contributions. A compatible reviewed upstream translator upgrade needs separate integration, matching invalidation/loader behavior and rebuilt binaries. |
| Windows clocks | Wine `GetTickCount/64` and `QueryInterruptTime` avoid reads of inaccessible KUSER_SHARED_DATA addresses. | Direct native tick clocks already rebuilt in 666, with binary marker gates. This is not a new toggle to enable for the current log. |
| Synchronization | Default Fastsync auto, semaphore fast path opt-in. Madsync is another engine. | Same default engine in our tree; current REPLACED and Primal logs confirm auto. Changing the label/default alone will not explain the reported gap. |
| Refresh and caps | Default 60 cap; maximum-panel display link is opt-in for capped mode. 40 FPS is a cap on suitable panels, not 60 FPS performance. | Our source preserves user 60 instead of automatically forcing 30 under serious heat. 666 still caps at 30 in the latest hot REPLACED capture. No claim that heat explains every slow capture. |
| Memory/upload work | BCD adds bounded staging renaming, upload flushes at 64 MB, shader-program release and asynchronous BC mip refill. | Candidates require separate memory/texture correctness review; BC refill is specific to unsupported BC formats/mip clamping. They are not evidence for the Godot OpenGL FPS gap. |
| Shader cache | Both paths have caches; BCD test also adds D3D12 shader packs. | Madeira already persists DXMT pipeline cache and learned warmup. D3D12 pack changes do not directly accelerate DX11 REPLACED or the observed Primal OpenGL path. |
| x64 image copy | BCD no-copy option defaults off; pure x64 mapping and invalidation change together. | Not enabled globally. It cannot be assumed responsible for BCD's speed without the tester's config. |
| D3D12 method entries/frame generation | Patchable entries and frame generation are opt-in and change specific paths. | Not imported as a general FPS fix. Generated/displayed frames cannot be treated as increased game simulation FPS. |
| Frontend effects | BCD has no matching private-shelf/scenic Metal lifecycle additions. | Source 667 stops hidden library Metal draws; simulator recorded zero hidden-library Metal frames. This does not by itself prove the cause of the original game slowdown. |

Primary changes: [submit before present](https://github.com/willfaust/dxmt/commit/4443a2eafa6bc624ac5c60e89a6fd1e13d881624),
[cached CPU mappings](https://github.com/willfaust/dxmt/commit/4f025e0afa25a8845b2db16276db3305d4c86088),
[BCD CPU pins](https://github.com/llamajose/madeira-bcd/commit/662604e65612df219aed5180df44a89060f31565).
Upstream performance figures are their tests of other games; they are not measurements of our build.

## Logs and renderer scope

- REPLACED `b80dcc91`: 1920x1080, selected/applied mode 1, thermal 0 throughout;
  FPS falls from about 59 to 27.5 as memory approaches 6.3 GB. Background GPU
  permission errors occur later and cannot explain preceding FPS drops.
- REPLACED `363fad4b`: actual drawable 1408x648; about 46–48 FPS at thermal 0,
  then thermal 2 at 19:04:19 switches deployed 666 to mode 3, about 26–28 FPS.
  Lower resolution did not establish that CPU alone is responsible.
- Primal Planet `b6be0ad9` build 649: Godot 4.4.1, desktop OpenGL 4.1 through
  Mesa 25.0.7 Zink/MoltenVK 1.4.2; gameplay about 20–32 FPS, thermal 0,
  mode 1, around 2.5 GB. This supports the tester's slow-performance report.
  The D3D11 submission patch cannot directly improve that observed renderer.
- BCD stable lacks the test's WiniosGL/OpenGL driver files. BCD test has the
  same basic OSMesa/glFinish-to-IOSurface presentation design and defaults to
  Zink when bundled. No read-only source comparison proves which renderer the
  tester actually used.

Godot 4 Windows supports `opengl3_angle` using its own EGL loader. The existing
SDL hint does not select that renderer. Madeira now offers an explicit comparison:
choose ANGLE in the game's existing OpenGL renderer setting. For detected Godot
games without explicit renderer arguments, launch adds
`--rendering-method gl_compatibility --rendering-driver opengl3_angle`.
Automatic mode stays on the existing backend, and explicit arguments win.
This is unmeasured on device and may be unsupported by older/custom exports.
Source: [Godot 4.4 Windows display server](https://github.com/godotengine/godot/blob/4.4-stable/platform/windows/display_server_windows.cpp).

## Verification and deployment

Parent DXMT patches apply to LF-only pinned sources, repeat safely and coexist
with runtime lifecycle, shader-cache/warmup and foreground GPU-gate patches.
The production CPU-cache helper passes concurrent Windows C fixtures. The
patched ARM64EC D3D11 swapchain passes the real compiler with shader payload
placeholders; this is a compile check, not a final linked DLL or GPU test.
Mac Swift policy/input/privacy checks run in the simulator verification workflow.

Bitrise verification #53 succeeded on source `489d75f` (source count 670):
production Globe gap/save-stall/focus fixtures, FPS policy, private library/PIN
fixtures and Godot argument/detection fixtures pass. The iOS Objective-C GPU
adapter/Swift bridge compile; visible native search renders Metal and the
hidden library submits zero frames. Evidence is stored locally at
`E:/CodexTemp/madeira-ui-preview-670/ui-preview.log` and `search-checks.json`.
This workflow does not link the release Wine/DXMT app or run a Windows game.

Future native builds must rebuild D3D11 plus the Metal archive. The compatibility
checker rejects cached bundles lacking present-order and cache-mapping markers.
The published 666 IPA has not been replaced by these source changes. A source
commit or successful simulator workflow is not a published IPA or a proven FPS gain.

Remaining device evidence needed: same scene and resolution, actual renderer,
requested/applied cap, thermal state, native GPU completion timing, exact BCD
version/config. The latest Party Animals failure is separate: FEX's 16 GB host
VA band exhausts, then a host BRK is reported as Windows 0xC000001D. It is not
fixed by these FPS patches. Celeste/Fading/touch-only Primal build 666 logs are
not on the server at this audit's latest refresh.
