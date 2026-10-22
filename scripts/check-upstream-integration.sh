#!/bin/bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:-$root/build/upstream-check}"
mkdir -p "$out"
exec > >(tee "$out/upstream-check.log") 2>&1
cd "$root"
[[ "$(uname -s)" = Darwin && "$(uname -m)" = arm64 ]]
git submodule update --init --depth 1 wine dxmt
bash scripts/apply-wine-patches.sh
bash scripts/apply-wine-patches.sh
bash scripts/apply-dxmt-patches.sh
python3 tests/host/check-dxmt-patches.py
python3 tests/host/check-session-object-alignment.py
python3 tests/host/check-thread-registry-reuse.py
python3 tests/host/check-upstream-render-core.py
python3 tests/host/check-upstream-qos.py
python3 tests/host/check-upstream-swap.py
python3 tests/host/check-session-priority.py --require-darwin
python3 tests/host/check-guest-main-qos.py --require-darwin
bash scripts/check-padded-lighting.sh
xcrun -sdk macosx metal -c dxmt/src/dxmt/dxmt_command.metal -std=metal3.1 \
  --target=air64-apple-macos14.0 -o "$out/dxmt_command.air"
# Reuse only generated headers/configuration from the hash-pinned public bundle.
# Do not load its old native libraries over the changed source or re-stamp them.
base="${TMPDIR:-/tmp}/madeira-upstream-headers.zip"
curl --fail --location --retry 3 --silent --show-error \
  'https://github.com/sonnx0868/Madeira-Build79-Direct-Source/releases/download/v0.1.6-build875-82d08f2/Madeira-v0.1.6-native-deps-build875.zip' -o "$base"
printf '%s  %s\n' 41c4a98894a82c767bb69194bd6d5ba8882facbaec7d74b85b5d8357bf50453c "$base" | shasum -a 256 -c -
python3 - "$base" <<'PY'
import zipfile,sys
from pathlib import Path
with zipfile.ZipFile(sys.argv[1]) as archive:
    for name in archive.namelist():
        if (name.startswith(('wine/build-macos/include/','wine/build-arm64ec/include/')) or
            name == 'wine/build-arm64ec/config.status') and not name.endswith('/'):
            target=Path(name)
            assert '..' not in target.parts and not target.is_absolute()
            target.parent.mkdir(parents=True,exist_ok=True)
            target.write_bytes(archive.read(name))
PY
bash build/ntdll-unix/build.sh upstream-check
# Compile actual ARM64EC consumers as well as executing the x64 byte fixtures.
toolchain=llvm-mingw-20260421-ucrt-macos-universal
tc="$root/toolchains/$toolchain/bin"
if [[ ! -x "$tc/arm64ec-w64-mingw32-clang++" ]]; then
  download="${TMPDIR:-/tmp}/$toolchain.tar.xz"
  curl --fail --location --retry 3 --silent --show-error \
    "https://github.com/mstorsjo/llvm-mingw/releases/download/20260421/$toolchain.tar.xz" -o "$download"
  printf '%s  %s\n' bd85a3975723815cef28dbbd2ca2cb0c926f6b348a12a0453f39f7af273cb3f7 "$download" | shasum -a 256 -c -
  mkdir -p "$root/toolchains"
  tar -xJf "$download" -C "$root/toolchains"
fi
export PATH="$tc:/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
git -C dxmt submodule update --init --depth 1 include/native/directx
python3 tests/host/check-dxmt-pe-compile.py --compiler "$tc/arm64ec-w64-mingw32-clang++" --require-tools \
  --only src/dxmt/dxmt_dynamic.cpp --only src/d3d11/d3d11_pipeline_cache.cpp \
  --only src/d3d11/d3d11_context_imm.cpp --only src/d3d11/d3d11_context_def.cpp
(cd wine/build-arm64ec && bash ./config.status)
make -C wine/build-arm64ec -j4 dlls/ntdll/arm64ec-windows/loader.o dlls/ntdll/arm64ec-windows/sync.o
python3 - "$out" <<'PY'
from pathlib import Path
import hashlib,json,subprocess,sys
out=Path(sys.argv[1])
files=['build/ntdll-unix/signal_arm64_ios.c','build/ntdll-unix/thread_ios.c',
       'build/ntdll-unix/virtual_ios.c','build/wineserver/mapping_ios.c',
       'dxmt/src/dxmt/dxmt_command.metal','dxmt/src/dxmt/dxmt_dynamic.cpp',
       'dxmt/src/d3d11/d3d11_context_impl.cpp','wine/dlls/ntdll/cs_x64_fastpath.c',
       'wine/dlls/ntdll/sync.c']
record={'format':'madeira-upstream-check-v1','commit':subprocess.check_output(
 ['git','rev-parse','HEAD'],text=True).strip(),'host':'Apple Silicon Mac',
 'checksPassed':True,'gameFPSMeasured':False,'iPadRAMMeasured':False,
 'sources':{f:hashlib.sha256(Path(f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in files}}
(out/'upstream-check.json').write_text(json.dumps(record,indent=2)+'\n')
PY
echo 'PASS: source integration and production-function host checks; no game FPS claim'
