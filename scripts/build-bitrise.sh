#!/bin/bash
# Use the same native build and Debug packaging as the Codemagic source line.
set -euo pipefail

mode="${1:-cached}"
# Allow the existing stored Bitrise workflow to request this source refresh.
if [[ "$mode" = cached && "${MADEIRA_NATIVE_SIGNAL_REFRESH:-0}" = 1 ]]; then mode=signal-refresh; fi
if [[ "$mode" = cached && "${MADEIRA_NATIVE_PACING_REFRESH:-0}" = 1 ]]; then mode=pacing-refresh; fi
if [[ "$mode" = cached && "${MADEIRA_D3D11_QUERY_REFRESH:-0}" = 1 ]]; then mode=query-refresh; fi
if [[ "$mode" = cached && "${MADEIRA_NTDLL_LOADER_REFRESH:-0}" = 1 ]]; then mode=loader-refresh; fi
if [[ "$mode" = cached && "${MADEIRA_NATIVE_RUNTIME_REFRESH:-0}" = 1 ]]; then mode=runtime-refresh; fi
if [[ "$mode" = cached && "${MADEIRA_GPU_PIPELINE_REFRESH:-0}" = 1 ]]; then mode=gpu-refresh; fi
case "$mode" in
  cached) ipa=Madeira-v0.1.6-source.ipa ;;
  bootstrap) ipa=Madeira-v0.1.6-source-bootstrap.ipa ;;
  window-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  renpy-gl-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  input-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  signal-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  pacing-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  query-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  loader-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  runtime-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  gpu-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  *) echo 'Usage: build-bitrise.sh cached|bootstrap|window-refresh|renpy-gl-refresh|input-refresh|signal-refresh|pacing-refresh' >&2; exit 2 ;;
esac

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"
if [[ "$(uname -s)" != Darwin || "$(uname -m)" != arm64 ]]; then
  echo 'Madeira requires an Apple Silicon macOS runner with Xcode.' >&2
  exit 1
fi
if [[ "$mode" != bootstrap ]]; then
  : "${MADEIRA_NATIVE_DEPS_URL:?Set MADEIRA_NATIVE_DEPS_URL or run madeira-v016-bootstrap}"
  : "${MADEIRA_NATIVE_DEPS_SHA256:?Set MADEIRA_NATIVE_DEPS_SHA256}"
fi

export CM_BUILD_DIR="$root"
export CM_TEMP_DIR="$root/build/bitrise/tmp"
export JOBS="${JOBS:-4}" HOMEBREW_NO_AUTO_UPDATE=1
out="${BITRISE_DEPLOY_DIR:-$root/build/bitrise/artifacts}"
derived="${DERIVED_DATA:-$root/build/bitrise/derived-data}"
configuration="${APP_CONFIGURATION:-Debug}"
[[ "$configuration" = Debug ]] || {
  echo 'This source runtime is validated with the Debug app configuration.' >&2; exit 1;
}
mkdir -p "$out" "$CM_TEMP_DIR"
# A failed retry must never publish an IPA left by a previous local invocation.
rm -f "$out/$ipa" "$out/madeira-update.json" "$out/ipa-sha256.txt"
exec > >(tee "$out/madeira-build.log") 2>&1
printf 'phase\tseconds\tstatus\n' > "$out/build-timings.tsv"
total_start=$SECONDS
phase=setup
phase_start=$SECONDS
on_exit() {
  status=$?
  if (( status != 0 )); then
    printf '%s\t%d\tfailed:%d\n' "$phase" "$((SECONDS-phase_start))" "$status" >> "$out/build-timings.tsv"
  fi
  printf 'total\t%d\t%d\n' "$((SECONDS-total_start))" "$status" >> "$out/build-timings.tsv"
}
trap on_exit EXIT
timed() {
  phase="$1"; shift
  phase_start=$SECONDS
  printf '\nMadeira build phase: %s\n' "$phase"
  "$@"
  printf '%s\t%d\tok\n' "$phase" "$((SECONDS-phase_start))" >> "$out/build-timings.tsv"
}

xcodebuild -version
xcrun --sdk iphoneos --show-sdk-version
git rev-parse HEAD
sysctl -n hw.memsize hw.ncpu

if [[ "$mode" = bootstrap ]]; then
  # Validate Foundation-only policies before the expensive native compilation.
  timed pe-launch python3 tests/host/check-pe-launch.py
  timed library-privacy python3 tests/host/check-library-privacy.py --require-swift
  timed pe-imports python3 tests/host/check-pe-imports.py --require-swift
  timed cdn-policy python3 tests/host/check-cdn-routing.py --require-swift
  timed diagnostics-policy python3 tests/host/check-remote-support.py --require-swift
  timed frontend-policy python3 tests/host/check-frontend.py
  timed shader-inventory python3 tests/host/check-translation-tools.py
  timed native-bootstrap bash scripts/bootstrap-native-deps.sh
  timed native-package bash scripts/package-native-deps.sh "$out/madeira-v0.1.6-native-deps.zip"
  shasum -a 256 "$out/madeira-v0.1.6-native-deps.zip" > "$out/native-deps-sha256.txt"
