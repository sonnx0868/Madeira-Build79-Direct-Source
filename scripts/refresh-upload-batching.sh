#!/bin/bash
# Rebuild D3D11's command scheduler, preserving the native Metal/shader archive.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/dxmt-pe/upload-batching-refresh"
mkdir -p "$out"
python3 "$root/tests/host/check-upload-batching.py"
python3 "$root/tests/host/check-upload-batching-receipts.py"
python3 "$root/tools/dxmt-upload-batching-bundle.py" --root "$root" --prepare "$out/prepared.json"
export HOMEBREW_NO_AUTO_UPDATE=1
formulae=()
for formula in ninja meson bison flex pkg-config; do
  brew list "$formula" >/dev/null 2>&1 || formulae+=("$formula")
done
if (( ${#formulae[@]} )); then brew install "${formulae[@]}"; fi
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
toolchain=llvm-mingw-20260421-ucrt-macos-universal
if [[ ! -x "$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-clang++" ]]; then
  download="${CM_TEMP_DIR:?}/$toolchain.tar.xz"
  curl --fail --location --retry 3 "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$toolchain.tar.xz" --output "$download"
  printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$download" | shasum -a 256 -c -
  mkdir -p "$root/toolchains"
  tar -xJf "$download" -C "$root/toolchains"
fi
export PATH="$root/toolchains/$toolchain/bin:$PATH"
python3 "$root/tests/host/check-swapchain-hook-abi.py" \
  --compiler "$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-clang++" --require-tools
wine_build="$root/wine/build-arm64ec"
(cd "$wine_build" && bash ./config.status)
make -C "$wine_build" -j"${JOBS:-4}" tools/winebuild/all tools/widl/all \
  libs/winecrt0/arm64ec-windows/libwinecrt0.a \
  dlls/ntdll/arm64ec-windows/libntdll.a dlls/dbghelp/arm64ec-windows/libdbghelp.a
python3 - "$root" "$out" <<'PY'
from pathlib import Path
import sys
root, out = map(Path, sys.argv[1:])
(out / 'cross.ini').write_text((root / 'dxmt/build-arm64ec-win.txt').read_text().replace('@GLOBAL_SOURCE_ROOT@', root.as_posix()))
(out / 'native.ini').write_text("[binaries]\nc = '/usr/bin/clang'\ncpp = '/usr/bin/clang++'\n")
PY
# A fresh dedicated build directory makes a restored pre-overlay object unable
# to satisfy the target. Existing receipt producers are never blindly restamped.
test ! -e "$out/pe/build.ninja" || { echo 'Upload refresh needs a fresh build directory' >&2; exit 1; }
meson setup --cross-file "$out/cross.ini" --native-file "$out/native.ini" --buildtype release \
  -Dwine_build_path="$wine_build" -Dmetal_std=metal3.1 "$out/pe" "$root/dxmt"
ninja -C "$out/pe" -j"${JOBS:-4}" src/d3d11/d3d11.dll
dll="$root/app/Madeira/arm64ec-windows/d3d11.dll"
cp "$dll" "$out/d3d11-base.dll"
cp "$out/pe/src/d3d11/d3d11.dll" "$dll"
"$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-strip" --strip-debug "$dll"
python3 "$root/tools/dxmt-upload-batching-bundle.py" --root "$root" --stamp --base "$out/prepared.json"
python3 "$root/tools/dxmt-upload-batching-bundle.py" --root "$root"
echo 'Refreshed ARM64EC D3D11 buffer-upload scheduling; native renderer, D3D9, shader compiler and other Windows DLLs preserved'
