#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:-$root/build/nutssync-check}"
mkdir -p "$out"
exec > >(tee "$out/nutssync-check.log") 2>&1
[[ "$(uname -s)" = Darwin && "$(uname -m)" = arm64 ]]
xcrun clang -std=gnu11 -O2 -g -Wall -Wextra -Werror -Wno-unused-function \
  -Wno-unused-parameter -Wno-incompatible-pointer-types-discards-qualifiers -mmacosx-version-min=15.0 "$root/tests/host/nutssync.c" -o "$out/nutssync-test"
mkdir -p "$out/config"
printf 'sync-engine = nutssync\n' > "$out/config/madeira.cfg"
run_suite() {
  local engine="$1" pid monitor status=0
  shift
  MADEIRA_DOCS_DIR="$out/config" "$out/nutssync-test" "$@" > "$out/$engine.log" 2>&1 & pid=$!
  (sleep 10; if kill -0 "$pid" 2>/dev/null; then /usr/bin/sample "$pid" 1 -file "$out/$engine-stacks.txt" || true; fi) & monitor=$!
  wait "$pid" || status=$?
  kill "$monitor" 2>/dev/null || true
  cat "$out/$engine.log"
  return "$status"
}
run_suite nutssync
run_suite nutssync-benchmark --benchmark-only
# Legacy path remains an explicit, executable compatibility choice.
printf 'sync-engine = madsync\n' > "$out/config/madeira.cfg"
run_suite madsync --benchmark-only
printf 'sync-engine = nutssync\n' > "$out/config/madeira.cfg"
xcrun clang -std=gnu11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Wno-incompatible-pointer-types-discards-qualifiers -mmacosx-version-min=15.0 \
  "$root/tests/host/nutssync.c" -o "$out/nutssync-asan"
ASAN_OPTIONS=abort_on_error=1:detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
  MADEIRA_DOCS_DIR="$out/config" "$out/nutssync-asan"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
xcrun --sdk iphoneos clang -std=gnu11 -O2 -arch arm64 -isysroot "$sdk" \
  -miphoneos-version-min=17.0 -Werror=implicit-function-declaration -Werror=unguarded-availability-new \
  -fsyntax-only "$root/build/madsync/madsync_bridge.c"
echo 'PASS production NutsSync bridge compiles against iPhoneOS SDK'
python3 "$root/tests/host/check-wineserver-symbol-aliases.py"
python3 "$root/tests/host/check-runtime-settings.py"
python3 "$root/tests/host/check-nutssync-policy.py"
if [[ -f "$root/wine/dlls/ntdll/unix/sync.c" ]]; then
  python3 "$root/tests/host/check-inproc-cache-retire.py"
fi
