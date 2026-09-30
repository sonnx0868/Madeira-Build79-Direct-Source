#!/bin/bash
# Read-only preflight for the generated inputs consumed by Madeira.xcodeproj.
# Run on macOS before opening Xcode; it turns opaque linker errors into an
# actionable list of missing build stages.
set -eu

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
MISSING=0

need_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "MISSING command: $1"
        MISSING=1
    fi
}

need_file() {
    if [[ ! -f "$REPO_ROOT/$1" ]]; then
        echo "MISSING file: $1"
        MISSING=1
    fi
}

need_dir() {
    if [[ ! -d "$REPO_ROOT/$1" ]]; then
        echo "MISSING directory: $1"
        MISSING=1
    fi
}

need_command xcodebuild
need_command xcrun
need_command plutil

need_file FEX/build-ios/FEXCore/Source/libFEXCore.a
need_file FEX/build-ios/FEXCore/Source/libFEXCore_Base.a
need_file FEX/build-ios/FEXCore/Source/libJemallocLibs.a
need_file FEX/build-ios/External/fmt/libfmt.a
need_file FEX/build-ios/External/cephes/libcephes_128bit.a
need_file FEX/build-ios/External/xxhash/cmake_unofficial/libxxhash.a
need_file FEX/build-ios/External/SoftFloat-3e/libsoftfloat_3e.a

need_file app/Madeira/libwineserver.a
need_file app/Madeira/libntdll_unix.a
need_file app/Madeira/libwin32u_unix.a
need_file app/Madeira/libdxmt_combined.a
need_file app/Madeira/prefix-template.tar.gz
need_dir app/Madeira/aarch64-windows
need_dir app/Madeira/arm64ec-windows
for dll in \
    concrt140.dll msvcp140.dll msvcp140_1.dll msvcp140_2.dll \
    msvcp140_atomic_wait.dll msvcp140_codecvt_ids.dll vcamp140.dll \
    vccorlib140.dll vcomp140.dll vcruntime140.dll vcruntime140_1.dll \
    vcruntime140_threads.dll; do
    need_file "app/Madeira/x86_64-vcruntime/$dll"
done

if command -v plutil >/dev/null 2>&1; then
    plutil -lint "$REPO_ROOT/app/Madeira/Info.plist" >/dev/null
    plutil -lint "$REPO_ROOT/app/Madeira/Madeira.entitlements" >/dev/null
fi

if [[ -x "$REPO_ROOT/tools/check-prefix-template.sh" ]]; then
    "$REPO_ROOT/tools/check-prefix-template.sh" \
        "$REPO_ROOT/app/Madeira/prefix-template.tar.gz"
fi

if [[ "$MISSING" -ne 0 ]]; then
    echo
    echo "Preflight FAILED. Build the native stages under build/* and follow"
    echo "tools/fetch-vcruntime.md before invoking xcodebuild."
    exit 1
fi

echo "Build inputs look complete. Select your Apple Development team in Xcode, then run:"
echo "  xcodebuild -project app/Madeira.xcodeproj -scheme Madeira -configuration Release -sdk iphoneos"
