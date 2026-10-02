#!/bin/bash
# Configure and build the upstream FEXCore archives for iOS arm64.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build="$root/FEX/build-ios"
patch="$root/build/fex-ios/clean-build.patch"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"

# The public FEX revision contains two ARM64EC-only diagnostics in sources
# that are also compiled for native iOS. Keep upstream pinned and apply only
# this auditable clean-build portability patch in the CI checkout.
if git -C "$root/FEX" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "FEX iOS portability patch already applied"
else
    git -C "$root/FEX" apply --check "$patch"
    git -C "$root/FEX" apply "$patch"
fi

cmake -S "$root/FEX" -B "$build" -G Ninja \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_SYSTEM_PROCESSOR=arm64 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=18.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DTUNE_CPU=none \
    -DTUNE_ARCH=generic \
    -DBUILD_TESTING=OFF \
    -DBUILD_THUNKS=OFF \
    -DBUILD_FEXCONFIG=OFF \
    -DBUILD_FEX_LINUX_TESTS=OFF \
    -DENABLE_FEX_ALLOCATOR=OFF \
    -DENABLE_ASSERTIONS=OFF \
    -DENABLE_CLANG_THUNKS=ON \
    -DENABLE_CCACHE=ON \
    -DENABLE_LTO=OFF \
    -DENABLE_OFFLINE_TELEMETRY=OFF

cmake --build "$build" --parallel "$jobs" --target \
    FEXCore FEXCore_Base JemallocLibs fmt cephes_128bit xxhash softfloat_3e

required=(
  FEXCore/Source/libFEXCore.a
  FEXCore/Source/libFEXCore_Base.a
  FEXCore/Source/libJemallocLibs.a
  External/fmt/libfmt.a
  External/cephes/libcephes_128bit.a
  External/xxhash/cmake_unofficial/libxxhash.a
  External/SoftFloat-3e/libsoftfloat_3e.a
)
for path in "${required[@]}"; do
    test -s "$build/$path" || { echo "Missing FEX output: $path" >&2; exit 1; }
done

echo "FEX iOS static libraries ready in $build"
