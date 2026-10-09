#!/bin/bash
# GPL-3.0-or-later WITH the Madeira Converter Exception, version 1.
# Build Mesa's OSMesa with the Zink driver for iOS (arm64) -> libOSMesa.dylib,
# staged at app/Madeira/gl/. Desktop OpenGL 3.3+ for Madeira's winios GL driver
# (build/win32u-unix/opengl_ios.c, "zink" backend): OSMesa renders into an
# IOSurface-backed buffer, Zink turns GL into Vulkan, MoltenVK
# (build/moltenvk-ios/build.sh) turns Vulkan into Metal.
#
# Mesa 25.0.x on purpose: OSMesa was removed in 25.1.
# OSMesa insists on a software gallium driver being built; softpipe is the one
# without LLVM. At runtime GALLIUM_DRIVER=zink selects Zink through the same
# software-winsys helper.
#
# Source: research/mesa-25.0.7 (https://archive.mesa3d.org/mesa-25.0.7.tar.xz,
# sha256 592272df3cf01e85e7db300c449df5061092574d099da275d19e97ef0510f8a6),
# with build/mesa-ios/patches/*.patch applied. The patches modify Mesa and
# are offered under Mesa's licence (MIT), so they can go upstream as they are.
# Python deps (meson, mako, pyyaml, ninja) come from research/mesa-venv; bison
# and flex are the ones that ship with macOS.
set -eu
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
HERE="$R/build/mesa-ios"
SRC="${MESA_SRC:-$R/research/mesa-25.0.7}"
MVK="${MOLTENVK_SRC:-$R/research/madeira-moltenvk-v1.4.2}"
VENV="${MESA_VENV:-$R/research/mesa-venv}"
B="$HERE/obj"
OUT="$R/app/Madeira/gl"
SDK="$(xcrun --sdk iphoneos --show-sdk-path)"

if [ ! -f "$SRC/meson.build" ]; then
    archive="${TMPDIR:-/tmp}/madeira-mesa-25.0.7.tar.xz"
    curl --fail --location --retry 3 https://archive.mesa3d.org/mesa-25.0.7.tar.xz -o "$archive"
    printf '%s  %s\n' 592272df3cf01e85e7db300c449df5061092574d099da275d19e97ef0510f8a6 "$archive" | shasum -a 256 -c -
    mkdir -p "$(dirname "$SRC")"
    tar -xJf "$archive" -C "$(dirname "$SRC")"
fi
if [ ! -x "$VENV/bin/meson" ] || [ ! -x "$VENV/bin/ninja" ]; then
    python3 -m venv "$VENV"
    "$VENV/bin/pip" install meson==1.9.0 mako==1.3.10 pyyaml==6.0.2 packaging==25.0 ninja==1.13.0
fi
if [ ! -d "$MVK" ]; then
    echo "MoltenVK not found at $MVK; run build/moltenvk-ios/build.sh first" >&2
    exit 1
fi

export PATH="$VENV/bin:$PATH"

# Validate the patch contents on each run; a marker alone cannot detect
# a changed patch or an interrupted previous application.
for p in "$HERE"/patches/*.patch; do
    [ -e "$p" ] || continue
    if patch --batch --fuzz=0 -d "$SRC" -p1 --dry-run --reverse < "$p" >/dev/null 2>&1; then
        continue
    elif patch --batch --fuzz=0 -d "$SRC" -p1 --dry-run --forward < "$p" >/dev/null 2>&1; then
        patch --batch --fuzz=0 -d "$SRC" -p1 --forward < "$p"
    else
        echo "Mesa source does not match $p; use a clean pinned Mesa source tree" >&2
        exit 1
    fi
done

mkdir -p "$B"
sed "s|@SDK@|$SDK|g" "$HERE/ios-arm64.cross" > "$B/ios-arm64.cross"

setup_mode=""
[ ! -f "$B/build.ninja" ] || setup_mode="--reconfigure"
    meson setup $setup_mode "$B" "$SRC" --cross-file "$B/ios-arm64.cross" \
        --buildtype=release -Db_ndebug=true --default-library=shared \
        -Dplatforms= \
        -Dgallium-drivers=softpipe,zink \
        -Dvulkan-drivers= \
        -Dosmesa=true \
        -Dopengl=true -Dgles1=disabled -Dgles2=disabled \
        -Dglx=disabled -Degl=disabled -Dgbm=disabled \
        -Dllvm=disabled -Dshader-cache=enabled -Dxmlconfig=disabled \
        -Dzstd=disabled -Dlibunwind=disabled -Dvalgrind=disabled -Dlmsensors=disabled \
        -Dexpat=disabled -Dbuild-tests=false \
        -Dvideo-codecs= \
        -Dmoltenvk-dir="$MVK"
ninja -C "$B" -j"${JOBS:-4}"

mkdir -p "$OUT"
cp "$B/src/gallium/targets/osmesa/libOSMesa.8.dylib" "$OUT/libOSMesa.dylib"
install_name_tool -id @rpath/libOSMesa.dylib "$OUT/libOSMesa.dylib"
xcrun -sdk iphoneos strip -x "$OUT/libOSMesa.dylib"
cp "$SRC/docs/license.rst" "$R/app/Madeira/licenses/Mesa-license.rst"
echo "$(basename "$SRC")" > "$OUT/libOSMesa.version"
shasum -a 256 "$OUT/libOSMesa.dylib" "$OUT/libMoltenVK.dylib" | shasum -a 256 | awk '{print $1}' > "$OUT/backend.version"
ls -l "$OUT"
