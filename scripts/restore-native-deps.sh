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
unzip -q -o "$archive" -d "$root"
python3 "$root/tools/patch-dxmt-query-log.py"
bash "$root/scripts/check-ios-build.sh"