else
  # A fresh app-only VM still compiles the bundled SwiftUI Metal effects.
  timed metal-toolchain xcodebuild -downloadComponent MetalToolchain
  xcrun -sdk macosx metal -v >/dev/null
  timed submodules git submodule update --init FEX wine dxmt madeira-dock
  timed wine-patches bash scripts/apply-wine-patches.sh
  timed dock-patches bash scripts/apply-dock-patches.sh
  timed dxmt-check python3 tests/host/check-dxmt-patches.py
  timed dxmt-patches bash scripts/apply-dxmt-patches.sh
  if [[ "$mode" = window-refresh || "$mode" = renpy-gl-refresh || "$mode" = input-refresh || "$mode" = signal-refresh || "$mode" = pacing-refresh || "$mode" = query-refresh || "$mode" = loader-refresh || "$mode" = runtime-refresh || "$mode" = gpu-refresh ]]; then
    refresh=--refresh-window
    [[ "$mode" != renpy-gl-refresh ]] || refresh=--refresh-renpy-gl
    [[ "$mode" != input-refresh ]] || refresh=--refresh-input
    [[ "$mode" != signal-refresh ]] || refresh=--refresh-signal
    [[ "$mode" != pacing-refresh ]] || refresh=--refresh-pacing
    [[ "$mode" != query-refresh ]] || refresh=--refresh-query
    [[ "$mode" != loader-refresh ]] || refresh=--refresh-loader
    [[ "$mode" != runtime-refresh ]] || refresh=--refresh-runtime
    [[ "$mode" != gpu-refresh ]] || refresh=--refresh-gpu
    timed native-restore bash scripts/restore-native-deps.sh "$refresh"
    timed native-package bash scripts/package-native-deps.sh "$out/madeira-v0.1.6-native-deps.zip"
    shasum -a 256 "$out/madeira-v0.1.6-native-deps.zip" > "$out/native-deps-sha256.txt"
  else
    timed native-restore bash scripts/restore-native-deps.sh
  fi
fi

if [[ "${MADEIRA_WOW64_ADDON_ONLY:-0}" = 1 ]]; then
  timed wow64-addon bash scripts/build-wow64-addon.sh
  exit 0
fi
timed wine-mono python3 tools/stage-wine-mono.py
if [[ -n "${MADEIRA_WOW64_ADDON_URL:-}" ]]; then
  timed native-package bash scripts/package-native-deps.sh "$out/madeira-v0.1.6-native-deps.zip"
  shasum -a 256 "$out/madeira-v0.1.6-native-deps.zip" > "$out/native-deps-sha256.txt"
fi

