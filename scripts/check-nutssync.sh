#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:-$root/build/nutssync-check}"
mkdir -p "$out"
exec > >(tee "$out/nutssync-check.log") 2>&1
[[ "$(uname -s)" = Darwin && "$(uname -m)" = arm64 ]]
xcrun clang -std=gnu11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function \
  -Wno-unused-parameter -mmacosx-version-min=15.0 "$root/tests/host/nutssync.c" -o "$out/nutssync-test"
mkdir -p "$out/config"
printf 'sync-engine = nutssync\n' > "$out/config/madeira.cfg"
MADEIRA_DOCS_DIR="$out/config" "$out/nutssync-test"
# Legacy path remains an explicit, executable compatibility choice.
printf 'sync-engine = madsync\n' > "$out/config/madeira.cfg"
# The old handoff table has a 256-entry limit. Its suite only uses that bound.
MADEIRA_DOCS_DIR="$out/config" "$out/nutssync-test"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
xcrun --sdk iphoneos clang -std=gnu11 -O2 -arch arm64 -isysroot "$sdk" \
  -miphoneos-version-min=17.0 -Werror=implicit-function-declaration \
  -fsyntax-only "$root/build/madsync/madsync_bridge.c"
echo 'PASS production NutsSync bridge compiles against iPhoneOS SDK'
