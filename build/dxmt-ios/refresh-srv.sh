#!/bin/bash
# Recompile the shader read emitter and its cache namespace only.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
bash "$root/scripts/check-srv-vector-load.sh"
llvm="$root/toolchains/llvm15-macos"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
out="$root/build/dxmt-ios/srv-refresh"
archive="$root/app/Madeira/libdxmt_combined.a"
mkdir -p "$out"
cp "$archive" "$out/base.a"
xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -std=c++20 -O2 -fno-exceptions -fno-rtti -fblocks \
  -I"$llvm/include" -I"$root/build/dxmt-ios" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
  -I"$root/dxmt/include/native/directx" -I"$root/dxmt/include/native/windows" \
  -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/airconv/nt/dxbc_converter_base.cpp" -o "$out/dxbc_converter_base.o"
python3 "$root/tools/dxmt-srv-bundle.py" --root "$root" --base "$out/base.a" --prepare "$out/dxbc_converter_base.o"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -fblocks -O2 -x objective-c -I"$out" -I"$root/build" -I"$root/dxmt/include" \
  -I"$root/dxmt/libs" -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/cache.c" -o "$out/cache.o"
xcrun ar r "$archive" "$out/dxbc_converter_base.o" "$out/cache.o"
xcrun ranlib "$archive"
python3 "$root/tools/dxmt-srv-bundle.py" --root "$root" --base "$out/base.a" --stamp
echo 'Refreshed bounded SRV reads and AIR cache identity; all other DXMT/LLVM objects and Wine/FEX binaries preserved'
