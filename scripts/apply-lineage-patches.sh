#!/bin/bash
# Apply the small set of Build 79 materialized changes on top of the exact
# public submodule commits recorded by the root repository.
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$script_dir/.." && pwd)"

apply_once() {
    local repo=$1 patch=$2
    if git -C "$root/$repo" apply --reverse --check "$root/$patch" >/dev/null 2>&1; then
        echo "$repo: lineage patch already applied"
    else
        git -C "$root/$repo" apply --check "$root/$patch"
        git -C "$root/$repo" apply "$root/$patch"
        echo "$repo: applied $patch"
    fi
}

apply_once FEX patches/fex-build79-materialized.patch
apply_once FEX patches/fex-ios-build-fix.patch
apply_once FEX patches/fex-ios-archhelpers-fix.patch
apply_once wine patches/wine-build79-materialized.patch
apply_once research/dxmt patches/dxmt-build79-materialized.patch
