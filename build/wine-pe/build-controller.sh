#!/bin/bash
# Rebuild the PE side of Madeira's controller bridge from the pinned Wine
# source.  The app snapshot and win32u unix side are compiled by Xcode/the iOS
# archive build, but games call through these PE DLLs first; shipping an older
# prebuilt farm silently turns every host query into "device not connected".
set -euo pipefail

R="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
B="$R/wine/build-arm64ec"
TC="$R/toolchains/llvm-mingw-20260421-ucrt-macos-universal/bin"
DEST="$R/app/Madeira/arm64ec-windows"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"
STRIP="$TC/arm64ec-w64-mingw32-strip"
export PATH="$TC:$PATH"

[ -x "$STRIP" ] || { echo "llvm-mingw not found at $TC" >&2; exit 1; }
[ -f "$B/config.status" ] || { echo "Wine ARM64EC tree is not configured: $B" >&2; exit 1; }

modules=(win32u xinput1_1 xinput1_2 xinput1_3 xinput1_4 xinput9_1_0 dinput dinput8)
targets=()
for module in "${modules[@]}"; do
    targets+=("dlls/$module/arm64ec-windows/$module.dll")
done

echo "== rebuild ${#targets[@]} ARM64EC controller bridge DLLs =="
make -C "$B" -j"$JOBS" "${targets[@]}"
mkdir -p "$DEST"
for target in "${targets[@]}"; do
    source="$B/$target"
    name="$(basename "$target")"
    test -s "$source"
    cp "$source" "$DEST/$name.tmp"
    "$STRIP" --strip-debug "$DEST/$name.tmp"
    mv -f "$DEST/$name.tmp" "$DEST/$name"
    printf '  %-20s %s bytes\n' "$name" "$(wc -c < "$DEST/$name" | tr -d ' ')"
done

# Static proof that the two guest APIs enter win32u, and that DirectInput is
# the Madeira implementation rather than an older generic Wine binary.
grep -a -q 'NtUserCallTwoParam' "$DEST/xinput1_3.dll"
grep -a -q 'NtUserCallTwoParam' "$DEST/dinput8.dll"
grep -a -q 'MADEIRA-DINPUT-IOS' "$DEST/dinput8.dll"
git -C "$R/wine" rev-parse HEAD > "$R/build/wine-pe/controller-pe.version"
echo "Controller PE bridge rebuilt from $(git -C "$R/wine" rev-parse --short HEAD)."
