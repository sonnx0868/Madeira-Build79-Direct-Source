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
