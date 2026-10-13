#!/bin/bash
set -euo pipefail
root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
: "${MADEIRA_NATIVE_DEPS_URL:?Set MADEIRA_NATIVE_DEPS_URL}"
: "${MADEIRA_NATIVE_DEPS_SHA256:?Set MADEIRA_NATIVE_DEPS_SHA256}"
archive="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/madeira-upstream-native-deps.zip"
headers=(-H "Accept: application/octet-stream")
if [[ -n "${MADEIRA_NATIVE_DEPS_TOKEN:-}" ]]; then headers+=(-H "Authorization: Bearer $MADEIRA_NATIVE_DEPS_TOKEN"); fi
curl --fail --location --retry 3 "${headers[@]}" "$MADEIRA_NATIVE_DEPS_URL" --output "$archive"
printf '%s  %s\n' "$MADEIRA_NATIVE_DEPS_SHA256" "$archive" | shasum -a 256 -c -
if [[ -n "${MADEIRA_NATIVE_DEPS_INNER_SHA256:-}" ]]; then
  # One-off rebuilds can reuse a completed bootstrap's deployment ZIP without
  # publishing it or persisting its expiring signed URL as a project secret.
  inner="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/madeira-verified-inner-native-deps.zip"
  python3 "$root/scripts/extract-native-bundle.py" "$archive" "$inner" "$MADEIRA_NATIVE_DEPS_INNER_SHA256"
  archive="$inner"
fi
unzip -q -o "$archive" -d "$root"
# FEXBridge includes these source headers even when its static libraries are
# restored. A non-recursive parent checkout leaves their directories empty.
git -C "$root/FEX" submodule update --init --depth 1 --jobs 4 \
  External/fmt External/xxhash External/range-v3 External/unordered_dense \
  External/rpmalloc Source/Common/cpp-optparse
git -C "$root/dxmt" submodule update --init --depth 1 include/native/directx
test -s "$root/FEX/External/fmt/include/fmt/format.h"
test -s "$root/FEX/External/range-v3/include/range/v3/all.hpp"
test -s "$root/FEX/External/unordered_dense/include/ankerl/unordered_dense.h"
bash "$root/build/stage-licenses.sh"
python3 "$root/tools/patch-dxmt-query-log.py"
if [[ "${1:-}" = --refresh-window ]]; then
  bash "$root/build/win32u-unix/build.sh" window-only
elif [[ -n "${1:-}" ]]; then
  echo 'Usage: restore-native-deps.sh [--refresh-window]' >&2; exit 2
fi
bash "$root/scripts/check-ios-build.sh"
