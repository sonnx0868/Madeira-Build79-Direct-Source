# Reusable game runtime

Quit ends the game and returns to Madeira's home/library. For direct executable
launches, the opt-in mode keeps **one Wine/JIT bootstrap** alive for the app run
and starts each game as a child of the bundled `madeira-session-host.exe`.
No Steam Library, Dock or desktop shell is required for folder games.

This is source implementation, not a device-compatibility claim. The Windows
smoke test launches only synthetic fixtures. iPad validation is still required.

Build570 device A/B: Silksong crashed while unwinding Mono's Finalizer naming
exception under the x64 supervisor, but entered gameplay with reuse disabled.
The supervisor is now **ARM64 native**, staged in `aarch64-windows`, and its PE
machine is checked before bootstrap. x64 games therefore use the existing
fresh cross-arch ARM64EC ntdll loader, not a clone of a live x64 supervisor's
ntdll. An old prefix symlink or `MADEIRA_USE_ARM64EC` must not force the internal
supervisor back onto the x64 path. This addresses the observed route regression;
the actual iPad exception/quit/second-game run remains an acceptance test.
Reuse is off by default until device validation; explicitly enable it in
Settings to test the new native-supervisor route. Existing explicit choices
are preserved. Disabling it is a safety fallback, not the runtime fix.

## Lifecycle

1. Cold Play prepares a private `drive_c/madeira-runtime/<random UUID>` channel,
   bootstraps Wine once with the headless session host and publishes a bounded
   binary launch request. The host uses `CreateProcessW` with an explicit EXE,
   command line, working directory and Unicode environment; it never uses a
   shell. Child creation is suspended until assignment to its Windows job.
2. The host monitors the job's entire process tree. A launcher EXE ending while
   its child game still runs is not treated as game exit. Each request carries a
   monotonic generation; an old Quit cannot close a later game. Separate start
   and control files prevent early Quit from overwriting an unpublished launch.
3. Quit posts WM_CLOSE to that job's windows; after the UI's grace period its
   force request terminates the job on the parent Wine thread. An active/failed
   accounting query is not interpreted as an empty job. Unsaved progress may be lost.
4. A confirmed empty job returns the UI home, but the runtime is initially
   **draining**, not ready for another game. Native lifecycle hooks reserve Wine
   threads before creation and retain their Mach port rights after attachment.
   Read-only `thread_info` checks must show all owned native threads really dead;
   a query failure, suspended thread, pending creation or Windows termination
   notification is not sufficient. GPU command-buffer completion is also tracked,
   independently of optional frame diagnostics.
5. Child exit allows up to three seconds for native peers to drain before
   existing JIT/image/window reclamation (formerly only 250 ms).
   If peers remain, mappings are retained and the engine is quarantined. The app
   does not unmap code under those peers or fake a successful cleanup. Only
   retired, quiescent generations can be reused for the next Play.
   Admission is rechecked natively before publication, not just by the UI.
   Dead process-identity metadata is released after quiescence so successive
   64-bit games cannot exhaust the loader's fixed identity registry.

The backend remains alive on the home page. Its registry and permanent server
objects are not initialized a second time, removing the old duplicate
`\\Registry` startup path. Warm Play does not allocate another debugger JIT pool.

## Profiles and limits

- Folder games and Steam entries explicitly configured to start their own EXE
  can use this route. Steam ownership/launch and cloud gates are not skipped.
  An authenticated Dock route, desktop session, utility installer, remote Metal
  or enabled experimental D3D12 backend needs separate validation/engine handling;
  it is not silently substituted by this direct-game host.
  A 32-bit entry with the optional native D3D9 frontend also keeps the direct
  route: that frontend's native worker pool is not covered by Wine-thread
  lifecycle evidence, so it cannot be advertised as safely reusable yet.
- Per-game resolution, arguments, controls, FPS mode, CPU reporting and renderer
  compatibility are applied again. Named environment overrides are merged into
  the parent's Windows environment (preserving PATH, SYSTEMROOT, TEMP etc.);
  absent game overrides remove values inherited from the previous game. No
  credentials or broad native environment dump are written into IPC.
- Missing controller preferences are seeded through the live Windows registry.
  Madeira must not rewrite `user.reg` while wineserver owns that registry.
- Core options that backends latch (JIT pool, synchronization, x87 precision,
  anisotropy/profile opt-outs) cannot be claimed as live switches. A different
  incompatible profile asks for an engine restart. Crashes, failed child startup,
  undrainable native workers or unsafe retirement can also require restart.
- `env.MADEIRA_MULTI_GAME = 0` before starting Madeira retains the legacy direct
  route and its one-session guard. Explicit `env.MADEIRA_JIT_IMAGE_RETIRE = 0`
  also disables reusable launches. No existing safety guard is simply removed.

The home page shows starting/draining/ready/restart-required status. Logs use
`[runtime-host]`, `[multi-game]` and `[runtime-retire]`; running frames do not emit
continuous lifecycle logs. Settings or an in-game three-line menu can send the
current log as before.

## Build and checks

Use **Madeira v0.1.5 source bootstrap + IPA** on `codex/log-upload-updates`.
This change requires new ntdll and DXMT native archives plus the ARM64 session host.
`check-ios-build.sh` rejects old cached archives lacking lifecycle/GPU hooks.
The DXMT source change is reproduced by `patches/dxmt-runtime-lifecycle.patch`;
no unpublished submodule change is needed.

`tests/host/check-multi-game.py --require-tools --require-swift` compiles the
actual native readiness policy with Mach dependency substitutes and the actual
packet/Swift codecs. Tests cover pending threads, arbitrary query failure,
GPU completion, descendants, PID/PEB reuse across two generations and unsafe
retirement. On Windows it also runs the real session host with three synthetic
launches, checking one persistent host PID, distinct child PIDs, environment
changes, preserved SYSTEMROOT and waiting for a grandchild. No user game or
Steam client is run by these tests.

Device acceptance: Play A, Quit, reach home, wait for **ready**, Play B, Quit,
then Play A again. Require `[runtime-host] supervisor=native-arm64`,
`[ec-child-ntdll]` for an x64 child, and `state=ready` after healthy cleanup.
Send a log from each game and after cleanup. Check resolution,
all input, sound, frame timings, RAM and repeated fixed-base loads. Repeat with
a bootstrapper/child-process game, then abnormal exit and forced Quit. Normal
healthy quits should not require app reset; unverified cleanup must never allow
an overlapping new game.
