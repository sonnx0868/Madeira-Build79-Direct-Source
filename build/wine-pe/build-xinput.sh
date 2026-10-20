#!/bin/bash
# Refresh XInput guards after the matching native win32u capability is built.
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
tree="$root/wine/build-arm64ec"
tc="$root/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
dest="$root/app/Madeira/arm64ec-windows"
# refresh-dxmt-query runs in a child shell; its Homebrew PATH does not reach
# this script. Wine's resource grammar requires Bison 3, not macOS's Bison 2.3.
bison_bin="$(brew --prefix bison)/bin"
flex_bin="$(brew --prefix flex)/bin"
test -x "$bison_bin/bison"
test -x "$flex_bin/flex"
export PATH="$tc:$bison_bin:$flex_bin:$PATH"
bison --version
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
test -x "$tc/arm64ec-w64-mingw32-strip"
test -f "$tree/config.status"
modules=(xinput1_1 xinput1_2 xinput1_3 xinput1_4 xinput9_1_0)
targets=()
for module in "${modules[@]}"; do
    targets+=("dlls/$module/arm64ec-windows/$module.dll")
done
make -C "$tree" -j"$jobs" BISON="$bison_bin/bison" "${targets[@]}"
mkdir -p "$dest"
for target in "${targets[@]}"; do
    name="$(basename "$target")"
    cp "$tree/$target" "$dest/$name.tmp"
    "$tc/arm64ec-w64-mingw32-strip" --strip-debug "$dest/$name.tmp"
    mv -f "$dest/$name.tmp" "$dest/$name"
done
python3 "$root/tools/xinput-host-authority-bundle.py" --root "$root" --stamp
