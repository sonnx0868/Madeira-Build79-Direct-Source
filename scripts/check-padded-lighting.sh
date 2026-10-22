#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${TMPDIR:-/tmp}/madeira-padded-light-tests-$$"
mkdir -p "$out"
for ownership in arc no-objc-arc; do
  flag=-fobjc-arc
  [[ "$ownership" != no-objc-arc ]] || flag=-fno-objc-arc
  xcrun clang "$flag" -O2 -framework Foundation -framework Metal \
    -I"$root/build/dxmt-ios" "$root/tests/host/padded-lighting.m" -o "$out/check-$ownership"
  MTL_DEBUG_LAYER=1 "$out/check-$ownership"
done
