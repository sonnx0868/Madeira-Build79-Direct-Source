#!/bin/bash
# Cold-build the generated native inputs required by upstream Madeira.
set -euo pipefail

root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
jobs="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"
export JOBS="$jobs" HOMEBREW_NO_AUTO_UPDATE=1
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:/opt/homebrew/opt/llvm/bin:$PATH"
log() { printf '\n========== %s ==========\n' "$*"; }

log "Install host tools"
formulae=(cmake ninja ccache bison flex pkg-config sevenzip llvm)
missing=()
for formula in "${formulae[@]}"; do brew list "$formula" >/dev/null 2>&1 || missing+=("$formula"); done
if (( ${#missing[@]} )); then brew install "${missing[@]}"; fi

log "Initialize Madeira v0.1.0 pinned submodules"
git -C "$root" submodule update --init FEX wine research/dxmt research/madeira-dock
git -C "$root/FEX" submodule update --init --depth 1 --jobs 4 \
    External/fmt External/xxhash External/range-v3 External/unordered_dense
git -C "$root/research/dxmt" submodule update --init --depth 1 include/native/directx

log "Install pinned llvm-mingw"
mingw_name="llvm-mingw-20260421-ucrt-macos-universal"
mingw_dir="$root/toolchains/$mingw_name"
if [[ ! -x "$mingw_dir/bin/arm64ec-w64-mingw32-clang" ]]; then
    mkdir -p "$root/toolchains"
    archive="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/$mingw_name.tar.xz"
    curl --fail --location --retry 3 \
      "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$mingw_name.tar.xz" \
      --output "$archive"
    printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$archive" | shasum -a 256 -c -
    tar -xJf "$archive" -C "$root/toolchains"
fi
export PATH="$mingw_dir/bin:$PATH"

log "Configure Wine generated-header trees"
wine_host="$root/wine/build-macos"
if [[ ! -f "$wine_host/config.status" ]]; then
    mkdir -p "$wine_host"
    (cd "$wine_host" && ../configure --enable-archs=aarch64 --without-x --without-vulkan \
        --without-freetype --without-gnutls --disable-tests --enable-winegstreamer)
fi
# The unixlib sources include a wider generated-header closure (wtypes,
# objidl, mfobjects, dwrite, and friends). Generate the whole include tree so
# Xcode updates do not expose the next missing header one at a time.
make -C "$wine_host" -j"$jobs" include/all tools/widl/all tools/winebuild/all

wine_ec="$root/wine/build-arm64ec"
if [[ ! -f "$wine_ec/config.status" ]]; then
    mkdir -p "$wine_ec"
    (cd "$wine_ec" && ../configure --enable-archs=arm64ec --without-x --without-vulkan \
        --without-freetype --without-gnutls --disable-tests --enable-winegstreamer)
fi
make -C "$wine_ec" -j"$jobs" include/all tools/widl/all tools/winebuild/all

log "Build FEX iOS"
bash "$root/build/fex-ios/build.sh"

log "Build GnuTLS and FFmpeg"
(cd "$root/build/gnutls-ios/src" && shasum -a 256 -c SHA256SUMS)
bash "$root/build/gnutls-ios/build.sh"
for lib in gmp nettle hogweed gnutls; do cp "$root/toolchains/gnutls-ios/lib/lib$lib.a" "$root/app/Madeira/lib$lib.a"; done
bash "$root/build/ffmpeg/build.sh"

log "Build FreeType"
if [[ ! -d "$root/research/freetype/.git" ]]; then
    git clone --filter=blob:none --depth 1 --branch VER-2-13-3 \
        https://github.com/freetype/freetype.git "$root/research/freetype"
fi
test "$(git -C "$root/research/freetype" rev-parse HEAD)" = 42608f77f20749dd6ddc9e0536788eaad70ea4b5
bash "$root/build/freetype-ios/build.sh"

log "Build Wine iOS static archives"
bash "$root/build/wineserver/build.sh"
bash "$root/build/ntdll-unix/build.sh"
bash "$root/build/win32u-unix/build.sh"

log "Build LLVM and DXMT"
bash "$root/build/llvm-ios/build.sh"
# `xcrun -f metal` can resolve Apple's placeholder launcher even when the
# separately distributed Metal Toolchain is absent. Always ask Xcode to
# install/verify the component; the command is a quick no-op when cached.
xcodebuild -downloadComponent MetalToolchain
xcrun -sdk macosx metal -v >/dev/null
bash "$root/build/dxmt-ios/build.sh"

log "Build Madeira Dock"
LLVM_MINGW="$mingw_dir/bin" bash "$root/build/madeira-dock/build.sh"

log "Fetch Microsoft x64 VC runtime"
vcrt="$root/app/Madeira/x86_64-vcruntime"
required_vcrt=(concrt140.dll msvcp140.dll msvcp140_1.dll msvcp140_2.dll msvcp140_atomic_wait.dll msvcp140_codecvt_ids.dll vcamp140.dll vccorlib140.dll vcomp140.dll vcruntime140.dll vcruntime140_1.dll vcruntime140_threads.dll)
missing_vcrt=0
for dll in "${required_vcrt[@]}"; do [[ -f "$vcrt/$dll" ]] || missing_vcrt=1; done
if [[ "$missing_vcrt" = 1 ]]; then
    work="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/madeira-vcredist"
    rm -rf "$work"; mkdir -p "$work/outer" "$work/inner" "$vcrt"
    curl --fail --location --retry 3 https://aka.ms/vs/17/release/vc_redist.x64.exe --output "$work/vc_redist.x64.exe"
    vc_sha="${VC_REDIST_X64_SHA256:-CC0FF0EB1DC3F5188AE6300FAEF32BF5BEEBA4BDD6E8E445A9184072096B713B}"
    printf '%s  %s\n' "$vc_sha" "$work/vc_redist.x64.exe" | shasum -a 256 -c -
    7zz x -y "$work/vc_redist.x64.exe" -o"$work/outer" >/dev/null
    # Microsoft's bundle stores its payload below .rsrc/<locale>/CABINET.
    # Extract every CAB independently: flattening all CABs into one directory
    # can overwrite payloads, while swallowing a 7-Zip failure hides the real
    # problem until the later "missing DLL" check.
    cabs=()
    while IFS= read -r -d '' cab; do cabs+=("$cab"); done \
        < <(find "$work/outer" -type f -iname '*.cab' -print0)
    if (( ${#cabs[@]} == 0 )); then
        echo "No CAB payload found in VC_redist.x64.exe; extracted files:" >&2
        find "$work/outer" -maxdepth 5 -type f -print >&2
        exit 1
    fi
    cab_index=0
    for cab in "${cabs[@]}"; do
        cab_out="$work/inner/$cab_index"
        mkdir -p "$cab_out"
        echo "Extracting VC runtime payload: ${cab#$work/outer/}"
        7zz x -y "$cab" -o"$cab_out" >/dev/null
        cab_index=$((cab_index + 1))
    done
    python3 - "$work/inner" "$vcrt" "${required_vcrt[@]}" <<'PY'
from pathlib import Path
import shutil, sys
src, dst = Path(sys.argv[1]), Path(sys.argv[2])
files = {p.name.lower(): p for p in src.rglob('*') if p.is_file()}
for name in sys.argv[3:]:
    if name.lower() not in files:
        available = '\n  '.join(sorted(files))
        raise SystemExit(f'missing VC runtime DLL: {name}\nExtracted files:\n  {available}')
    shutil.copy2(files[name.lower()], dst / name)
PY
fi

# Preserve the Microsoft Authenticode payload. A truncated/modified runtime is
# both unusable for this reproducible build and outside the intended input.
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
print(f'Validated {len(sys.argv) - 2} signed Microsoft runtime DLLs')
PY

log "Stage licences and validate"
bash "$root/build/stage-licenses.sh"
bash "$root/scripts/check-ios-build.sh"
echo "Upstream Madeira native dependencies are ready."
