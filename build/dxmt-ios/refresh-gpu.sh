#!/bin/bash
# Keep LLVM, FEX, Wine and all other native objects exactly as restored.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
python3 "$root/tests/host/check-pipeline-cache.py"
python3 "$root/tests/host/check-live-pipeline-cache.py"
python3 "$root/tests/host/check-shader-warmup.py"
python3 "$root/tests/host/check-color-store.py"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
out="$root/build/dxmt-ios/gpu-refresh"
archive="$root/app/Madeira/libdxmt_combined.a"
mkdir -p "$out"
cp "$archive" "$out/libdxmt-base.a"
cp "$root/app/Madeira/arm64ec-windows/d3d11.dll" "$out/d3d11-base.dll"
for name in pipeline_cache_ios pipeline_warmup_ios; do
  xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" \
    -miphoneos-version-min=18.0 -fblocks -O2 -fobjc-arc \
    -c "$root/build/dxmt-ios/$name.m" -o "$out/$name.o"
done
cp "$archive" "$out/libdxmt-gpu.a"
xcrun ar r "$out/libdxmt-gpu.a" "$out/pipeline_cache_ios.o" "$out/pipeline_warmup_ios.o"
xcrun ranlib "$out/libdxmt-gpu.a"
mv "$out/libdxmt-gpu.a" "$archive"
bash "$root/scripts/refresh-dxmt-query.sh"
python3 "$root/tools/dxmt-gpu-bundle.py" --root "$root" --stamp \
  --base "$out/libdxmt-base.a" --base-dll "$out/d3d11-base.dll"
python3 "$root/tools/dxmt-pacing-bundle.py" --root "$root"
echo 'Refreshed GPU pipelines and full-clear color stores; Wine, FEX and other DXMT/LLVM objects preserved'
