#!/bin/bash
# One ordered integration pipeline for source bootstrap and cached-bundle builds.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
for name in dxmt-bcd-copy-structure-count.patch dxmt-lazy-shader-objects.patch dxmt-logarithmic-dynamic-trim.patch dxmt-bounded-command-reuse.patch; do
    patch="$root/patches/$name"
    if git -C "$root/dxmt" apply --reverse --check "$patch" >/dev/null 2>&1; then
        git -C "$root/dxmt" apply --reverse "$patch"
    fi
done
padded_patch="$root/patches/dxmt-padded-light-table.patch"
writer_patch="$root/patches/dxmt-multikernel-metallib.patch"
for patch in "$padded_patch" "$writer_patch"; do
    if git -C "$root/dxmt" apply --reverse --check "$patch" >/dev/null 2>&1; then
        git -C "$root/dxmt" apply --reverse "$patch"
    fi
done
residency_patch="$root/patches/dxmt-texture-residency.patch"
if git -C "$root/dxmt" apply --reverse --check "$residency_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$residency_patch"
fi
phase_patch="$root/patches/dxmt-lighting-phase.patch"
if git -C "$root/dxmt" apply --reverse --check "$phase_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$phase_patch"
fi
zero_light_patch="$root/patches/dxmt-zero-light-guard.patch"
if git -C "$root/dxmt" apply --reverse --check "$zero_light_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$zero_light_patch"
fi
upload_batching_patch="$root/patches/dxmt-upload-batching.patch"
if git -C "$root/dxmt" apply --reverse --check "$upload_batching_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$upload_batching_patch"
fi
depth_retention_patch="$root/patches/dxmt-depth-rename-retention.patch"
if git -C "$root/dxmt" apply --reverse --check "$depth_retention_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$depth_retention_patch"
fi
present_resource_patch="$root/patches/dxmt-present-resource-lifetime.patch"
if git -C "$root/dxmt" apply --reverse --check "$present_resource_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$present_resource_patch"
fi
# Stage sampling extends methods already changed by the earlier lifecycle and
# diagnostics patches. Remove this exact top layer before validating those
# layers, then reapply it; unrelated working-tree changes are never discarded.
stage_patch="$root/patches/dxmt-gpu-stages.patch"
if git -C "$root/dxmt" apply --reverse --check "$stage_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$stage_patch"
fi
retirement_patch="$root/patches/dxmt-retirement-lifetime.patch"
if git -C "$root/dxmt" apply --reverse --check "$retirement_patch" >/dev/null 2>&1; then
    git -C "$root/dxmt" apply --reverse "$retirement_patch"
fi
for name in patches/dxmt-resolve-srv-mips.patch patches/dxmt-marshal-progress.patch \
            patches/dxmt-madeira-query-log.patch patches/dxmt-runtime-lifecycle.patch \
            patches/dxmt-gameplay-performance.patch build/dxmt-ios/clean-build.patch \
            build/dxmt-ios/pipeline-cache.patch patches/dxmt-metal-public-atomic.patch \
            build/dxmt-ios/pipeline-warmup.patch patches/dxmt-gpu-foreground.patch \
            patches/dxmt-submit-before-present.patch patches/dxmt-cached-cpu-mappings.patch \
            patches/dxmt-producer-pacing.patch patches/dxmt-query-park.patch patches/dxmt-dead-color-store.patch \
            patches/dxmt-swapchain-hook-abi.patch patches/dxmt-gpu-trace.patch \
            patches/dxmt-bounded-srv-vector-load.patch patches/dxmt-explicit-lod-guard.patch \
            patches/dxmt-texture-headroom.patch patches/dxmt-lossless-color-views.patch \
            patches/dxmt-frameflow.patch patches/dxmt-light-graphics.patch \
            patches/dxmt-guarded-pcf.patch; do
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
git -C "$root/dxmt" apply --check "$zero_light_patch"
git -C "$root/dxmt" apply "$zero_light_patch"
git -C "$root/dxmt" apply --check "$retirement_patch"
git -C "$root/dxmt" apply "$retirement_patch"
git -C "$root/dxmt" apply --check "$stage_patch"
git -C "$root/dxmt" apply "$stage_patch"
echo 'Applied DXMT stage timestamp hooks'
git -C "$root/dxmt" apply --check "$present_resource_patch"
git -C "$root/dxmt" apply "$present_resource_patch"
echo 'Applied DXMT Present lock and native creation lifetime fix'
git -C "$root/dxmt" apply --check "$depth_retention_patch"
git -C "$root/dxmt" apply "$depth_retention_patch"
echo 'Applied completed depth backing retirement'
git -C "$root/dxmt" apply --check "$upload_batching_patch"
git -C "$root/dxmt" apply "$upload_batching_patch"
echo 'Applied independent buffer upload batching'
git -C "$root/dxmt" apply --check "$residency_patch"
git -C "$root/dxmt" apply "$residency_patch"
echo 'Applied lossless cold BC residency hooks'

git -C "$root/dxmt" apply --check "$phase_patch"
git -C "$root/dxmt" apply "$phase_patch"
echo 'Applied full-resolution lighting phase scheduling'
git -C "$root/dxmt" apply --check "$writer_patch"
git -C "$root/dxmt" apply "$writer_patch"
git -C "$root/dxmt" apply --check "$padded_patch"
git -C "$root/dxmt" apply "$padded_patch"
echo 'Applied optional zero-extended lighting entry and bounded native dispatch adapter'

for name in dxmt-bounded-command-reuse.patch dxmt-logarithmic-dynamic-trim.patch dxmt-lazy-shader-objects.patch dxmt-bcd-copy-structure-count.patch; do
    git -C "$root/dxmt" apply --check "$root/patches/$name"
    git -C "$root/dxmt" apply "$root/patches/$name"
done
