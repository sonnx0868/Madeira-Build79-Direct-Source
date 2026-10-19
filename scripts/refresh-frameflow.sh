#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
# Also carries the verified ARM64EC stack-walk fix and the native WGI adapter.
bash "$root/scripts/refresh-stackwalk.sh"
python3 "$root/tests/host/check-upload-flow.py"
python3 "$root/tests/host/check-shader-warmup.py"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
out="$root/build/dxmt-ios/frameflow-refresh"
mkdir -p "$out"
archive="$root/app/Madeira/libdxmt_combined.a"
cp "$archive" "$out/base.a"
src="$root/dxmt/src"
for name in pipeline_cache_ios pipeline_warmup_ios; do
    xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
        -fblocks -O2 -fobjc-arc -c "$root/build/dxmt-ios/$name.m" -o "$out/$name.o"
done
# Neither changed class grows: ResourceInitializer replaces a redundant size_t
# capacity field, and StagingResource adds no instance data or virtual methods.
# Keep the original flags and direct native winemetal include order.
for name in dxmt_staging dxmt_resource_initializer; do
    xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
        -fblocks -O2 -std=c++20 -fexceptions -frtti -Wno-extern-c-compat -Wno-microsoft-exception-spec \
        -DDXMT_NATIVE=1 -DDXMT_MADEIRA=1 -DDXMT_IOS=1 -DDXMT_PAGE_SIZE=4096 -DNOMINMAX \
        -I"$src/nativemetal" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
        -I"$src/winemetal" -I"$src/airconv" -I"$src/util" -I"$src/dxmt" \
        -I"$src/d3d9" -I"$src/d3d9/unix" -I"$src/d3d9shim" \
        -I"$root/dxmt/include/native/directx" -I"$root/dxmt/include/native/windows" \
        -I"$root/build/dxmt-ios/shader-headers" -c "$src/dxmt/$name.cpp" -o "$out/$name.o"
done
cp "$archive" "$out/candidate.a"
xcrun ar r "$out/candidate.a" "$out/pipeline_cache_ios.o" "$out/pipeline_warmup_ios.o" \
    "$out/dxmt_staging.o" "$out/dxmt_resource_initializer.o"
xcrun ranlib "$out/candidate.a"
mv "$out/candidate.a" "$archive"
bash "$root/scripts/refresh-dxmt-query.sh"
python3 "$root/tools/dxmt-frameflow-bundle.py" --root "$root" --stamp --base "$out/base.a"
python3 "$root/tools/dxmt-gpu-bundle.py" --root "$root" --stamp
python3 "$root/tools/dxmt-pacing-bundle.py" --root "$root"
