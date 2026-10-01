#!/bin/bash
# Cold-build every generated native input needed by Madeira.xcodeproj on a
# clean Apple-silicon macOS runner. Designed for Codemagic's 120-minute job;
# all expensive directories are cacheable and every stage is idempotent.
set -euo pipefail

root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"
export JOBS="$jobs"
export HOMEBREW_NO_AUTO_UPDATE=1
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"

log() { printf '\n========== %s ==========\n' "$*"; }

log "Install host build tools"
formulae=(cmake ninja ccache bison flex pkg-config sevenzip)
missing=()
for formula in "${formulae[@]}"; do
    brew list "$formula" >/dev/null 2>&1 || missing+=("$formula")
done
if (( ${#missing[@]} )); then brew install "${missing[@]}"; fi

log "Verify pinned submodules and apply Madeira lineage patches"
git -C "$root" submodule update --init --recursive
bash "$root/scripts/apply-lineage-patches.sh"

log "Install pinned llvm-mingw"
mingw_name="llvm-mingw-20260421-ucrt-macos-universal"
mingw_dir="$root/toolchains/$mingw_name"
if [[ ! -x "$mingw_dir/bin/arm64ec-w64-mingw32-clang" ]]; then
    mkdir -p "$root/toolchains"
    archive="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/$mingw_name.tar.xz"
    curl --fail --location --retry 3 \
        "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$mingw_name.tar.xz" \
        --output "$archive"
    printf '%s  %s\n' \
        bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 \
        "$archive" | shasum -a 256 -c -
    tar -xJf "$archive" -C "$root/toolchains"
fi
export PATH="$mingw_dir/bin:$PATH"

log "Configure Wine host/generated-header tree"
wine_host="$root/wine/build-macos"
if [[ ! -f "$wine_host/config.status" ]]; then
    mkdir -p "$wine_host"
    (
        cd "$wine_host"
        ../configure --enable-archs=aarch64 --without-x --without-vulkan \
            --without-freetype --without-gnutls --disable-tests --enable-winegstreamer
    )
fi
# server_protocol.h is tracked in this Wine revision; only the WIDL-generated
# DirectWrite headers need make targets.
make -C "$wine_host" -j"$jobs" include/dwrite.h include/dwrite_3.h

log "Configure Wine ARM64EC generated-header tree"
wine_ec="$root/wine/build-arm64ec"
if [[ ! -f "$wine_ec/config.status" ]]; then
    mkdir -p "$wine_ec"
    (
        cd "$wine_ec"
        ../configure --enable-archs=arm64ec --without-x --without-vulkan \
            --without-freetype --without-gnutls --disable-tests --enable-winegstreamer
    )
fi
make -C "$wine_ec" -j"$jobs" include/dwrite.h include/dwrite_3.h

log "Build FEX iOS static archives"
bash "$root/build/fex-ios/build.sh"

log "Build GnuTLS/GMP/Nettle headers and archives"
(
    cd "$root/build/gnutls-ios/src"
    shasum -a 256 -c SHA256SUMS
)
bash "$root/build/gnutls-ios/build.sh"
for lib in gmp nettle hogweed gnutls; do
    cp "$root/toolchains/gnutls-ios/lib/lib$lib.a" "$root/app/Madeira/lib$lib.a"
done

log "Build FreeType for Wine text rendering"
if [[ ! -d "$root/research/freetype/.git" ]]; then
    git clone --filter=blob:none --depth 1 --branch VER-2-13-3 \
        https://github.com/freetype/freetype.git "$root/research/freetype"
fi
# VER-2-13-3 is an annotated tag: 534ad... is the tag object while Git
# checks out its peeled commit 42608f... . Verify the commit that is actually
# compiled, rather than comparing HEAD with the tag object ID.
test "$(git -C "$root/research/freetype" rev-parse HEAD)" = \
    42608f77f20749dd6ddc9e0536788eaad70ea4b5
bash "$root/build/freetype-ios/build.sh"

log "Build Wine unix-side static archives"
bash "$root/build/wineserver/build.sh"
bash "$root/build/ntdll-unix/build.sh"
bash "$root/build/win32u-unix/build.sh"

log "Build LLVM 15 for iOS and combine DXMT"
bash "$root/build/llvm-ios/build.sh"
MADEIRA_ALLOW_NO_D3D12=1 bash "$root/build/dxmt-ios/build.sh"

log "Fetch the official Microsoft VC++ x64 runtime"
vcrt="$root/app/Madeira/x86_64-vcruntime"
required_vcrt=(
  concrt140.dll msvcp140.dll msvcp140_1.dll msvcp140_2.dll
  msvcp140_atomic_wait.dll msvcp140_codecvt_ids.dll vcamp140.dll
  vccorlib140.dll vcomp140.dll vcruntime140.dll vcruntime140_1.dll
  vcruntime140_threads.dll
)
missing_vcrt=0
for dll in "${required_vcrt[@]}"; do [[ -f "$vcrt/$dll" ]] || missing_vcrt=1; done
if [[ "$missing_vcrt" = 1 ]]; then
    work="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/madeira-vcredist"
    rm -rf "$work"
    mkdir -p "$work/outer" "$work/inner" "$vcrt"
    curl --fail --location --retry 3 \
        https://aka.ms/vs/17/release/vc_redist.x64.exe \
        --output "$work/vc_redist.x64.exe"
    vc_sha="${VC_REDIST_X64_SHA256:-CC0FF0EB1DC3F5188AE6300FAEF32BF5BEEBA4BDD6E8E445A9184072096B713B}"
    printf '%s  %s\n' "$vc_sha" "$work/vc_redist.x64.exe" | shasum -a 256 -c -
    7zz x -y "$work/vc_redist.x64.exe" -o"$work/outer" >/dev/null
    find "$work/outer" -type f -name '*.cab' -print0 |
        while IFS= read -r -d '' cab; do
            7zz x -y "$cab" -o"$work/inner" >/dev/null || true
        done
    python3 - "$work/inner" "$vcrt" "${required_vcrt[@]}" <<'PY'
from pathlib import Path
import shutil, sys
src, dst = Path(sys.argv[1]), Path(sys.argv[2])
files = {p.name.lower(): p for p in src.rglob('*') if p.is_file()}
for name in sys.argv[3:]:
    p = files.get(name.lower())
    if not p:
        raise SystemExit(f'missing VC runtime DLL after extraction: {name}')
    shutil.copy2(p, dst / name)
PY
fi

python3 - "$vcrt" "${required_vcrt[@]}" <<'PY'
from pathlib import Path
import struct, sys
root = Path(sys.argv[1])
for name in sys.argv[2:]:
    path = root / name
    data = path.read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    cert_off, cert_size = struct.unpack_from('<II', data, pe + 24 + 112 + 4 * 8)
    if not cert_size or cert_off + cert_size > len(data):
        raise SystemExit(f'VC runtime is missing its Authenticode payload: {path}')
PY

log "Validate native dependency closure"
bash "$root/scripts/check-ios-build.sh"
echo "Native dependency bootstrap completed successfully."
