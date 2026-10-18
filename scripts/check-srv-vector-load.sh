#!/bin/bash
# Compile the production LLVM emitter and execute both paths on a real Mac GPU.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:-$root/build/srv-load-check}"
llvm="$root/toolchains/llvm15-macos"
mkdir -p "$out" "$llvm"
exec > >(tee "$out/srv-load-check.log") 2>&1
archive="${TMPDIR:-/tmp}/madeira-llvm15-macos.tar.xz"
if [[ ! -s "$llvm/include/llvm/IR/IRBuilder.h" ]]; then
  curl --fail --location --retry 3 \
    'https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/clang%2Bllvm-15.0.7-arm64-apple-darwin22.0.tar.xz' -o "$archive"
  printf '%s  %s\n' 867c6afd41158c132ef05a8f1ddaecf476a26b91c85def8e124414f9a9ba188d "$archive" | shasum -a 256 -c -
  tar -xJf "$archive" --strip-components=1 -C "$llvm"
fi
"$llvm/bin/llvm-config" --version
test "$("$llvm/bin/llvm-config" --version)" = 15.0.7
git -C "$root" submodule update --init dxmt
bash "$root/scripts/apply-dxmt-patches.sh"
python3 "$root/tests/host/check-dxmt-patches.py"
xcrun clang++ $("$llvm/bin/llvm-config" --cxxflags) -std=c++20 -O2 \
  -I"$root/build/dxmt-ios" -I"$root/dxmt/src/airconv" \
  "$root/tests/host/gpu-srv-load-emit.cpp" "$root/dxmt/src/airconv/metallib_writer.cpp" \
  $("$llvm/bin/llvm-config" --ldflags --system-libs --libs core bitwriter) \
  -Wl,-rpath,"$llvm/lib" -o "$out/emitter"
"$out/emitter" scalar "$out/scalar"
"$out/emitter" vector "$out/vector"
"$out/emitter" scalar-heavy "$out/scalar-heavy"
"$out/emitter" vector-heavy "$out/vector-heavy"
xcrun clang -O2 -fobjc-arc "$root/tests/host/gpu-srv-load-metal.m" \
  -framework Foundation -framework Metal -o "$out/metal-check"
"$out/metal-check" "$out/scalar.metallib" "$out/vector.metallib" "$out/scalar-heavy.metallib" "$out/vector-heavy.metallib"
# A shipping source refresh must also compile the real caller, not only a test wrapper.
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
git -C "$root/dxmt" submodule update --init include/native/directx
xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -std=c++20 -O2 -fno-exceptions -fno-rtti -fblocks -fsyntax-only \
  -I"$llvm/include" -I"$root/build/dxmt-ios" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
  -I"$root/dxmt/include/native/directx" -I"$root/dxmt/include/native/windows" \
  -I"$root/dxmt/src/winemetal" -I"$root/dxmt/src/airconv" \
  "$root/dxmt/src/airconv/nt/dxbc_converter_base.cpp"
echo 'PASS: real shader converter compiles against iPhoneOS SDK'
