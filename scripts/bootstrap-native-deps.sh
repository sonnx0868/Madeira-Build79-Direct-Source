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

log "Fetch Microsoft x64 VC runtime"
bash "$root/scripts/fetch-vcruntime.sh"

log "Fetch LÖVE LuaJIT GC64 compatibility runtime"
bash "$root/scripts/fetch-love-luajit-gc64.sh"

log "Fetch ANGLE OpenGL ES to D3D11 compatibility runtime"
bash "$root/scripts/fetch-angle-d3d11.sh"

log "Initialize pinned upstream submodules"
git -C "$root" submodule update --init FEX wine dxmt madeira-dock
git -C "$root/FEX" submodule update --init --depth 1 --jobs 4 \
    External/fmt External/xxhash External/range-v3 External/unordered_dense
git -C "$root/dxmt" submodule update --init --depth 1 include/native/directx

# Keep tiny integration fixes in the main source tree instead of relying on a
# dirty, unpublished submodule checkout. Applying twice is harmless; any third
# state is a real source drift and must fail rather than silently mispatch.
for wine_patch in \
    "$root/patches/wine-socket-cmsg-rate-limit.patch" \
    "$root/patches/wine-luajit-gc64-file-redirect.patch"; do
    if git -C "$root/wine" apply --check "$wine_patch"; then
        git -C "$root/wine" apply "$wine_patch"
    elif ! git -C "$root/wine" apply --reverse --check "$wine_patch"; then
        echo "Wine source no longer matches $wine_patch" >&2
        exit 1
    fi
done

bash "$root/scripts/apply-wine-patches.sh"

dxmt_patch="$root/patches/dxmt-madeira-query-log.patch"
if git -C "$root/dxmt" apply --check "$dxmt_patch"; then
    git -C "$root/dxmt" apply "$dxmt_patch"
elif ! git -C "$root/dxmt" apply --reverse --check "$dxmt_patch"; then
    echo "DXMT source no longer matches $dxmt_patch" >&2
    exit 1
fi

runtime_patch="$root/patches/dxmt-runtime-lifecycle.patch"
if git -C "$root/dxmt" apply --check "$runtime_patch"; then
    git -C "$root/dxmt" apply "$runtime_patch"
elif ! git -C "$root/dxmt" apply --reverse --check "$runtime_patch"; then
    echo "DXMT source no longer matches $runtime_patch" >&2
    exit 1
fi

bash "$root/scripts/apply-dxmt-gameplay-patch.sh"

# The ARM64EC PE modules are tracked build inputs today. Patch the same source
# fix into them deterministically until their rebuild joins this bootstrap.
python3 "$root/tools/patch-dxmt-query-log.py"

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

log "Rebuild Wine controller PE bridge"
bash "$root/build/wine-pe/build-controller.sh"
bash "$root/build/wine-pe/build-wintypes.sh"
bash "$root/build/wine-pe/build-d3dcompiler.sh"

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

log "Build reusable session host"
LLVM_MINGW="$mingw_dir/bin" bash "$root/build/session-host/build.sh"

log "Build Madeira Dock"
LLVM_MINGW="$mingw_dir/bin" bash "$root/build/madeira-dock/build.sh"

log "Build Madeira on-device pairing library"
if ! command -v rustup >/dev/null 2>&1; then
    brew install rustup
    export PATH="/opt/homebrew/opt/rustup/bin:$PATH"
fi
if ! rustup toolchain list | grep -q '^stable.*default'; then
    rustup default stable
fi
rustup target add aarch64-apple-ios
bash "$root/build/rppairing-ios/build.sh"

log "Stage licences and validate"
bash "$root/build/stage-licenses.sh"
bash "$root/scripts/check-ios-build.sh"
echo "Upstream Madeira native dependencies are ready."
