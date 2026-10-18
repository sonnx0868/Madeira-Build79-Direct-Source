#!/bin/bash
# Rebuild the optimizer and cache identity against the restored LLVM15 archive.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
llvm="$root/toolchains/llvm15-macos"
out="$root/build/dxmt-ios/optimizer-refresh"
archive="$root/app/Madeira/libdxmt_combined.a"
mkdir -p "$out" "$llvm" "$out/shader-headers"
if [[ ! -s "$llvm/include/llvm/IR/IRBuilder.h" ]]; then
  download="${TMPDIR:-/tmp}/madeira-llvm15-macos.tar.xz"
  curl --fail --location --retry 3 --silent --show-error \
    'https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/clang%2Bllvm-15.0.7-arm64-apple-darwin22.0.tar.xz' -o "$download"
  printf '%s  %s\n' 867c6afd41158c132ef05a8f1ddaecf476a26b91c85def8e124414f9a9ba188d "$download" | shasum -a 256 -c -
  tar -xJf "$download" --strip-components=1 -C "$llvm"
fi
test "$("$llvm/bin/llvm-config" --version)" = 15.0.7
xcrun clang++ $("$llvm/bin/llvm-config" --cxxflags) -std=c++20 -O2 \
  -I"$root/dxmt/src/airconv" -I"$root/build/dxmt-ios" \
  "$root/tests/host/air-sample-convergence.cpp" "$root/dxmt/src/airconv/nt/air_builder.cpp" \
  $("$llvm/bin/llvm-config" --ldflags --system-libs --libs core passes) \
  -Wl,-rpath,"$llvm/lib" -o "$out/convergence-check"
"$out/convergence-check"
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
python3 "$root/tools/dxmt-optimizer-bundle.py" --root "$root" --base "$out/base.a" --prepare "$out/airconv_context.o"
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -fblocks -O2 -x objective-c -I"$out" -I"$root/build" -I"$root/dxmt/include" \
  -I"$root/dxmt/libs" -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/cache.c" -o "$out/cache.o"
xcrun ar r "$archive" "$out/airconv_context.o" "$out/cache.o"
xcrun ranlib "$archive"
python3 "$root/tools/dxmt-optimizer-bundle.py" --root "$root" --base "$out/base.a" --stamp
echo 'Refreshed shader optimizer and content-based cache namespace'
