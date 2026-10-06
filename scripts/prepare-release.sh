#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
# A full history gives a monotonically increasing build for this source line.
if [[ "$(git -C "$root" rev-parse --is-shallow-repository)" = true ]]; then
  git -C "$root" fetch --unshallow origin
fi
python3 "$root/scripts/release-metadata.py" "$1"
