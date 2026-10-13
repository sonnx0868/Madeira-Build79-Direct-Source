#!/bin/bash
# Use the same native build and Debug packaging as the Codemagic source line.
set -euo pipefail

mode="${1:-cached}"
case "$mode" in
  cached) ipa=Madeira-v0.1.6-source.ipa ;;
  bootstrap) ipa=Madeira-v0.1.6-source-bootstrap.ipa ;;
  window-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  renpy-gl-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  input-refresh) ipa=Madeira-v0.1.6-source.ipa ;;
  *) echo 'Usage: build-bitrise.sh cached|bootstrap|window-refresh|renpy-gl-refresh|input-refresh' >&2; exit 2 ;;
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
  timed native-bootstrap bash scripts/bootstrap-native-deps.sh
  timed native-package bash scripts/package-native-deps.sh "$out/madeira-v0.1.6-native-deps.zip"
  shasum -a 256 "$out/madeira-v0.1.6-native-deps.zip" > "$out/native-deps-sha256.txt"
else
  # A fresh app-only VM still compiles the bundled SwiftUI Metal effects.
  timed metal-toolchain xcodebuild -downloadComponent MetalToolchain
  xcrun -sdk macosx metal -v >/dev/null
  timed submodules git submodule update --init FEX wine dxmt madeira-dock
  timed wine-patches bash scripts/apply-wine-patches.sh
  timed dxmt-check python3 tests/host/check-dxmt-patches.py
  timed dxmt-patches bash scripts/apply-dxmt-patches.sh
  if [[ "$mode" = window-refresh || "$mode" = renpy-gl-refresh || "$mode" = input-refresh ]]; then
    refresh=--refresh-window
    [[ "$mode" != renpy-gl-refresh ]] || refresh=--refresh-renpy-gl
    [[ "$mode" != input-refresh ]] || refresh=--refresh-input
    timed native-restore bash scripts/restore-native-deps.sh "$refresh"
    timed native-package bash scripts/package-native-deps.sh "$out/madeira-v0.1.6-native-deps.zip"
    shasum -a 256 "$out/madeira-v0.1.6-native-deps.zip" > "$out/native-deps-sha256.txt"
  else
    timed native-restore bash scripts/restore-native-deps.sh
  fi
fi

checks() {
  python3 tests/host/check-bcd-compatibility.py
  python3 tests/host/check-version.py
  python3 tests/host/check-native-wrapper.py
  python3 tests/host/check-renpy-renderer.py
  python3 tests/host/check-renpy-wgl-probe.py
  python3 tests/host/check-native-dll-fallback.py
  python3 tests/host/check-window-proc.py
  python3 tests/host/check-user-object-generation.py
  python3 tests/host/check-bootstrap-resources.py
  python3 tests/host/check-cpu-cache.py
  python3 tests/host/check-cpu-aot-survey.py
  # Bootstrap runs these native fixtures before expensive dependency builds.
  if [[ "$mode" != bootstrap ]]; then
    python3 tests/host/check-pipeline-cache.py
    python3 tests/host/check-shader-warmup.py
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
  python3 tests/host/check-controller-refresh.py --require-swift
  python3 tests/host/check-touch-gamepad.py
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
