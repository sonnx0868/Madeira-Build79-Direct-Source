#!/bin/bash
# Build the LLVM 15 static libraries used by DXMT airconv for iOS arm64.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
toolchains="$root/toolchains"
source_dir="$toolchains/llvm-project"
host_build="$toolchains/llvm-host-build"
ios_build="$toolchains/llvm-ios-build"
llvm_commit="8dfdcc7b7bf66834a761bd8de445840ef68e4d1a"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"

mkdir -p "$toolchains"
if [[ ! -d "$source_dir/.git" ]]; then
    git clone --filter=blob:none --no-checkout \
        https://github.com/llvm/llvm-project.git "$source_dir"
fi
git -C "$source_dir" fetch --depth=1 origin "$llvm_commit"
git -C "$source_dir" checkout --detach "$llvm_commit"

# LLVM 15 assumes non-Darwin platforms use GNU --gc-sections. iOS uses ld64
# and needs the Darwin -dead_strip spelling.
python3 - "$source_dir/llvm/cmake/modules/AddLLVM.cmake" <<'PY'
from pathlib import Path
import sys
p = Path(sys.argv[1])
s = p.read_text()
if 'MATCHES "Darwin|iOS"' not in s:
    changed = s.replace('MATCHES "Darwin"', 'MATCHES "Darwin|iOS"')
    if changed == s:
        raise SystemExit('AddLLVM.cmake Darwin condition not found')
    p.write_text(changed)
PY

if [[ ! -x "$host_build/bin/llvm-tblgen" ]]; then
    cmake -S "$source_dir/llvm" -B "$host_build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_ENABLE_PROJECTS= \
        -DLLVM_TARGETS_TO_BUILD= \
        -DLLVM_INCLUDE_TESTS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_BENCHMARKS=OFF
    cmake --build "$host_build" --parallel "$jobs" --target llvm-tblgen
fi

# Reconfigure on every run so a restored Codemagic cache picks up option
# changes. DXMT needs LLVM's static libraries and generated headers only;
# excluding tools prevents the iOS build from trying to link libLTO.dylib and
# LLVM executables with host/GNU linker flags such as `-z`.
cmake -S "$source_dir/llvm" -B "$ios_build" -G Ninja \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_TABLEGEN="$host_build/bin/llvm-tblgen" \
    -DLLVM_HOST_TRIPLE=arm64-apple-ios17.0 \
    -DLLVM_DEFAULT_TARGET_TRIPLE=arm64-apple-ios17.0 \
    -DLLVM_TARGET_ARCH=host \
    -DLLVM_TARGETS_TO_BUILD= \
    -DLLVM_ENABLE_PROJECTS= \
    -DLLVM_INCLUDE_TOOLS=OFF \
    -DLLVM_BUILD_TOOLS=OFF \
    -DLLVM_INCLUDE_UTILS=OFF \
    -DLLVM_BUILD_UTILS=OFF \
    -DLLVM_BUILD_LLVM_DYLIB=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_ENABLE_ZLIB=OFF \
    -DLLVM_ENABLE_ZSTD=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF

# DXMT's airconv meson.build records the output of
# `llvm-config --libs bitwriter passes`. Building LLVM's default `all` target
# also builds MCA, ORC JIT, ExecutionEngine, XRay, ObjCopy and iOS executables
# that the app never links; that exceeded Codemagic's job duration. Build only
# the static-library closure airconv names.
manifest="$root/build/llvm-ios/static-libs.txt"
targets=()
while IFS= read -r target; do
    [[ -n "$target" ]] && targets+=("$target")
done < "$manifest"
cmake --build "$ios_build" --parallel "$jobs" --target "${targets[@]}"
for target in "${targets[@]}"; do
    test -s "$ios_build/lib/lib$target.a" || { echo "Missing LLVM archive: lib$target.a" >&2; exit 1; }
done
echo "DXMT LLVM static-library closure ready (${#targets[@]} archives)"
