#!/bin/bash
# Check production scheduling semantics, then independently verify Metal output.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$BITRISE_DEPLOY_DIR"
mkdir -p "$out"
exec > >(tee "$out/upload-batching-check.log") 2>&1
git -C "$root" submodule update --init --depth 1 dxmt
bash "$root/scripts/apply-dxmt-patches.sh"
python3 "$root/tests/host/check-upload-batching.py"
sdk="$(xcrun --sdk iphoneos --show-sdk-path)"
src="$root/dxmt/src"
xcrun --sdk iphoneos clang++ -arch arm64 -isysroot "$sdk" -miphoneos-version-min=18.0 \
  -fblocks -O2 -std=c++20 -fexceptions -frtti -Wno-extern-c-compat -Wno-microsoft-exception-spec \
  -DDXMT_NATIVE=1 -DDXMT_MADEIRA=1 -DDXMT_IOS=1 -DDXMT_PAGE_SIZE=4096 -DNOMINMAX \
  -I"$src/nativemetal" -I"$root/dxmt/include" -I"$root/dxmt/libs" \
  -I"$src/winemetal" -I"$src/airconv" -I"$src/util" -I"$src/dxmt" \
  -I"$src/d3d9" -I"$src/d3d9/unix" -I"$src/d3d9shim" \
  -I"$root/dxmt/include/native/directx" -I"$root/dxmt/include/native/windows" \
  -c "$src/dxmt/dxmt_context.cpp" -o "$out/dxmt_context-ios.o"
if ! xcrun --find metal >/dev/null 2>&1; then
  xcodebuild -downloadComponent MetalToolchain
fi
xcrun clang++ -std=c++20 -O2 -fobjc-arc -framework Foundation -framework Metal \
  -I"$root/build/dxmt-ios" "$root/tests/host/upload-batching-metal.mm" \
  -o "$out/upload-batching-metal"
MTL_DEBUG_LAYER=1 "$out/upload-batching-metal" 4 > "$out/upload-batching-validation.json"
MTL_DEBUG_LAYER=0 "$out/upload-batching-metal" 40 > "$out/upload-batching-performance.json"
python3 - "$out" <<'PY'
from pathlib import Path
import json,sys
root=Path(sys.argv[1])
for name in ('validation','performance'):
    data=json.loads((root/('upload-batching-'+name+'.json')).read_text())
    assert data['byte_parity'] and data['copied_bytes']==32512
    assert data['baseline_blit_passes']==50 and data['batched_blit_passes']==1
    print(name, json.dumps(data,sort_keys=True))
PY
