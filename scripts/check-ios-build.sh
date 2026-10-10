#!/bin/bash
set -eu
root="$(cd "$(dirname "$0")/.." && pwd)"
missing=0
need() { if [[ ! -s "$root/$1" ]]; then echo "MISSING: $1"; missing=1; fi; }

python3 "$root/tools/patch-dxmt-query-log.py" --check || missing=1

for path in \
  FEX/build-ios/FEXCore/Source/libFEXCore.a \
  FEX/build-ios/FEXCore/Source/libFEXCore_Base.a \
  FEX/build-ios/FEXCore/Source/libJemallocLibs.a \
  FEX/build-ios/External/fmt/libfmt.a \
  FEX/build-ios/External/cephes/libcephes_128bit.a \
  FEX/build-ios/External/xxhash/cmake_unofficial/libxxhash.a \
  FEX/build-ios/External/SoftFloat-3e/libsoftfloat_3e.a \
  app/Madeira/libwineserver.a app/Madeira/libntdll_unix.a \
  app/Madeira/libwin32u_unix.a app/Madeira/libdxmt_combined.a \
  app/Madeira/libavformat.a app/Madeira/libavcodec.a \
  app/Madeira/libswresample.a app/Madeira/libavutil.a \
  app/Madeira/libmadeira_rppairing.a \
  app/Frameworks/StikJIT.xcframework/ios-arm64/StikJIT.framework/StikJIT \
  "app/Madeira/Madeira JIT.shortcut" \
  app/Madeira/arm64ec-windows/wintypes.dll \
  app/Madeira/arm64ec-windows/d3dcompiler_47.dll \
  app/Madeira/arm64ec-windows/wined3d.dll \
  app/Madeira/arm64ec-windows/gdiplus.dll \
  app/Madeira/arm64ec-windows/mlang.dll \
  app/Madeira/arm64ec-windows/sspicli.dll \
  app/Madeira/arm64ec-windows/opengl32.dll \
  app/Madeira/arm64ec-windows/glu32.dll \
  app/Madeira/gl/libOSMesa.dylib \
  app/Madeira/gl/libMoltenVK.dylib \
  app/Madeira/gl/backend.version \
  app/Madeira/arm64ec-windows/dockhost.exe; do need "$path"; done

for dll in win32u.dll user32.dll dinput.dll dinput8.dll \
  xinput1_1.dll xinput1_2.dll xinput1_3.dll xinput1_4.dll \
  xinput9_1_0.dll; do need "app/Madeira/arm64ec-windows/$dll"; done
need "app/Madeira/arm64ec-windows/lua51-gc64.dll"
need "app/Madeira/aarch64-windows/madeira-session-host.exe"
if [[ -s "$root/app/Madeira/aarch64-windows/madeira-session-host.exe" ]]; then
  python3 "$root/tests/host/check-runtime-host-arch.py" || missing=1
fi
need "app/Madeira/arm64ec-windows/libEGL.dll"
need "app/Madeira/arm64ec-windows/libGLESv2.dll"
need "app/Madeira/licenses/ANGLE-BSD-3.txt"
need "app/Madeira/licenses/ANGLE-THIRD-PARTY-NOTICES.html"
need "build/wine-pe/controller-pe.version"
need "build/wine-pe/opengl-pe.version"
need "app/Madeira/aarch64-windows/opengl32.dll"
need "app/Madeira/aarch64-windows/glu32.dll"
if [[ -s "$root/build/wine-pe/opengl-pe.version" ]]; then
  expected="$(git -C "$root/wine" rev-parse HEAD)"
  actual="$(tr -d '[:space:]' < "$root/build/wine-pe/opengl-pe.version")"
  if [[ "$actual" != "$expected" ]]; then
    echo "INVALID: OpenGL PE bridge came from a different Wine source; rebuild source-bootstrap"; missing=1;
  fi
fi

if [[ -s "$root/app/Madeira/arm64ec-windows/lua51-gc64.dll" ]]; then
  printf '%s  %s\n' 94fb3e3d4b1f6acce0110e46aadf1ecab1fa17c4ed1ab34caef3c5c2121c208e \
    "$root/app/Madeira/arm64ec-windows/lua51-gc64.dll" | shasum -a 256 -c - >/dev/null || missing=1
