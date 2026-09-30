#!/bin/bash
# Restore the versioned native inputs that are intentionally not stored in the
# public repository. Intended for Codemagic and other clean macOS CI runners.
set -euo pipefail

: "${MADEIRA_NATIVE_DEPS_URL:?Set MADEIRA_NATIVE_DEPS_URL to the pinned ZIP}"
: "${MADEIRA_NATIVE_DEPS_SHA256:?Set MADEIRA_NATIVE_DEPS_SHA256}"

repo_root="${CM_BUILD_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
temp_root="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}"
archive="$temp_root/madeira-native-deps.zip"

curl_args=(--fail --location --retry 3 --show-error)
if [[ -n "${MADEIRA_NATIVE_DEPS_TOKEN:-}" ]]; then
    curl_args+=(--header "Authorization: Bearer $MADEIRA_NATIVE_DEPS_TOKEN")
fi

curl "${curl_args[@]}" "$MADEIRA_NATIVE_DEPS_URL" --output "$archive"
printf '%s  %s\n' "$MADEIRA_NATIVE_DEPS_SHA256" "$archive" | shasum -a 256 -c -
unzip -q -o "$archive" -d "$repo_root"

echo "Restored native dependencies from a SHA-256-verified archive."
