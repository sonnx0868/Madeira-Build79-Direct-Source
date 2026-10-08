#!/bin/bash
# One ordered integration pipeline for source bootstrap and cached-bundle builds.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
for name in dxmt-madeira-query-log.patch dxmt-runtime-lifecycle.patch dxmt-gameplay-performance.patch; do
    patch="$root/patches/$name"
    if git -C "$root/dxmt" apply --reverse --check "$patch" >/dev/null 2>&1; then
        echo "DXMT patch already applied: $name"
    elif git -C "$root/dxmt" apply --check "$patch"; then
        git -C "$root/dxmt" apply "$patch"
        echo "Applied DXMT patch: $name"
    else
        echo "DXMT source no longer matches $patch" >&2
        exit 1
    fi
done
