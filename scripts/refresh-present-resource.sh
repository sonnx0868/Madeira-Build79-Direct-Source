#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
python3 "$root/tests/host/check-present-resource.py" --require-darwin
python3 "$root/tests/host/check-light-graphics.py" --require-swift
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
out="$root/build/dxmt-ios/present-resource-refresh"
mkdir -p "$out"
archive="$root/app/Madeira/libdxmt_combined.a"
cp "$archive" "$out/base.a"
cp "$root/app/Madeira/arm64ec-windows/d3d11.dll" "$out/base-d3d11.dll"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -fblocks -O2 -x objective-c -I"$out" -I"$root/build" -I"$root/dxmt/include" \
  -I"$root/dxmt/libs" -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/winemetal_unix.c" -o "$out/winemetal_unix.o"
cp "$archive" "$out/candidate.a"
xcrun ar r "$out/candidate.a" "$out/winemetal_unix.o"
xcrun ranlib "$out/candidate.a"
mv "$out/candidate.a" "$archive"
bash "$root/scripts/refresh-dxmt-query.sh"
python3 "$root/tools/dxmt-present-resource-bundle.py" --root "$root" --stamp \
  --base "$out/base.a" --base-dll "$out/base-d3d11.dll"
echo 'Refreshed native creation lifetime and ARM64EC Present/filtering; other native objects unchanged'
