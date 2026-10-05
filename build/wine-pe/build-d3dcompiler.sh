#!/bin/bash
# Rebuild Wine's ARM64EC d3dcompiler_47.dll, including the in-tree
# vkd3d-shader HLSL compiler used by ANGLE/OpenGL folder games.
set -eu
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TC="$R/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
export PATH="$TC:$PATH"
B="$R/wine/build-arm64ec"

if [ ! -f "$B/config.status" ]; then
    mkdir -p "$B"
    (cd "$B" && ../configure --enable-archs=arm64ec --without-x --disable-tests --enable-winegstreamer)
fi

make -C "$B/dlls/d3dcompiler_47"
src="$B/dlls/d3dcompiler_47/arm64ec-windows/d3dcompiler_47.dll"
out="$R/app/Madeira/arm64ec-windows/d3dcompiler_47.dll"
test -s "$src"
cp "$src" "$out"
ls -l "$out"
