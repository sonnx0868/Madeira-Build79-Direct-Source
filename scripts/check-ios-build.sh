#!/bin/bash
set -eu
root="$(cd "$(dirname "$0")/.." && pwd)"
missing=0
need() { if [[ ! -s "$root/$1" ]]; then echo "MISSING: $1"; missing=1; fi; }

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
  app/Madeira/arm64ec-windows/dockhost.exe; do need "$path"; done

for dll in win32u.dll dinput.dll dinput8.dll \
  xinput1_1.dll xinput1_2.dll xinput1_3.dll xinput1_4.dll \
  xinput9_1_0.dll; do need "app/Madeira/arm64ec-windows/$dll"; done
need "app/Madeira/arm64ec-windows/lua51-gc64.dll"
need "build/wine-pe/controller-pe.version"

if [[ -s "$root/app/Madeira/arm64ec-windows/lua51-gc64.dll" ]]; then
  printf '%s  %s\n' 94fb3e3d4b1f6acce0110e46aadf1ecab1fa17c4ed1ab34caef3c5c2121c208e \
    "$root/app/Madeira/arm64ec-windows/lua51-gc64.dll" | shasum -a 256 -c - >/dev/null || missing=1
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
