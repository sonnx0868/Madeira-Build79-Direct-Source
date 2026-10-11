#!/bin/bash
# Configure (first time) and build the ARM64EC FEX module (libarm64ecfex.dll,
# shipped as xtajit64.dll). Options mirror the development build's CMakeCache.
set -euo pipefail
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export PATH="${LLVM_MINGW:-$R/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin}:$PATH"
B="$R/FEX/build-arm64ec-ios"
python3 "$R/scripts/apply-fex-cache.py" --compiler "$(command -v arm64ec-w64-mingw32-clang++)"
# A host fmt_DIR can survive in CMakeCache.txt or come from Homebrew. Always
# compile the pinned dependencies for Windows instead of importing host libraries.
cmake -S "$R/FEX" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$R/FEX/Data/CMake/toolchain_mingw.cmake" \
        -DMINGW_TRIPLE=arm64ec-w64-mingw32 -DFEX_IOS_HOST_BUILD=ON \
        -DCMAKE_C_FLAGS=-DFEX_IOS_HOST=1 -DCMAKE_CXX_FLAGS=-DFEX_IOS_HOST=1 \
        -DTUNE_CPU=none -DTUNE_ARCH=generic -DENABLE_GUEST_WINDOW=OFF \
        -DENABLE_LTO=OFF -DENABLE_OFFLINE_TELEMETRY=OFF \
        -DCMAKE_DISABLE_FIND_PACKAGE_fmt=ON \
        -DCMAKE_DISABLE_FIND_PACKAGE_range-v3=ON \
        -DCMAKE_DISABLE_FIND_PACKAGE_unordered_dense=ON \
        -DENABLE_FEX_ALLOCATOR=ON -DENABLE_JEMALLOC_GLIBC_ALLOC=ON -DENABLE_OFFLINE_RUNTIME=ON \
        -DBUILD_FEXCONFIG=ON -DENABLE_CLANG_THUNKS=ON -DENABLE_CCACHE=ON \
        -DBUILD_TESTING=OFF -DBUILD_THUNKS=OFF -DENABLE_ASSERTIONS=OFF
cmake --build "$B" --parallel "${JOBS:-4}" --target arm64ecfex
cp "$B/Bin/libarm64ecfex.dll" "$R/app/Madeira/arm64ec-windows/xtajit64.dll" && ls -l "$R/app/Madeira/arm64ec-windows/xtajit64.dll"
