#!/bin/bash
# One ordered integration pipeline for source bootstrap and cached-bundle builds.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
for name in patches/dxmt-madeira-query-log.patch patches/dxmt-runtime-lifecycle.patch \
            patches/dxmt-gameplay-performance.patch build/dxmt-ios/clean-build.patch \
            build/dxmt-ios/pipeline-cache.patch; do
    patch="$root/$name"
    if git -C "$root/dxmt" apply --reverse --check "$patch" >/dev/null 2>&1; then
        echo "DXMT patch already applied: $name"
    elif git -C "$root/dxmt" apply --check "$patch"; then
        git -C "$root/dxmt" apply "$patch"
        echo "Applied DXMT patch: $name"
    elif [[ "$name" == "build/dxmt-ios/clean-build.patch" ]] && \
         git -C "$root/dxmt" apply --reverse --check "$root/build/dxmt-ios/legacy-metal-source-fallback.patch" >/dev/null 2>&1; then
        git -C "$root/dxmt" apply --reverse "$root/build/dxmt-ios/legacy-metal-source-fallback.patch"
        git -C "$root/dxmt" apply --check "$patch"
        git -C "$root/dxmt" apply "$patch"
        echo "Updated legacy native Metal source fallback"
    else
        echo "DXMT source no longer matches $patch" >&2
        exit 1
    fi
done
