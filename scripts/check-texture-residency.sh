#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:-$root/build/texture-residency-check}"
mkdir -p "$out"
exec > >(tee "$out/texture-residency.log") 2>&1
xcrun clang -fno-objc-arc -fblocks -O2 -g -Wall -Wextra -Werror=implicit-function-declaration \
  -framework Foundation -framework Metal "$root/tests/host/texture-residency.m" -o "$out/texture-residency"
python3 "$root/tools/run-texture-residency-test.py" "$out"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -fno-objc-arc -fblocks -O2 -x objective-c -c "$root/build/dxmt-ios/texture_residency.m" \
  -o "$out/texture-residency-ios.o"
echo 'PASS: native Metal paging/parity/failure fixtures and iOS compilation'