fi

if [[ -s "$root/app/Madeira/arm64ec-windows/libEGL.dll" ]]; then
  printf '%s  %s\n' d9c8541eaf0293c67ece10e97d00a8b689d5e043a8356d43224aac1af3a21a5f \
    "$root/app/Madeira/arm64ec-windows/libEGL.dll" | shasum -a 256 -c - >/dev/null || missing=1
fi
if [[ -s "$root/app/Madeira/arm64ec-windows/libGLESv2.dll" ]]; then
  printf '%s  %s\n' a1275f57b47575db9aa3a577e5eacba1d7f1d5578ef6a8072468d788c93c85ff \
    "$root/app/Madeira/arm64ec-windows/libGLESv2.dll" | shasum -a 256 -c - >/dev/null || missing=1
  grep -a -q 'D3D11CreateDevice' "$root/app/Madeira/arm64ec-windows/libGLESv2.dll" || {
    echo "INVALID: libGLESv2.dll has no ANGLE D3D11 backend"; missing=1;
  }
fi
if [[ -s "$root/app/Madeira/licenses/ANGLE-THIRD-PARTY-NOTICES.html" ]]; then
  printf '%s  %s\n' 000ae5775ffa701d57afe7ac3831b76799e8250a2d0c328d1785cba935aab38d \
    "$root/app/Madeira/licenses/ANGLE-THIRD-PARTY-NOTICES.html" | shasum -a 256 -c - >/dev/null || missing=1
fi

if [[ -s "$root/build/wine-pe/controller-pe.version" && -e "$root/wine/.git" ]]; then
  expected="$(git -C "$root/wine" rev-parse HEAD)"
  actual="$(tr -d '[:space:]' < "$root/build/wine-pe/controller-pe.version")"
  if [[ "$actual" != "$expected" ]]; then
    echo "INVALID: controller PE bridge came from $actual, Wine source is $expected"; missing=1
  fi
fi

if [[ -s "$root/app/Madeira/arm64ec-windows/xinput1_3.dll" ]]; then
  grep -a -q 'NtUserCallTwoParam' "$root/app/Madeira/arm64ec-windows/xinput1_3.dll" || {
    echo "INVALID: xinput1_3.dll has no win32u controller bridge"; missing=1;
  }
fi
if [[ -s "$root/app/Madeira/arm64ec-windows/dinput8.dll" ]]; then
  grep -a -q 'MADEIRA-DINPUT-IOS' "$root/app/Madeira/arm64ec-windows/dinput8.dll" || {
    echo "INVALID: dinput8.dll is not Madeira's iOS controller build"; missing=1;
  }
fi
if [[ -s "$root/app/Madeira/arm64ec-windows/user32.dll" ]]; then
  grep -a -q 'NtUserGetPointerInfoList' "$root/app/Madeira/arm64ec-windows/user32.dll" || {
    echo "INVALID: user32.dll has no functional GetPointerInfo route"; missing=1;
  }
fi

# Both DLLs link vkd3d-shader. ANGLE can resolve D3DCompile2VKD3D through
# wined3d.dll, so checking only d3dcompiler_47.dll would allow Balatro's E5017
# loading loop to reappear in a cached/native-bundle build.
for dll in d3dcompiler_47.dll wined3d.dll; do
  if [[ -s "$root/app/Madeira/arm64ec-windows/$dll" ]] &&
     grep -a -q 'Flattening conditional blocks with non-discard jump instructions' \
       "$root/app/Madeira/arm64ec-windows/$dll"; then
    echo "INVALID: $dll still embeds the pre-fix E5017 compiler"; missing=1
  fi
done

if [[ -s "$root/app/Madeira/libwin32u_unix.a" ]]; then
  nm -g "$root/app/Madeira/libwin32u_unix.a" 2>/dev/null | grep -q 'winios_drv_post_key_scan' || {
    echo "INVALID: libwin32u_unix.a predates the physical keyboard scan-code bridge"; missing=1;
  }
