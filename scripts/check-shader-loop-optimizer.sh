#!/bin/bash
# Standalone experiment; neither the native app bundle nor its defaults change.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$BITRISE_DEPLOY_DIR"
mkdir -p "$out" "$root/toolchains/llvm15-macos"
exec > >(tee "$out/shader-loop-check.log") 2>&1
llvm="$root/toolchains/llvm15-macos"
if [[ ! -s "$llvm/include/llvm/IR/IRBuilder.h" ]]; then
  archive="${TMPDIR:-/tmp}/madeira-llvm15-macos.tar.xz"
  curl --fail --location --retry 3 --silent --show-error \
    'https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.7/clang%2Bllvm-15.0.7-arm64-apple-darwin22.0.tar.xz' -o "$archive"
  printf '%s  %s\n' 867c6afd41158c132ef05a8f1ddaecf476a26b91c85def8e124414f9a9ba188d "$archive" | shasum -a 256 -c -
  tar -xJf "$archive" --strip-components=1 -C "$llvm"
fi
test "$("$llvm/bin/llvm-config" --version)" = 15.0.7
git -C "$root" submodule update --init dxmt
bash "$root/scripts/apply-dxmt-patches.sh"
python3 "$root/tests/host/check-dxmt-patches.py"
xcrun clang++ $("$llvm/bin/llvm-config" --cxxflags) -std=c++20 -O2 \
  -I"$root/dxmt/src/airconv" -I"$root/build/dxmt-ios" \
  "$root/tests/host/air-sample-convergence.cpp" "$root/dxmt/src/airconv/nt/air_builder.cpp" \
  $("$llvm/bin/llvm-config" --ldflags --system-libs --libs core passes) \
  -Wl,-rpath,"$llvm/lib" -o "$out/convergence-check"
"$out/convergence-check"
python3 "$root/tests/host/check-guarded-pcf.py" \
  --llvm-config "$llvm/bin/llvm-config" \
  --cxx "$(xcrun --find clang++)" --cc "$(xcrun --find clang)"
if ! xcrun --find metal-tt >/dev/null 2>&1; then
  xcodebuild -downloadComponent MetalToolchain
fi
python3 "$root/tools/shader-loop-lab.py"
