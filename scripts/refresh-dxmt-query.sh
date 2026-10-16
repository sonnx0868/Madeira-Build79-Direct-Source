#!/bin/bash
# Rebuild only the ARM64EC D3D11 frontend; keep the native renderer and DLL farm.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
export HOMEBREW_NO_AUTO_UPDATE=1
formulae=()
for formula in ninja meson bison flex pkg-config; do
  brew list "$formula" >/dev/null 2>&1 || formulae+=("$formula")
done
if (( ${#formulae[@]} )); then brew install "${formulae[@]}"; fi
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
toolchain=llvm-mingw-20260421-ucrt-macos-universal
if [[ ! -x "$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-clang++" ]]; then
  archive="${CM_TEMP_DIR:?}/$toolchain.tar.xz"
  curl --fail --location --retry 3 "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$toolchain.tar.xz" --output "$archive"
  printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$archive" | shasum -a 256 -c -
  mkdir -p "$root/toolchains"
  tar -xJf "$archive" -C "$root/toolchains"
fi
export PATH="$root/toolchains/$toolchain/bin:$PATH"
python3 "$root/tests/host/check-swapchain-hook-abi.py" \
  --compiler "$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-clang++" --require-tools
wine_build="$root/wine/build-arm64ec"
(cd "$wine_build" && bash ./config.status)
make -C "$wine_build" -j"${JOBS:-4}" tools/winebuild/all tools/widl/all \
  libs/winecrt0/arm64ec-windows/libwinecrt0.a \
  dlls/ntdll/arm64ec-windows/libntdll.a dlls/dbghelp/arm64ec-windows/libdbghelp.a
out="$root/build/dxmt-pe/query-refresh"
mkdir -p "$out"
python3 - "$root" "$out" <<'PY'
from pathlib import Path
import sys
r,o=map(Path,sys.argv[1:])
(o/'cross.ini').write_text((r/'dxmt/build-arm64ec-win.txt').read_text().replace('@GLOBAL_SOURCE_ROOT@',r.as_posix()))
(o/'native.ini').write_text("[binaries]\nc = '/usr/bin/clang'\ncpp = '/usr/bin/clang++'\n")
PY
meson setup --cross-file "$out/cross.ini" --native-file "$out/native.ini" --buildtype release \
  -Dwine_build_path="$wine_build" -Dmetal_std=metal3.1 "$out/pe" "$root/dxmt"
ninja -C "$out/pe" -j"${JOBS:-4}" src/d3d11/d3d11.dll
original="$root/app/Madeira/arm64ec-windows/d3d11.dll"
cp "$original" "$out/d3d11-base.dll"
cp "$out/pe/src/d3d11/d3d11.dll" "$original"
"$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-strip" --strip-debug "$original"
python3 "$root/tools/dxmt-query-bundle.py" --root "$root" --stamp --base "$out/d3d11-base.dll"
python3 "$root/tools/dxmt-swapchain-bundle.py" --root "$root" --stamp --base "$out/d3d11-base.dll"
echo 'Refreshed ARM64EC d3d11.dll GPU event polling; native renderer and other DLLs unchanged'
