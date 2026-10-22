#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
out="$root/build/dxmt-ios/padded-light-refresh"
llvm="$root/toolchains/llvm15-macos"
archive="$root/app/Madeira/libdxmt_combined.a"
if python3 "$root/tools/dxmt-padded-light-bundle.py" --root "$root" >/dev/null 2>&1; then
  echo 'Verified padded-light native objects already match these sources'
  exit 0
fi
mkdir -p "$out/shader-headers" "$llvm"
if [[ ! -s "$llvm/include/llvm/IR/IRBuilder.h" ]]; then
  download="${TMPDIR:-/tmp}/madeira-llvm15-macos.tar.xz"
  curl --fail --location --retry 3 --retry-all-errors --retry-delay 2 --connect-timeout 30 --silent --show-error \
    'https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/clang%2Bllvm-15.0.7-arm64-apple-darwin22.0.tar.xz' -o "$download"
  printf '%s  %s\n' 867c6afd41158c132ef05a8f1ddaecf476a26b91c85def8e124414f9a9ba188d "$download" | shasum -a 256 -c -
  tar -xJf "$download" --strip-components=1 -C "$llvm"
fi
test "$("$llvm/bin/llvm-config" --version)" = 15.0.7
bash "$root/scripts/check-padded-lighting.sh"
cp "$archive" "$out/base.a"
for shader in air_msad air_samplepos air_tessellation; do
  xcrun -sdk macosx metal -o "$out/shader-headers/$shader.air" -c "$root/dxmt/src/airconv/shaders/$shader.metal" \
    -std=metal3.1 --target=air64-apple-macos14.0
  xxd -n "$shader" -i "$out/shader-headers/$shader.air" "$out/shader-headers/$shader.h"
done
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -std=c++20 -O2 -fno-exceptions -fno-rtti -fblocks \
  -I"$llvm/include" -I"$root/build/dxmt-ios" -I"$out/shader-headers" \
  -I"$root/dxmt/src/airconv" -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS \
  -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS \
  -c "$root/dxmt/src/airconv/airconv_context.cpp" -o "$out/airconv_context.o"
python3 "$root/tools/dxmt-padded-light-bundle.py" --root "$root" --prepare "$out/airconv_context.o"
xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -std=c++20 -O2 -fno-exceptions -fno-rtti -fblocks -I"$llvm/include" \
  -I"$root/dxmt/src/airconv" -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS \
  -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS \
  -c "$root/dxmt/src/airconv/metallib_writer.cpp" -o "$out/metallib_writer.o"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -fblocks -O2 -x objective-c -I"$out" -I"$root/build" -I"$root/dxmt/include" \
  -I"$root/dxmt/libs" -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/cache.c" -o "$out/cache.o"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" \
  -miphoneos-version-min=18.0 -fno-objc-arc -fblocks -O2 -x objective-c \
  -I"$out" -I"$root/build" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
  -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/winemetal_unix.c" -o "$out/winemetal_unix.o"
xcrun ar r "$archive" "$out/airconv_context.o" "$out/cache.o" "$out/metallib_writer.o" "$out/winemetal_unix.o"
xcrun ranlib "$archive"
python3 "$root/tools/dxmt-padded-light-bundle.py" --root "$root" --stamp --base "$out/base.a"
echo 'Refreshed full-resolution padded lighting; existing guest runtime and texture pager preserved'
