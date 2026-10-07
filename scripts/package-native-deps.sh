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
  build/wine-pe/controller-pe.version \
  app/Madeira/libwineserver.a app/Madeira/libntdll_unix.a \
  app/Madeira/libwin32u_unix.a app/Madeira/libdxmt_combined.a \
  app/Madeira/libavformat.a app/Madeira/libavcodec.a \
  app/Madeira/libswresample.a app/Madeira/libavutil.a \
  app/Madeira/libmadeira_rppairing.a \
  app/Madeira/x86_64-vcruntime \
  app/Madeira/arm64ec-windows/win32u.dll \
  app/Madeira/arm64ec-windows/user32.dll \
  app/Madeira/arm64ec-windows/dinput.dll \
  app/Madeira/arm64ec-windows/dinput8.dll \
  app/Madeira/arm64ec-windows/xinput1_1.dll \
  app/Madeira/arm64ec-windows/xinput1_2.dll \
  app/Madeira/arm64ec-windows/xinput1_3.dll \
  app/Madeira/arm64ec-windows/xinput1_4.dll \
  app/Madeira/arm64ec-windows/xinput9_1_0.dll \
  app/Madeira/arm64ec-windows/lua51-gc64.dll \
  app/Madeira/arm64ec-windows/libEGL.dll \
  app/Madeira/arm64ec-windows/libGLESv2.dll \
  app/Madeira/arm64ec-windows/wintypes.dll \
  app/Madeira/arm64ec-windows/d3dcompiler_47.dll \
  app/Madeira/arm64ec-windows/wined3d.dll \
  app/Madeira/arm64ec-windows/dockhost.exe \
  app/Madeira/arm64ec-windows/madeira-session-host.exe \
  app/Madeira/arm64ec-windows/dock-notices.txt \
  app/Madeira/licenses/ANGLE-THIRD-PARTY-NOTICES.html)
echo "Created $output"
shasum -a 256 "$output"
