#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
patch="$root/patches/dxmt-gameplay-performance.patch"
if git -C "$root/dxmt" apply --check "$patch"; then
    git -C "$root/dxmt" apply "$patch"
elif ! git -C "$root/dxmt" apply --reverse --check "$patch"; then
    echo "DXMT source no longer matches $patch" >&2
    exit 1
fi
