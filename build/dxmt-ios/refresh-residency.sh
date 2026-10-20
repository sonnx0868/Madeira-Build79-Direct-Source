#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
out="$root/build/dxmt-ios/residency-refresh"
archive="$root/app/Madeira/libdxmt_combined.a"
mkdir -p "$out"
python3 "$root/tools/dxmt-residency-bundle.py" --root "$root" --prepare
bash "$root/scripts/check-texture-residency.sh"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" \
  -miphoneos-version-min=18.0 -fno-objc-arc -fblocks -O2 -x objective-c \
  -I"$out" -I"$root/build" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
  -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/winemetal_unix.c" -o "$out/winemetal_unix.o"
cp "$archive" "$out/base.a"
xcrun ar r "$archive" "$out/winemetal_unix.o"
xcrun ranlib "$archive"
python3 "$root/tools/dxmt-residency-bundle.py" --root "$root" --stamp --base "$out/base.a"
echo 'Refreshed the Metal residency boundary; shader compiler and Windows DLLs preserved'
