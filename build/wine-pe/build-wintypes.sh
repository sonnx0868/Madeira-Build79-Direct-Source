#!/bin/bash
# Build/stage the ARM64EC WinRT host used by modern Unity IL2CPP imports.
set -eu
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TC="$R/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
export PATH="$TC:$PATH"
B="$R/wine/build-arm64ec"

if [ ! -f "$B/config.status" ]; then
    mkdir -p "$B"
    (cd "$B" && ../configure --enable-archs=arm64ec --without-x --disable-tests --enable-winegstreamer)
fi

make -C "$B/dlls/wintypes"
src="$B/dlls/wintypes/arm64ec-windows/wintypes.dll"
out="$R/app/Madeira/arm64ec-windows/wintypes.dll"
test -s "$src"
cp "$src" "$out"
ls -l "$out"
