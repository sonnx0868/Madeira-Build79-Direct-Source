#!/bin/bash
# Apply Madeira-owned Wine changes without requiring push access to the Wine
# submodule's upstream repository. Safe to call repeatedly in local/CI builds.
set -euo pipefail
root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
patch="$root/build/wine-pe/madeira-v0.1.4.patch"

test -s "$patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine Madeira v0.1.4 patch already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
    echo "Applied Wine Madeira v0.1.4 patch"
else
    echo "Wine source does not match the pinned commit required by $patch" >&2
    exit 1
fi

# Native ARM64 children of an ARM64EC session need the inverse DLL-farm
# fallback in their own PE loader (before is_valid_binary rejects system32).
patch="$root/patches/wine-native-child-dlls.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine native child DLL fallback already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the native child DLL fallback patch" >&2
    exit 1
fi

patch="$root/patches/wine-window-proc-validation.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine window procedure validation already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the window procedure validation patch" >&2
    exit 1
fi

patch="$root/patches/wine-user-object-generation.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine client user object generation tracking already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the user object generation patch" >&2
    exit 1
fi

patch="$root/patches/wine-opengl-winios.patch"
if git -C "$root/wine" apply --reverse --check "$patch" >/dev/null 2>&1; then
    echo "Wine winios OpenGL patch already applied"
elif git -C "$root/wine" apply --check "$patch"; then
    git -C "$root/wine" apply "$patch"
else
    echo "Wine source does not match the pinned OpenGL patch" >&2
    exit 1
fi
