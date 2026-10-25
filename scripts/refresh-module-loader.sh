#!/bin/bash
# Rebuild only the three PE ntdll loaders and the headless Steam host.
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
if [[ ! -x "$root/toolchains/$toolchain/bin/i686-w64-mingw32-clang" ]]; then
  archive="${CM_TEMP_DIR:?}/$toolchain.tar.xz"
  curl --fail --location --retry 3 "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$toolchain.tar.xz" --output "$archive"
  printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$archive" | shasum -a 256 -c -
  mkdir -p "$root/toolchains"
  tar -xJf "$archive" -C "$root/toolchains"
fi
export PATH="$root/toolchains/$toolchain/bin:$PATH"
# A cached archive carries headers and config.status, not a generated Makefile.
for tree in build-arm64ec build-macos; do
  (cd "$root/wine/$tree" && bash ./config.status)
done
bash "$root/build/wine-pe/build-ntdll.sh"
WINE_PE_ARCH=aarch64 bash "$root/build/wine-pe/build-ntdll.sh"
WINE_PE_ARCH=aarch64 bash "$root/build/wine-pe/build-modules.sh" wow64
SKIP_DXMT=1 bash "$root/build/wine-i386/build.sh" ntdll
python3 "$root/tools/check-wow64-addon.py" --root "$root" --write "$root/build/wine-i386/addon-receipt.json"
bash "$root/build/madeira-dock/build.sh"
python3 "$root/tools/loader-bundle.py" --root "$root" --stamp