fi
if [[ -s "$root/app/Madeira/libntdll_unix.a" ]]; then
  grep -a -q 'shared-roots-v2' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a has destructive shared root enumeration; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'opengl32 (winios WGL)' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a lacks the OpenGL unix dispatch; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'THIN_RESERVE' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a lacks optional thin reservations; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'ml1230 pacing stream' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a lacks ring-fill audio pacing; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'darwin-tos-v1' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a lacks Darwin UDP TOS translation; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'swap-pressure-v1' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a lacks pressure-aware startup swap; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'gameplay-observers=v1' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a predates quiet gameplay observers; rebuild source-bootstrap"; missing=1;
  }
  nm -g "$root/app/Madeira/libntdll_unix.a" 2>/dev/null | grep -q 'wine_runtime_thread_attach' || {
    echo "INVALID: libntdll_unix.a has no reusable-runtime thread lifecycle hooks"; missing=1;
  }
  grep -a -q 'NtCreateFile redirect' "$root/app/Madeira/libntdll_unix.a" || {
    echo "INVALID: libntdll_unix.a predates the effective LÖVE GC64 file redirect"; missing=1;
  }
  nm -g "$root/app/Madeira/libntdll_unix.a" 2>/dev/null | grep -q 'madeira_steam_dns_getaddrinfo' || {
    echo "INVALID: libntdll_unix.a has no app-local Steam DNS resolver"; missing=1;
  }
fi

if [[ -s "$root/app/Madeira/libwineserver.a" ]]; then
  grep -a -q 'request-wake-coalesced-v1' "$root/app/Madeira/libwineserver.a" || {
    echo "INVALID: libwineserver.a predates request wake coalescing; rebuild source-bootstrap"; missing=1;
  }
  grep -a -q 'server-stop-v1' "$root/app/Madeira/libwineserver.a" || {
    echo "INVALID: libwineserver.a lacks independent game termination; rebuild source-bootstrap"; missing=1;
  }
fi

if [[ -s "$root/app/Madeira/libdxmt_combined.a" ]]; then
  for marker in async-writer-v1 shader-compiler-content-v1 pipeline-binary-v1; do
    grep -a -q "$marker" "$root/app/Madeira/libdxmt_combined.a" || {
      echo "INVALID: libdxmt_combined.a lacks $marker; rebuild source-bootstrap"; missing=1;
    }
  done
  grep -a -q 'sandbox-v1 reader ready' "$root/app/Madeira/libdxmt_combined.a" || {
    echo "INVALID: libdxmt_combined.a predates the 64-bit iOS shader cache fix; rebuild source-bootstrap"; missing=1;
  }
  nm -g "$root/app/Madeira/libdxmt_combined.a" 2>/dev/null | grep -q 'wine_runtime_gpu_begin' || {
    echo "INVALID: libdxmt_combined.a has no reusable-runtime GPU completion hooks"; missing=1;
  }
fi

if [[ -s "$root/app/Madeira/arm64ec-windows/d3d11.dll" ]]; then
  grep -a -q 'compiler-workers=v1' "$root/app/Madeira/arm64ec-windows/d3d11.dll" || {
    echo "INVALID: d3d11.dll predates bounded shader workers; rebuild source-bootstrap"; missing=1;
  }
fi

for dll in msvcp90.dll msvcr90.dll \
  concrt140.dll msvcp140.dll msvcp140_1.dll msvcp140_2.dll \
  msvcp140_atomic_wait.dll msvcp140_codecvt_ids.dll vcamp140.dll \
  vccorlib140.dll vcomp140.dll vcruntime140.dll vcruntime140_1.dll \
  vcruntime140_threads.dll; do need "app/Madeira/x86_64-vcruntime/$dll"; done

plutil -lint "$root/app/Madeira/Info.plist" >/dev/null
plutil -lint "$root/app/Madeira/Madeira.entitlements" >/dev/null
plutil -lint "$root/app/MadeiraJITHelper/Info.plist" >/dev/null
[[ "$missing" = 0 ]] || { echo "Native dependency preflight failed." >&2; exit 1; }
echo "All native inputs required by Madeira.xcodeproj are present."
