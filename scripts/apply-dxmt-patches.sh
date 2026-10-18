#!/bin/bash
# One ordered integration pipeline for source bootstrap and cached-bundle builds.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
# Stage sampling extends methods already changed by the earlier lifecycle and
# diagnostics patches. Remove this exact top layer before validating those
# layers, then reapply it; unrelated working-tree changes are never discarded.
stage_patch="$root/patches/dxmt-gpu-stages.patch"
if git -C "$root/dxmt" apply --reverse --check "$stage_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$stage_patch"
fi
for name in patches/dxmt-madeira-query-log.patch patches/dxmt-runtime-lifecycle.patch \
            patches/dxmt-gameplay-performance.patch build/dxmt-ios/clean-build.patch \
            build/dxmt-ios/pipeline-cache.patch patches/dxmt-metal-public-atomic.patch \
            build/dxmt-ios/pipeline-warmup.patch patches/dxmt-gpu-foreground.patch \
            patches/dxmt-submit-before-present.patch patches/dxmt-cached-cpu-mappings.patch \
            patches/dxmt-producer-pacing.patch patches/dxmt-query-park.patch patches/dxmt-dead-color-store.patch \
            patches/dxmt-swapchain-hook-abi.patch patches/dxmt-gpu-trace.patch \
            patches/dxmt-bounded-srv-vector-load.patch patches/dxmt-explicit-lod-guard.patch; do
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
git -C "$root/dxmt" apply --check "$stage_patch"
git -C "$root/dxmt" apply "$stage_patch"
echo 'Applied DXMT stage timestamp hooks'
