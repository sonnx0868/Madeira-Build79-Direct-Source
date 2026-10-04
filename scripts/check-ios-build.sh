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
