#!/bin/bash
# Stage the official LÖVE 11.5 Windows x64 LuaJIT runtime. This build uses
# GC64, so Lua objects do not require iOS's permanently unavailable low 2 GB.
set -euo pipefail

root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
out="$root/app/Madeira/arm64ec-windows/lua51-gc64.dll"
want_dll="94fb3e3d4b1f6acce0110e46aadf1ecab1fa17c4ed1ab34caef3c5c2121c208e"
if [[ -s "$out" ]] && [[ "$(shasum -a 256 "$out" | awk '{print $1}')" == "$want_dll" ]]; then
    echo "LÖVE 11.5 LuaJIT GC64 already staged."
    exit 0
fi

archive="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/love-11.5-win64.zip"
url="https://github.com/love2d/love/releases/download/11.5/love-11.5-win64.zip"
curl --fail --location --retry 3 "$url" --output "$archive"
printf '%s  %s\n' ba6e56be2685e53c817749c4a5007f51137136fe5a3ab64920508babc2e74369 "$archive" | shasum -a 256 -c -

mkdir -p "$(dirname "$out")"
tmp="$out.tmp"
unzip -p "$archive" love-11.5-win64/lua51.dll > "$tmp"
printf '%s  %s\n' "$want_dll" "$tmp" | shasum -a 256 -c -
python3 - "$tmp" <<'PY'
import struct, sys
d = open(sys.argv[1], 'rb').read(512)
assert d[:2] == b'MZ'
pe = struct.unpack_from('<I', d, 0x3c)[0]
assert d[pe:pe+4] == b'PE\0\0'
assert struct.unpack_from('<H', d, pe + 4)[0] == 0x8664
PY
mv -f "$tmp" "$out"
echo "Staged official LÖVE 11.5 GC64 lua51.dll ($(wc -c < "$out" | tr -d ' ') bytes)."
