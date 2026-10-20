#!/bin/bash
# Rebuild the two verified defects; preserve the remaining native bundle.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/dxmt-pe/depth-retention-refresh"
mkdir -p "$out"

# Exercise production pool ownership and XInput routing before compilation.
# The Mac versions also run the address/undefined-behaviour sanitizers.
python3 "$root/tests/host/check-depth-rename-retention.py"
python3 "$root/tests/host/check-xinput-host-authority.py"
bash "$root/build/win32u-unix/build.sh" input-only
python3 "$root/tools/dxmt-depth-retention-bundle.py" --root "$root" \
  --prepare "$out/base.json"
bash "$root/scripts/refresh-dxmt-query.sh"
python3 "$root/tools/dxmt-depth-retention-bundle.py" --root "$root" \
  --stamp --base "$out/base.json"

# The capability is implemented by native Win32u and consumed by all four
# XInput implementations (9.1.0 forwards to 1.4). Ship both halves together.
bash "$root/build/wine-pe/build-xinput.sh"
python3 "$root/tools/xinput-host-authority-bundle.py" --root "$root"
echo 'Rebuilt idle depth backing retirement and authoritative iOS controller discovery'
