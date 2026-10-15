#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
if [[ ! -s "$root/app/Madeira/mono/receipt.json" ]]; then
  python3 "$root/tools/stage-wine-mono.py" --root "$root"
fi
# A full history gives a monotonically increasing build for this source line.
if [[ "$(git -C "$root" rev-parse --is-shallow-repository)" = true ]]; then
  # Historical gitlinks may refer to commits removed from upstream forks. Only
  # the parent history is needed for the build count; keep the built pins intact.
  git -C "$root" fetch --no-recurse-submodules --unshallow origin
fi
python3 "$root/scripts/release-metadata.py" "$1"
