#!/bin/bash
# Rebuild every ARM64EC consumer of Wine's in-tree vkd3d-shader compiler.
# ANGLE's D3DCompile2VKD3D route may be resolved from wined3d.dll, so staging
# only d3dcompiler_47.dll leaves the old E5017 compiler inside the app.
set -euo pipefail
R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TC="$R/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
export PATH="$TC:$PATH"
B="$R/wine/build-arm64ec"
DEST="$R/app/Madeira/arm64ec-windows"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"

if [ ! -f "$B/config.status" ]; then
    mkdir -p "$B"
    (cd "$B" && ../configure --enable-archs=arm64ec --without-x --disable-tests --enable-winegstreamer)
fi

# Rebuild the archive first so both DLL link steps consume the patched HLSL
# code instead of a cached pre-patch libvkd3d-shader.a.
make -C "$B/libs/vkd3d" -j"$JOBS"
make -C "$B/dlls/d3dcompiler_47" -j"$JOBS"
make -C "$B/dlls/wined3d" -j"$JOBS"

mkdir -p "$DEST"
for module in d3dcompiler_47 wined3d; do
    src="$B/dlls/$module/arm64ec-windows/$module.dll"
    out="$DEST/$module.dll"
    test -s "$src"
    if grep -a -q 'Flattening conditional blocks with non-discard jump instructions' "$src"; then
        echo "INVALID: $src still embeds the pre-fix E5017 compiler" >&2
        exit 1
    fi
    cp "$src" "$out"
    printf '  %-20s %s bytes\n' "$module.dll" "$(wc -c < "$out" | tr -d ' ')"
done
