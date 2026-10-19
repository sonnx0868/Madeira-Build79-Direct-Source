#!/bin/bash
# Rebuild the retirement worker and Present frontend; preserve all other objects.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
python3 "$root/tests/host/check-gpu-retirement.py" --require-darwin
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
out="$root/build/dxmt-ios/retirement-refresh"
mkdir -p "$out"
archive="$root/app/Madeira/libdxmt_combined.a"
cp "$archive" "$out/base.a"
cp "$root/app/Madeira/arm64ec-windows/d3d11.dll" "$out/base-d3d11.dll"
src="$root/dxmt/src"
xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
    -fblocks -O2 -std=c++20 -fexceptions -frtti -Wno-extern-c-compat -Wno-microsoft-exception-spec \
    -DDXMT_NATIVE=1 -DDXMT_MADEIRA=1 -DDXMT_IOS=1 -DDXMT_PAGE_SIZE=4096 -DNOMINMAX \
    -I"$src/nativemetal" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
    -I"$src/winemetal" -I"$src/airconv" -I"$src/util" -I"$src/dxmt" \
    -I"$src/d3d9" -I"$src/d3d9/unix" -I"$src/d3d9shim" \
    -I"$root/dxmt/include/native/directx" -I"$root/dxmt/include/native/windows" \
    -I"$root/build/dxmt-ios/shader-headers" -c "$src/dxmt/dxmt_command_queue.cpp" -o "$out/dxmt_command_queue.o"
cp "$archive" "$out/candidate.a"
xcrun ar r "$out/candidate.a" "$out/dxmt_command_queue.o"
xcrun ranlib "$out/candidate.a"
mv "$out/candidate.a" "$archive"
bash "$root/scripts/refresh-dxmt-query.sh"
python3 "$root/tools/dxmt-retirement-bundle.py" --root "$root" --stamp \
    --base "$out/base.a" --base-dll "$out/base-d3d11.dll"
echo 'Refreshed Metal retirement lifetime and seven Present wait sites; other native objects preserved'
