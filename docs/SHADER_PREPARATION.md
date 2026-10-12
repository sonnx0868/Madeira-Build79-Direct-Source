# Preparing shaders and pipelines for Silksong

## What this implementation does

The ordinary DXMT render and compute thunks record the translated Metal library,
function name, scalar specialization constants, and all descriptor fields those
thunks set. On the next process launch, a background worker reconstructs up to 64
observed pipelines against the existing immutable Metal binary archive. Exact
matching PSOs are returned directly when the game requests them. The render
thread never waits for replay and no draw is dropped while replay is running.

This is learned preparation. It does not enumerate every Unity shader variant
from Silksong's assets or make an offline shader pack for an unplayed game.
The first session still translates/compiles new shaders. Existing DXMT disk
shader caching supplies translated libraries on later sessions.

Automatic is enabled for the recognized Silksong profile while its Unity
optimizations are enabled. The per-game **Prepare shaders on launch** picker has
Automatic, On, and Off. `env.MADEIRA_SHADER_WARMUP = 0/1` supplies the global
automatic choice. Explicit per-game choices win. Restart Madeira after changing
the setting because device cache objects live for the process lifetime.

## Boundaries and fallbacks

- Per-game cache scope survives app updates; executable size/mtime changes start
  a new scope. Library bytes have SHA256 identities and are checked on replay.
  A missing per-game binary archive can read the previous shared GPU/OS archive
  as a seed, so moving to game scopes doesn't deliberately discard existing PSOs.
- Shader library files: at most 2 MiB each, 256 identities, 32 MiB per inventory.
- Descriptor journal: at most 128 recipes, 512 KiB; at most 64 replay attempts.
- Replay checks a 1.5 second budget between attempts and stops on serious thermal
  pressure. A single Metal operation can exceed this budget; it is not a hard
  deadline. Reconstructed libraries are released after replay; at most 64 PSOs
  wait for consumption. Pipeline cache device/scope objects are capped at eight.
- Replay requests `MTLPipelineOptionFailOnBinaryArchiveMiss`: missing binary
  entries are skipped. Ordinary game requests retain the existing compilation
  fallback. Unsupported constants, custom vertex layouts, patch shaders, mesh
  pipelines and private logic operations are not approximated for replay.
- Only known ordinary DXMT descriptors are marked for warm reuse. Options,
  reflection requests, and a caller's explicit archive policy are preserved.
- Mesh/tessellation and remote Metal paths retain their existing behavior.
- The disk archive still flushes on its existing 10-second background schedule.
  A very short first session may save a recipe without a matching binary yet;
  the next launch skips that entry and continues normally.
- DXBC/AIR shader cache identity remains managed by the existing compiler-content
  stamp. Metal archives are namespaced by GPU and OS and validate library/state
  identities. No downloaded pipeline binary is trusted or installed here.

## Measuring on the device

1. Use a bootstrap-built IPA containing `learned-pipeline-replay-v1`.
   Build609/612's native bundle does not contain this feature; the cached build
   now rejects that bundle rather than shipping inactive warmup UI.
2. Start Silksong, enter the same area and play at least 30 seconds so the binary
   archive has time to save. Keep resolution and CPU settings unchanged.
3. Fully restart Madeira, launch the same game, repeat the same path, and upload
   the session log. Repeat with **Prepare shaders on launch: Off**, restarting.
4. Compare first-frame latency, Present/frame gaps and maximum native pipeline
   creation duration, not just average FPS. Look for:
   - `[shader-profile] replay=1 inventory=game-v1`
   - `[shader-warmup] learned-pipeline-replay-v1 ... ready=... skipped=...`
   - `[shader-warmup] pipeline-hits=...`
   - `[pipeline-cache] native-creations=... avg-ms=... max-ms=...`

The first three prove activation/reuse, not a frame-time improvement. Mono JIT,
asset I/O, garbage collection, CPU translation and thermal pressure can still
cause pauses even when pipeline warmup succeeds.

## CPU work in this change

Verified CPU code caching now keys its app-side filename by the bundled
`xtajit64.dll` SHA256 rather than `CFBundleVersion`. App-only updates therefore
keep compatible learned code. Unknown translator identity falls back to the
app build. The existing backend independently verifies compiler identity,
host features, translation settings, IR and ARM64EC target classifications.
No FEX core/translator code is modified by this change.

`tools/cpu-aot-survey.py` is a read-only research tool:

```powershell
python tools/cpu-aot-survey.py "D:/Games/Silksong/Silksong.exe" --output silksong-aot.json
python tools/cpu-aot-survey.py "D:/Games/Silksong/UnityPlayer.dll" --output unity-aot.json
```

It reports PE identity, executable/writable sections, AMD64 unwind function
ranges, imports, ASLR, TLS/CLR and nearby engine hints. It does **not** emit ARM64
code. Unwind metadata is incomplete and is not a hot-function profile.

The GTA wasm2c route starts with a structured, fixed Wasm engine; an ordinary PE
has indirect branches, SEH, DLL bindings and potentially generated code. For
Silksong, native Unity/Mono modules must be examined separately from the small
launcher, and Mono's generated code remains a separate problem. A credible next
experiment needs real module files, a device execution profile and relocation/
exception handling validation before any AOT code is put on the gameplay path.
`FEX/AGENTS.md` forbids AI-generated contributions to FEX; this work stays in
Madeira's host integration and independent research tools.

## Verification

Windows-runnable checks: ordered patch pipeline/idempotence, source contracts,
malformed PE/candidate classification, and survey of the real bundled x64 lab.
macOS checks: `check-pipeline-cache.py`, `check-shader-warmup.py` (native production
code with protocol doubles under ASan/UBSan), and `check-translation-tools.py`
(Swift profile/reset/hash fixtures). The Bitrise script runs all three gates.
Native fixture success is distinct from a real Metal GPU benchmark.

At implementation time, macOS/iOS compilation and iPad measurements are pending.
The browser control service needed to trigger Bitrise is missing from the current
Codex plugin installation. Do not report this source change as a built/published
IPA or claim a measured Silksong speedup until those steps succeed.
