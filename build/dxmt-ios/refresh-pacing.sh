#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
out="$root/build/dxmt-ios/pacing-obj"
archive="$root/app/Madeira/libdxmt_combined.a"
mkdir -p "$out"
test -s "$archive"
# Recompile only the existing Objective-C boundary with its original flags.
# No shader compiler, PE DLL, FEX, audio or Wine rebuild is involved.
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" \
  -miphoneos-version-min=18.0 -fblocks -O2 -x objective-c \
  -I"$out" -I"$root/build" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
  -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  -c "$root/dxmt/src/winemetal/unix/winemetal_unix.c" -o "$out/winemetal_unix.o"
candidate="$out/libdxmt_pacing.a"
cp "$archive" "$candidate"
xcrun ar r "$candidate" "$out/winemetal_unix.o"
xcrun ranlib "$candidate"
python3 "$root/tools/dxmt-pacing-bundle.py" --root "$root" --archive "$candidate" --stamp --base "$archive"
mv "$candidate" "$archive"
echo 'Refreshed winemetal_unix.o producer pacing; every other DXMT/LLVM object unchanged'
