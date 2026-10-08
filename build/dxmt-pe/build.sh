#!/bin/bash
# Ship the compiler scheduling change in the ARM64EC frontends, not only source.
set -euo pipefail
root="$(cd "$(dirname "$0")/../.." && pwd)"
build="$root/dxmt/build-arm64ec"
tc="$root/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
export PATH="$tc:$PATH"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"
make -C "$root/wine/build-arm64ec" -j"$jobs" \
    libs/winecrt0/arm64ec-windows/libwinecrt0.a \
    dlls/ntdll/arm64ec-windows/libntdll.a \
    dlls/dbghelp/arm64ec-windows/libdbghelp.a
# Resolve source-root placeholders explicitly; the top-level toolchain is not
# inside the DXMT submodule. Meson's cross files must contain absolute paths.
python3 - "$root" "$root/build/dxmt-pe/cross.ini" <<'PY'
from pathlib import Path
import sys
root = Path(sys.argv[1])
text = (root / "dxmt/build-arm64ec-win.txt").read_text()
Path(sys.argv[2]).write_text(text.replace("@GLOBAL_SOURCE_ROOT@", root.as_posix()))
PY
options=(--buildtype=release -Dwine_build_path="$root/wine/build-arm64ec" -Dmetal_std=metal3.1)
if [[ -f "$build/meson-private/coredata.dat" ]]; then
    meson setup --reconfigure "${options[@]}" "$build" "$root/dxmt"
else
    meson setup --cross-file "$root/build/dxmt-pe/cross.ini" "${options[@]}" "$build" "$root/dxmt"
fi
ninja -C "$build" -j"$jobs" src/d3d11/d3d11.dll src/dxgi/dxgi.dll src/d3d9/d3d9.dll src/winemetal/winemetal.dll
for item in d3d11/d3d11 dxgi/dxgi d3d9/d3d9 winemetal/winemetal; do
    dll="${item##*/}.dll"
    cp "$build/src/$item.dll" "$root/app/Madeira/arm64ec-windows/$dll"
    "$tc/arm64ec-w64-mingw32-strip" --strip-debug "$root/app/Madeira/arm64ec-windows/$dll"
done
grep -a -q 'compiler-workers=v1' "$root/app/Madeira/arm64ec-windows/d3d11.dll"
python3 "$root/tools/patch-dxmt-query-log.py" --check
