#!/bin/bash
# Refresh XInput guards after the matching native win32u capability is built.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
tree="$root/wine/build-arm64ec"
tc="$root/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
dest="$root/app/Madeira/arm64ec-windows"
export PATH="$tc:$PATH"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
test -x "$tc/arm64ec-w64-mingw32-strip"
test -f "$tree/config.status"
modules=(xinput1_1 xinput1_2 xinput1_3 xinput1_4 xinput9_1_0)
targets=()
for module in "${modules[@]}"; do
    targets+=("dlls/$module/arm64ec-windows/$module.dll")
done
make -C "$tree" -j"$jobs" "${targets[@]}"
mkdir -p "$dest"
for target in "${targets[@]}"; do
    name="$(basename "$target")"
    cp "$tree/$target" "$dest/$name.tmp"
    "$tc/arm64ec-w64-mingw32-strip" --strip-debug "$dest/$name.tmp"
    mv -f "$dest/$name.tmp" "$dest/$name"
done
python3 "$root/tools/xinput-host-authority-bundle.py" --root "$root" --stamp
