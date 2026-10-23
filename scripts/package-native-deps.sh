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
  build/wine-pe/wgi-host-gamepad.json \
  build/dxmt-ios/frameflow.json \
  build/dxmt-ios/retirement.json \
  build/dxmt-ios/present-resource.json \
  build/dxmt-pe/depth-retention.json \
  build/dxmt-pe/upload-batching.json \
  build/wine-pe/xinput-host-authority.json \
  build/win32u-unix/input-wake.json \
  build/wine-pe/exec-callback-pe.version \
  build/wine-pe/exec-callback-pe.json \
  build/wine-pe/module-loader.json \
  build/wine-pe/opengl-pe.version \
  build/ntdll-unix/lrcpc2-alias.json \
  build/ntdll-unix/runtime-scheduling-loader.json \
  build/ntdll-unix/game-graphics-preferences.json \
  build/ntdll-unix/fex-alias-policy.json \
  build/madsync/native-receipt.json \
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
  app/Madeira/arm64ec-windows/windows.gaming.input.dll \
  app/Madeira/arm64ec-windows/lua51-gc64.dll \
  app/Madeira/arm64ec-windows/libEGL.dll \
  app/Madeira/arm64ec-windows/libGLESv2.dll \
  app/Madeira/arm64ec-windows/gdiplus.dll \
  app/Madeira/arm64ec-windows/mlang.dll \
  app/Madeira/arm64ec-windows/sspicli.dll \
  app/Madeira/arm64ec-windows/opengl32.dll \
  app/Madeira/arm64ec-windows/glu32.dll \
  app/Madeira/aarch64-windows/opengl32.dll \
  app/Madeira/aarch64-windows/glu32.dll \
  app/Madeira/gl \
  app/Madeira/arm64ec-windows/ntdll.dll \
  app/Madeira/arm64ec-windows/kernelbase.dll \
  app/Madeira/arm64ec-windows/xaudio2_8.dll \
  app/Madeira/arm64ec-windows/xaudio2_9.dll \
  app/Madeira/arm64ec-windows/wintypes.dll \
  app/Madeira/arm64ec-windows/d3dcompiler_47.dll \
  app/Madeira/arm64ec-windows/wined3d.dll \
  app/Madeira/arm64ec-windows/d3d11.dll \
  app/Madeira/arm64ec-windows/dxgi.dll \
  app/Madeira/arm64ec-windows/d3d9.dll \
  app/Madeira/arm64ec-windows/winemetal.dll \
  app/Madeira/arm64ec-windows/dockhost.exe \
  app/Madeira/arm64ec-windows/xtajit64.dll \
  app/Madeira/translation-lab \
  app/Madeira/aarch64-windows/madeira-session-host.exe \
  app/Madeira/aarch64-windows/ntdll.dll \
  app/Madeira/aarch64-windows/kernelbase.dll \
  app/Madeira/arm64ec-windows/dock-notices.txt \
  app/Madeira/licenses/ANGLE-THIRD-PARTY-NOTICES.html \
  app/Madeira/licenses/Mesa-license.rst \
  app/Madeira/licenses/MoltenVK-Apache-2.0.txt \
  app/Madeira/licenses/SPIRV-Cross-Apache-2.0.txt \
  app/Madeira/licenses/SPIRV-Tools-Apache-2.0.txt \
  app/Madeira/licenses/cereal-BSD-3-Clause.txt)
if [[ -f "$root/app/Madeira/i386-windows/ntdll.dll" ]]; then
  test -s "$root/build/wine-i386/addon-receipt.json"
  (cd "$root" && zip -qry "$output" app/Madeira/i386-windows build/wine-i386/addon-receipt.json)
fi
if [[ -f "$root/build/dxmt-ios/producer-pacing.json" ]]; then
  (cd "$root" && zip -q "$output" build/dxmt-ios/producer-pacing.json)
fi
if [[ -f "$root/build/dxmt-ios/shader-vector-load.json" ]]; then
  (cd "$root" && zip -q "$output" build/dxmt-ios/shader-vector-load.json)
fi
if [[ -f "$root/build/dxmt-ios/shader-optimizer.json" ]]; then
  (cd "$root" && zip -q "$output" build/dxmt-ios/shader-optimizer.json)
fi
test -s "$root/build/dxmt-ios/texture-residency.json"
(cd "$root" && zip -q "$output" build/dxmt-ios/texture-residency.json)
if [[ -f "$root/build/dxmt-ios/residency-drain.json" ]]; then
  (cd "$root" && zip -q "$output" build/dxmt-ios/residency-drain.json)
fi
if [[ -f "$root/build/dxmt-ios/padded-lighting.json" ]]; then
  (cd "$root" && zip -q "$output" build/dxmt-ios/padded-lighting.json)
fi
if [[ -f "$root/build/dxmt-pe/query-park.json" ]]; then
  (cd "$root" && zip -q "$output" build/dxmt-pe/query-park.json)
fi
test -s "$root/build/dxmt-pe/color-views.json"
(cd "$root" && zip -q "$output" build/dxmt-pe/color-views.json)
test -s "$root/build/wine-pe/stackwalk-alias.json"
(cd "$root" && zip -q "$output" build/wine-pe/stackwalk-alias.json)
test -s "$root/build/wine-pe/native-callback-context.json"
(cd "$root" && zip -q "$output" build/wine-pe/native-callback-context.json)
test -s "$root/build/dxmt-ios/gpu-pipeline.json"
(cd "$root" && zip -q "$output" build/dxmt-ios/gpu-pipeline.json)
test -s "$root/build/dxmt-pe/swapchain-hooks.json"
(cd "$root" && zip -q "$output" build/dxmt-pe/swapchain-hooks.json)
if [[ -f "$root/build/full-native-rebuild.json" ]]; then
  (cd "$root" && zip -q "$output" build/full-native-rebuild.json build/full-native-start.json)
  # A cold receipt binds every shipped native payload, including rebuilt
  # tracked libraries (GnuTLS/GMP) that older incremental ZIP lists omitted.
  python3 - "$root" "$output" <<'PY'
from pathlib import Path
import hashlib, json, sys, zipfile
root, output = Path(sys.argv[1]), Path(sys.argv[2])
payloads = json.loads((root / 'build/full-native-rebuild.json').read_text())['payloads']
added = 0
with zipfile.ZipFile(output, 'a', compression=zipfile.ZIP_DEFLATED) as archive:
    included = set(archive.namelist())
    for name, expected in payloads.items():
        path = root / name
        assert path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest() == expected, name
        if name not in included:
            archive.write(path, name)
            added += 1
print('PASS: full native package covers all', len(payloads), 'bound payloads; added', added)
PY
fi

echo "Created $output"
shasum -a 256 "$output"
