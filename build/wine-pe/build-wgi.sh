#!/bin/bash
# Refresh the PE adapter only; its host snapshot syscall ABI already exists.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
tree="$root/wine/build-arm64ec"
tc="$root/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
export PATH="$tc:$PATH"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
make -C "$tree" -j"$jobs" dlls/windows.gaming.input/arm64ec-windows/windows.gaming.input.dll
dest="$root/app/Madeira/arm64ec-windows/windows.gaming.input.dll"
cp "$tree/dlls/windows.gaming.input/arm64ec-windows/windows.gaming.input.dll" "$dest.tmp"
"$tc/arm64ec-w64-mingw32-strip" --strip-debug "$dest.tmp"
mv -f "$dest.tmp" "$dest"
python3 "$root/tools/wgi-gamepad-bundle.py" --root "$root" --stamp
