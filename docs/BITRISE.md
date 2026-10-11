# Madeira on Bitrise

Use Bitrise CI directly; GitHub hosts the source, not the build runner.
The configuration has manual workflows while comparing it with Codemagic.

## Connect the project

1. Add a Bitrise CI project in your workspace for
   `https://github.com/sonnx0868/Madeira-Build79-Direct-Source.git`.
2. Select branch `codex/log-upload-updates` and the existing root `bitrise.yml`.
   Store the configuration in the repository so each build uses its own source.
3. Use an Apple Silicon Xcode stack. The configuration uses
   `osx-xcode-27.0.x`, offered by this workspace's trial. Madeira's helper
   needs iOS SDK 26 or later.
4. Keep build access private. The artifact step disables its public install
   page and email notifications. No Apple signing certificate is needed for
   this ad-hoc Debug IPA; sideload it with the usual signing tool.

Artifacts are exported as `Madeira-v0.1.6-artifacts.zip`. Extract that outer ZIP
to get the IPA and logs. Bitrise's installable-IPA parser requires
`embedded.mobileprovision`; this sideload build receives its provisioning
profile from your signing tool later, so it is uploaded as a generic ZIP.

## Choose a workflow

| Workflow | Inputs | Output |
| --- | --- | --- |
| `madeira-v016-ipa` | Compatible native ZIP URL and SHA-256 | Debug IPA, release manifest, IPA hash, logs, phase timings |
| `madeira-v016-bootstrap` | Pinned source and native build scripts | The same outputs plus a reusable native ZIP and its SHA-256 |

For app-only changes, add these project environment variables:

- `MADEIRA_NATIVE_DEPS_URL`: a direct download URL for the ZIP produced by
  `scripts/package-native-deps.sh`, on Bitrise, Codemagic or another artifact host.
- `MADEIRA_NATIVE_DEPS_SHA256`: the 64 hexadecimal digit hash from that build.
- `MADEIRA_NATIVE_DEPS_TOKEN`: optional; store it as a Bitrise Secret when the
  download endpoint requires a Bearer token. Never put credentials in this repo.

Bitrise artifact download URLs may expire; use a fresh link or durable storage.
Do not substitute an IPA for the native ZIP: the ZIP also contains static
libraries and generated Wine headers needed for linking the app.

Without a native ZIP, run `madeira-v016-bootstrap` once. It selects the Large
resource class with 14 GB RAM and caps compilation at four jobs. The cached
workflow also uses Large: this workspace's limited trial does not offer Medium.
Check which machine classes and build duration limits
your trial or plan includes before starting a full native build.

The cached workflow also verifies/downloads Xcode's Metal Toolchain component:
the app's SwiftUI effects contain Metal shaders even when native libraries are
restored rather than rebuilt.
Native restore initializes FEX's pinned dependency headers and DirectX headers,
then refreshes the generated license copies required by the app's build phase.

Only reuse bundles from builds with compatible native sources and ABI.
Changes in FEX, Wine, DXMT, their patches or native build flags require a fresh
bootstrap. `restore-native-deps.sh` verifies the archive hash and invokes the
existing native preflight; a missing, old or invalid bundle fails the build.
The preflight does not establish compatibility with every possible native
source change. Track which source commit produced the bundle.

## Compare speed

The build exports `madeira-build.log` and `build-timings.tsv`, including native
restore/build, app checks, Xcode compilation and IPA packaging. Bitrise's build
page separately records queue and provisioning time; those occur before the
script and are not included in its timings. Compare the same source and native
bundle against Codemagic before claiming a speed improvement.

The Xcode 27 Metal compiler rejects the pinned DXMT tessellation helper's
private atomic builtin. The integration patch uses the public
`atomic_fetch_add_explicit` API with the same threadgroup counter and relaxed
ordering, matching the upstream DXMT fix. Bootstrap compiles all three AIR
helpers early, before Wine/FEX/Mesa/LLVM, so a compiler mismatch fails promptly.

The build stamps the full Git history count into both app and helper and
records the exact source commit in `madeira-update.json`. It does not publish a
GitHub release or change the app's update server automatically.
