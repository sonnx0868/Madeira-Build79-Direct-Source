#!/bin/bash
# Compile the actual embedded AIR helpers before the expensive native bootstrap.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${CM_TEMP_DIR:-${TMPDIR:-/tmp}}/madeira-metal-preflight"
mkdir -p "$out"
for shader in air_msad air_samplepos air_tessellation; do
  # Keep these arguments identical to the AIR generator in dxmt-ios/build.sh.
  xcrun -sdk macosx metal -o "$out/$shader.air" -c \
    "$root/dxmt/src/airconv/shaders/$shader.metal" \
    -std=metal3.1 --target=air64-apple-macos14.0
  test -s "$out/$shader.air"
  echo "PASS: Metal helper $shader compiles"
done
