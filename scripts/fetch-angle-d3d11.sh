#!/bin/bash
# Stage ANGLE's x64 EGL / OpenGL ES runtime. SDL2 can load these DLLs directly;
# ANGLE then emits D3D11, which Madeira's DXMT backend translates to Metal.
set -euo pipefail

root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
out="$root/app/Madeira/arm64ec-windows"
licenses="$root/app/Madeira/licenses"
egl="$out/libEGL.dll"
gles="$out/libGLESv2.dll"
notices="$licenses/ANGLE-THIRD-PARTY-NOTICES.html"

# Electron 28.1.0 / Chromium 120 carries ANGLE 2.1.22152 at 4ae5f681dfe6.
# The official Electron Windows archive is used only as a reproducible upstream
# binary source; no Electron executable or runtime is shipped by Madeira.
version="28.1.0"
archive_name="electron-v${version}-win32-x64.zip"
archive_sha="6ab17fa3ec537e7d9a5e6483f321a0634333c30a6fdfaec1cc8fd07953cbff89"
egl_sha="d9c8541eaf0293c67ece10e97d00a8b689d5e043a8356d43224aac1af3a21a5f"
gles_sha="a1275f57b47575db9aa3a577e5eacba1d7f1d5578ef6a8072468d788c93c85ff"
notices_sha="000ae5775ffa701d57afe7ac3831b76799e8250a2d0c328d1785cba935aab38d"

hash_is() {
    [[ -s "$2" ]] && [[ "$(sha256_file "$2")" == "$1" ]]
}
sha256_file() {
    if command -v shasum >/dev/null 2>&1; then shasum -a 256 "$1" | awk '{print $1}'
    else sha256sum "$1" | awk '{print $1}'
    fi
}
require_hash() {
    local expected="$1" path="$2"
    [[ "$(sha256_file "$path")" == "$expected" ]] || {
        echo "SHA-256 mismatch: $path" >&2
        return 1
    }
}
if hash_is "$egl_sha" "$egl" && hash_is "$gles_sha" "$gles" && hash_is "$notices_sha" "$notices"; then
    echo "ANGLE D3D11 EGL/GLES runtime already staged."
    exit 0
fi

archive="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/$archive_name"
url="https://github.com/electron/electron/releases/download/v${version}/$archive_name"
curl --fail --location --retry 3 "$url" --output "$archive"
require_hash "$archive_sha" "$archive"

mkdir -p "$out" "$licenses"
tmpdir="$(mktemp -d "${TMPDIR:-/tmp}/madeira-angle.XXXXXX")"
trap 'rm -rf "$tmpdir"' EXIT
unzip -p "$archive" libEGL.dll > "$tmpdir/libEGL.dll"
unzip -p "$archive" libGLESv2.dll > "$tmpdir/libGLESv2.dll"
unzip -p "$archive" LICENSES.chromium.html > "$tmpdir/ANGLE-THIRD-PARTY-NOTICES.html"

require_hash "$egl_sha" "$tmpdir/libEGL.dll"
require_hash "$gles_sha" "$tmpdir/libGLESv2.dll"
require_hash "$notices_sha" "$tmpdir/ANGLE-THIRD-PARTY-NOTICES.html"
python_cmd="$(command -v python3 || command -v python)"
"$python_cmd" - "$tmpdir/libEGL.dll" "$tmpdir/libGLESv2.dll" <<'PY'
import struct, sys
for path in sys.argv[1:]:
    data = open(path, "rb").read(512)
    assert data[:2] == b"MZ", path
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    assert data[pe:pe + 4] == b"PE\0\0", path
    assert struct.unpack_from("<H", data, pe + 4)[0] == 0x8664, path
PY
grep -a -q 'eglGetPlatformDisplayEXT' "$tmpdir/libEGL.dll"
grep -a -q 'D3D11CreateDevice' "$tmpdir/libGLESv2.dll"

mv -f "$tmpdir/libEGL.dll" "$egl"
mv -f "$tmpdir/libGLESv2.dll" "$gles"
mv -f "$tmpdir/ANGLE-THIRD-PARTY-NOTICES.html" "$notices"
echo "Staged ANGLE x64 EGL/GLES D3D11 runtime ($(wc -c < "$gles" | tr -d ' ') byte libGLESv2.dll)."