checks() {
  local metrics_sdk
  metrics_sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
  xcrun --sdk iphoneos clang -fobjc-arc -fmodules -fblocks \
    -target arm64-apple-ios17.0 -isysroot "$metrics_sdk" -fsyntax-only app/Madeira/WineProcessBridge.m
  echo 'PASS: production Wine bridge and REPLACED metrics compile against iPhoneOS SDK'
  python3 tests/host/check-wow64-restore.py
  python3 tests/host/check-present-pacing.py
  python3 tests/host/check-runtime-scheduling-loader.py --require-darwin
  python3 tools/ntdll-runtime-bundle.py --root "$root"
  python3 tests/host/check-d3d11-query-park.py
  python3 tools/dxmt-pacing-bundle.py --root "$root"
  python3 tools/dxmt-query-bundle.py --root "$root"
  python3 tools/dxmt-swapchain-bundle.py --root "$root"
  python3 tests/host/check-pe-launch.py
  python3 tests/host/check-windows-arguments.py
  python3 tests/host/check-unity-folder-profile.py
  python3 tests/host/check-game-process.py --require-objc
  python3 tests/host/check-fex-host-features.py
  python3 tests/host/check-lrcpc2-alias.py
  python3 tests/host/check-qreg-alias.py
  python3 tests/host/check-builtin-xinput.py
  python3 tests/host/check-guest-spill.py
  python3 tests/host/check-dxmt-cpu-mappings.py
  python3 tests/host/check-gpu-lifecycle.py
  python3 tests/host/check-workload-diagnostics.py
  python3 tests/host/check-guest-main-qos.py --require-darwin
  python3 tests/host/check-replaced-metrics.py --require-darwin
  python3 tests/host/check-library-recents.py --require-swift
  python3 tests/host/check-jit-detach.py --require-swift
  python3 tests/host/check-jit-request-lifecycle.py --require-swift
  python3 tests/host/check-bcd-compatibility.py
  python3 tests/host/check-game-startup-fixes.py
  python3 tests/host/check-game-log-archive.py
  python3 tests/host/check-gog-rules.py
  python3 tests/host/check-epic-manifest.py
  python3 tests/host/check-wine-mono.py --require-objc
  python3 tests/host/check-dock-contract.py
  python3 tests/host/check-version.py
  python3 tests/host/check-native-wrapper.py
  python3 tests/host/check-renpy-renderer.py
  python3 tests/host/check-renpy-wgl-probe.py
  python3 tests/host/check-native-dll-fallback.py
  python3 tests/host/check-module-path.py
  python3 tests/host/check-external-steam.py
  python3 tests/host/check-window-proc.py
  python3 tests/host/check-user-object-generation.py
  python3 tests/host/check-bootstrap-resources.py
  python3 tests/host/check-cpu-cache.py
  python3 tests/host/check-cpu-aot-survey.py
  # Bootstrap runs these native fixtures before expensive dependency builds.
  if [[ "$mode" != bootstrap ]]; then
    python3 tests/host/check-pipeline-cache.py
    python3 tests/host/check-live-pipeline-cache.py
    python3 tests/host/check-shader-warmup.py
    python3 tests/host/check-color-store.py
    python3 tests/host/check-renpy-startup.py
  fi
  python3 tests/host/check-translation-tools.py
  python3 tests/host/check-remote-support.py --require-swift
  python3 tests/host/check-frontend.py
  python3 tests/host/check-game-memory.py --require-swift
  python3 tests/host/check-game-pointer.py --require-swift
  python3 tests/host/check-presented-input.py --require-swift
  python3 tests/host/check-thermal-pacing.py --require-swift
  python3 tests/host/check-runtime-profilers.py
  python3 tests/host/check-profile-display.py --require-objc
  python3 tests/host/check-engine-detection.py --require-swift
  python3 tests/host/check-gamemaker-stats.py
  python3 tests/host/check-session-polling.py --require-swift
  python3 tests/host/check-library-privacy.py --require-swift
  python3 tests/host/check-pe-imports.py --require-swift
  python3 tests/host/check-cdn-routing.py --require-swift
  python3 tests/host/check-controller-refresh.py --require-swift
  python3 tests/host/check-touch-gamepad.py
  python3 tests/host/check-control-presets.py
  python3 tests/host/check-runtime-preparation.py --require-swift
  python3 tests/host/check-installed-game-repair.py --require-swift
  python3 tests/host/check-gameplay-performance.py --require-tools --require-objc
  python3 tests/host/check-startup-performance.py --require-tools
  python3 tests/host/check-silksong-startup.py --require-tools --require-swift
  python3 tests/host/check-multi-game.py --require-tools --require-swift
}
timed app-checks checks
timed release-identity bash scripts/prepare-release.sh "$ipa"
timed xcodebuild xcodebuild -project "${XCODE_PROJECT:-app/Madeira.xcodeproj}" \
  -scheme "${XCODE_SCHEME:-Madeira}" -configuration "$configuration" \
  -sdk iphoneos -destination 'generic/platform=iOS' -derivedDataPath "$derived" \
  -jobs "$JOBS" CURRENT_PROJECT_VERSION="$(cat build/release-info/build-number.txt)" \
  ENABLE_DEBUG_DYLIB=NO CODE_SIGNING_ALLOWED=NO build

package() {
  app="$derived/Build/Products/$configuration-iphoneos/Madeira.app"
  test -d "$app"
  test -d "$app/Frameworks/StikJIT.framework"
  test -d "$app/PlugIns/MadeiraJITHelper.appex"
  codesign --force --sign - --timestamp=none "$app/Frameworks/StikJIT.framework"
  codesign --force --sign - --timestamp=none "$app/PlugIns/MadeiraJITHelper.appex"
  codesign --force --sign - --entitlements "$root/app/Madeira/Madeira.entitlements" "$app"
  codesign --verify --deep --strict "$app"
  # An isolated staging folder avoids stale Payload entries on a retry.
  staging="$(mktemp -d "$CM_TEMP_DIR/ipa.XXXXXX")"
  mkdir -p "$staging/Payload"
  ditto "$app" "$staging/Payload/Madeira.app"
  (cd "$staging" && /usr/bin/zip -qry "$out/$ipa" Payload)
  /usr/bin/unzip -tq "$out/$ipa"
  cp "$root/build/release-info/madeira-update.json" "$out/madeira-update.json"
  shasum -a 256 "$out/$ipa" > "$out/ipa-sha256.txt"
}
timed ipa-package package
echo "IPA ready: $ipa"
