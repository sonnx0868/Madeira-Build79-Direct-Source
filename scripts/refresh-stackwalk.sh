#!/bin/bash
# Rebuild the one ARM64EC ntdll that owns RtlWalkFrameChain.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
export HOMEBREW_NO_AUTO_UPDATE=1
formulae=()
for formula in bison flex pkg-config; do
  brew list "$formula" >/dev/null 2>&1 || formulae+=("$formula")
done
if (( ${#formulae[@]} )); then brew install "${formulae[@]}"; fi
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
toolchain=llvm-mingw-20260421-ucrt-macos-universal
if [[ ! -x "$root/toolchains/$toolchain/bin/arm64ec-w64-mingw32-clang" ]]; then
  archive="${CM_TEMP_DIR:?}/$toolchain.tar.xz"
  curl --fail --location --retry 3 "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$toolchain.tar.xz" --output "$archive"
  printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$archive" | shasum -a 256 -c -
  mkdir -p "$root/toolchains"
  tar -xJf "$archive" -C "$root/toolchains"
fi
export PATH="$root/toolchains/$toolchain/bin:$PATH"
(cd "$root/wine/build-arm64ec" && bash ./config.status)
out="$root/build/wine-pe/stackwalk-refresh"
mkdir -p "$out"
cp "$root/app/Madeira/arm64ec-windows/ntdll.dll" "$out/base-ntdll.dll"
bash "$root/build/wine-pe/build-ntdll.sh"
python3 "$root/tools/ntdll-stackwalk-bundle.py" --root "$root" --stamp --base "$out/base-ntdll.dll"
if ! python3 "$root/tools/wgi-gamepad-bundle.py" --root "$root" >/dev/null 2>&1; then
    bash "$root/build/wine-pe/build-wgi.sh"
fi
echo 'Refreshed only ARM64EC ntdll stack-walk alias handling; other loaders and native libraries preserved'
