#!/bin/bash
# Build the missing i386 DLL farm from this checkout. Native 64-bit libraries
# and the tracked ARM64 WoW64/FEX modules remain unchanged in this addon.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:?Bitrise artifact directory required}"
export HOMEBREW_NO_AUTO_UPDATE=1
formulae=()
for formula in cmake ninja meson bison flex pkg-config; do
  brew list "$formula" >/dev/null 2>&1 || formulae+=("$formula")
done
if (( ${#formulae[@]} )); then brew install "${formulae[@]}"; fi
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
toolchain=llvm-mingw-20260421-ucrt-macos-universal
if [[ ! -x "$root/toolchains/$toolchain/bin/i686-w64-mingw32-clang" ]]; then
  archive="${CM_TEMP_DIR:?}/$toolchain.tar.xz"
  curl --fail --location --retry 3 \
    "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$toolchain.tar.xz" --output "$archive"
  printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$archive" | shasum -a 256 -c -
  mkdir -p "$root/toolchains"
  tar -xJf "$archive" -C "$root/toolchains"
fi
export PATH="$root/toolchains/$toolchain/bin:$PATH"
bash "$root/build/wine-i386/build.sh"
python3 "$root/tools/check-wow64-addon.py" --root "$root" --write "$out/wow64-addon-receipt.json"
cp "$out/wow64-addon-receipt.json" "$root/build/wine-i386/addon-receipt.json"
(cd "$root" && zip -qry "$out/madeira-wow64-addon.zip" app/Madeira/i386-windows build/wine-i386/addon-receipt.json)
shasum -a 256 "$out/madeira-wow64-addon.zip" > "$out/wow64-addon-sha256.txt"
echo 'WoW64 addon built and import closure checked. No device/game execution has been performed.'
