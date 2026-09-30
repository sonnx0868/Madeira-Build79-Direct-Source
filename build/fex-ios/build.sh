#!/bin/bash
# Configure and build the FEXCore static libraries linked by Madeira.app.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build="$root/FEX/build-ios"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"

cmake -S "$root/FEX" -B "$build" -G Ninja \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_SYSTEM_PROCESSOR=arm64 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0 \
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
    -DENABLE_OFFLINE_TELEMETRY=OFF \
    -DCMAKE_DISABLE_FIND_PACKAGE_fmt=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_range-v3=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_unordered_dense=ON

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
    [[ -f "$build/$path" ]] || { echo "Missing FEX output: $path" >&2; exit 1; }
    [[ "$(xcrun lipo -archs "$build/$path")" = arm64 ]] || {
        echo "Wrong FEX archive architecture: $path" >&2
        exit 1
    }
done

printf 'FEX iOS static libraries ready in %s\n' "$build"
