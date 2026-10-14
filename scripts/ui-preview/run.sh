#!/bin/bash
set -euo pipefail
root="${BITRISE_SOURCE_DIR:-$(cd "$(dirname "$0")/../.." && pwd)}"
work="$root/build/ui-preview"
out="${BITRISE_DEPLOY_DIR:-$work/artifacts}"
mkdir -p "$work" "$out"
exec > >(tee "$out/ui-preview.log") 2>&1
python3 "$root/tests/host/check-game-log-archive.py"
python3 "$root/tests/host/check-gpu-lifecycle.py"
python3 "$root/tests/host/check-game-pointer.py" --require-swift
python3 "$root/tests/host/check-thermal-pacing.py" --require-swift
python3 "$root/tests/host/check-library-privacy.py" --require-swift
python3 "$root/tests/host/check-engine-detection.py" --require-swift
python3 "$root/tests/host/check-gog-rules.py"
python3 "$root/tests/host/check-epic-manifest.py"
python3 "$root/scripts/ui-preview/generate.py" "$work"
app="$work/MadeiraPreview.app"
mkdir -p "$app/madeira-art"
for id in 367520 1663850 1030300 3097560 2350270 2333000; do
  for kind in cover hero; do
    [[ "$kind" = cover ]] && name=library_600x900.jpg || name=library_hero.jpg
    curl --fail --location --retry 2 --max-time 30 \
      "https://cdn.cloudflare.steamstatic.com/steam/apps/$id/$name" \
      -o "$app/madeira-art/${kind}_${id}.jpg" || true
  done
done

sdk="$(xcrun --sdk iphonesimulator --show-sdk-path)"
xcrun --sdk iphonesimulator clang -fobjc-arc -fmodules -fblocks \
  -target arm64-apple-ios17.0-simulator -isysroot "$sdk" \
  -c "$root/app/Madeira/IOSDisplayShim.m" -o "$work/IOSDisplayShim.o"
xcrun --sdk iphonesimulator swiftc -parse-as-library -swift-version 5 -emit-object \
  -target arm64-apple-ios17.0-simulator -sdk "$sdk" \
  -import-objc-header "$root/app/Madeira/IOSDisplayShim.h" \
  "$work/DiagnosticTypes.swift" -o "$work/DiagnosticTypes.o"
echo 'PASS: production Objective-C GPU adapter and Swift diagnostic bridge compile against iOS SDK'
xcrun --sdk iphonesimulator metal -target air64-apple-ios17.0-simulator -isysroot "$sdk" \
  -c "$root/app/Madeira/LiquidMetalSkin.metal" -o "$work/LiquidMetalSkin.air"
xcrun --sdk iphonesimulator metallib "$work/LiquidMetalSkin.air" -o "$app/default.metallib"
xcrun --sdk iphonesimulator swiftc -parse-as-library -swift-version 5 \
  -target arm64-apple-ios17.0-simulator -sdk "$sdk" \
  "$work/Preview.swift" -o "$app/MadeiraPreview"
xcrun actool --platform iphonesimulator --target-device ipad \
  --minimum-deployment-target 17.0 --compile "$app" "$work/Assets.xcassets"
python3 - "$app/Info.plist" <<'PY'
import plistlib,sys
with open(sys.argv[1],'wb') as f:
    plistlib.dump(dict(CFBundleIdentifier='org.madeira.ui-preview',CFBundleExecutable='MadeiraPreview',
        CFBundleName='Madeira Preview',CFBundleVersion='1',CFBundleShortVersionString='1.0',
        CFBundlePackageType='APPL',LSRequiresIPhoneOS=True,UIDeviceFamily=[2],
        UILaunchScreen={},UIApplicationSceneManifest={'UIApplicationSupportsMultipleScenes':False},
        UISupportedInterfaceOrientations=['UIInterfaceOrientationLandscapeLeft','UIInterfaceOrientationLandscapeRight']),f)
PY
codesign --force --sign - "$app"
xcrun simctl list devices available --json > "$work/devices.json"
device="$(python3 - "$work/devices.json" <<'PY'
import json,sys
devices=[d for group in json.load(open(sys.argv[1]))['devices'].values() for d in group if 'iPad' in d['name'] and d.get('isAvailable')]
if not devices: raise SystemExit('No installed iPad simulator runtime on this runner')
devices.sort(key=lambda d:('13-inch' not in d['name'],'Pro' not in d['name']))
print(devices[0]['udid'])
PY
)"
xcrun simctl boot "$device" || true
xcrun simctl bootstatus "$device" -b
xcrun simctl install "$device" "$app"
xcrun simctl launch "$device" org.madeira.ui-preview
container="$(xcrun simctl get_app_container "$device" org.madeira.ui-preview data)"
for screen in library library-metal library-restored private pin; do
  for attempt in $(seq 1 30); do
    [[ ! -f "$container/Documents/$screen.ready" ]] || break
    sleep 1
  done
  test -f "$container/Documents/$screen.ready"
  xcrun simctl io "$device" screenshot "$out/$screen-screen.png"
  touch "$container/Documents/$screen.captured"
  cp "$container/Documents/$screen.png" "$out/$screen.png"
done
for attempt in $(seq 1 10); do
  [[ ! -f "$container/Documents/screenshots-ready.txt" ]] || break
  sleep 1
done
cp "$container/Documents/search-checks.json" "$out/search-checks.json"
cp "$container/Documents/search-layers.txt" "$out/search-layers.txt"
python3 - "$out/search-checks.json" <<'PY'
import json,sys
checks=json.load(open(sys.argv[1]))
assert all(checks[k]['nativeSearchBars']==1 for k in ('library','library-metal','library-restored')),checks
assert checks['library']['metalLayers']==checks['library-restored']['metalLayers']==0,checks
if checks['library-metal']['metalDeviceAvailable']:
    assert checks['library-metal']['metalLayers']>0,checks
    assert checks['library-metal']['renderedFrames']>0,checks
    assert checks['hidden-library']['renderedFrames']==0,checks
    print('PASS: native inline search gains Metal when enabled and removes it when disabled')
    print('PASS: hidden library submits no Metal frames while visible search remains animated')
else:
    assert checks['library-metal']['metalLayers']==0,checks
    print('PASS: native search remains usable when simulator has no Metal device')
    print('SKIP: visual Liquid metal verification requires a runner with Metal GPU support')
PY
cp "$work/preview-source.json" "$out/preview-source.json"
echo 'PASS: production SwiftUI shelf/search/PIN rendered on iPad Simulator with fixture data'
