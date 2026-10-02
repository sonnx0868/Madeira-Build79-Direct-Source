#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
output="${1:-$root/madeira-upstream-native-deps.zip}"
bash "$root/scripts/check-ios-build.sh"
rm -f "$output"
(cd "$root" && zip -qry "$output" \
  FEX/build-ios \
  wine/build-macos/include wine/build-macos/config.status \
  wine/build-arm64ec/include wine/build-arm64ec/config.status \
  app/Madeira/libwineserver.a app/Madeira/libntdll_unix.a \
  app/Madeira/libwin32u_unix.a app/Madeira/libdxmt_combined.a \
  app/Madeira/libavformat.a app/Madeira/libavcodec.a \
  app/Madeira/libswresample.a app/Madeira/libavutil.a \
  app/Madeira/x86_64-vcruntime \
  app/Madeira/arm64ec-windows/dockhost.exe \
  app/Madeira/arm64ec-windows/dock-notices.txt)
echo "Created $output"
shasum -a 256 "$output"
