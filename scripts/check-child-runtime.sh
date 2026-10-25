#!/bin/bash
# Isolated Mac/iPhoneOS compile gate; produces no IPA or replacement archives.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="${BITRISE_DEPLOY_DIR:-$root/build/child-runtime-check}"
mkdir -p "$out"
exec > >(tee "$out/child-runtime-check.log") 2>&1
cd "$root"
[[ "$(uname -s)" = Darwin && "$(uname -m)" = arm64 ]]
# Apple's arm64 ASan runtime supports address checks but not LeakSanitizer.
export ASAN_OPTIONS=abort_on_error=1:detect_leaks=0
export UBSAN_OPTIONS=halt_on_error=1
python3 tests/host/check-child-peb-isolation.py
python3 tests/host/check-child-jit-environment.py
python3 tests/host/check-queue-pipe-readiness.py
python3 tests/host/check-wow64-stack-guard.py
python3 tests/host/check-wow64-guard-neighbor.py
python3 tests/host/check-guard-bridge-context.py
python3 tests/host/check-dns-owned-state.py
python3 tests/host/check-wow64-stack-release.py
python3 tests/host/check-wow-context.py
python3 tests/host/check-input-wake.py --require-darwin
git submodule update --init --depth 1 wine
bash scripts/apply-wine-patches.sh
python3 tests/host/check-user-stack-host-pages.py
python3 tests/host/check-renpy-startup.py
python3 tests/host/check-process-winstation-init.py
# Import generated configuration headers only, from the verified926 bundle.
base="${TMPDIR:-/tmp}/madeira-child-runtime-headers.zip"
curl --fail --location --retry 3 --retry-all-errors --retry-delay 2 --silent --show-error \
  'https://github.com/sonnx0868/Madeira-Build79-Direct-Source/releases/download/v0.1.6-build926-d784eea/Madeira-v0.1.6-native-deps-build926.zip' -o "$base"
printf '%s  %s\n' ea9a0a166ce08e73fefc879a07cf3f97df3732d3c633884e8e3709372cae8138 "$base" | shasum -a 256 -c -
python3 - "$base" <<'PY'
import zipfile,sys
from pathlib import Path
with zipfile.ZipFile(sys.argv[1]) as archive:
    for name in archive.namelist():
        if not name.startswith('wine/build-macos/include/') or name.endswith('/'):
            continue
        target=Path(name)
        assert '..' not in target.parts and not target.is_absolute()
        target.parent.mkdir(parents=True,exist_ok=True)
        target.write_bytes(archive.read(name))
PY
bash build/ntdll-unix/build.sh child-check
bash build/wineserver/build.sh queue-check
python3 - "$out" <<'PY'
from pathlib import Path
import hashlib,json,subprocess,sys
files=['build/ntdll-unix/'+n+'_ios.c' for n in ('env','loader','process','server','signal_arm64','virtual')]
files+=['build/wineserver/queue_ios.c']
files+=['build/ntdll-unix/ios_guard_access.h','build/ntdll-unix/dnsapi_unixlib_ios.c']
files+=['build/ntdll-unix/ios_browser_child_role.h','wine/include/winternl.h',
        'wine/dlls/ntdll/thread.c','wine/dlls/ntdll/unix/system.c','wine/dlls/wow64/system.c']
record={'format':'madeira-child-runtime-check-v1','commit':subprocess.check_output(
 ['git','rev-parse','HEAD'],text=True).strip(),'MacProductionRegressionsPassed':True,
 'ASanUBSanPassed':True,'nativePipePollPassed':True,'iPhoneOSUnitsCompiled':9,
 'realMachGuardNeighborAliasPassed':True,'ownerStackReleasePassed':True,
 'hostPageUserStackGuardsPassed':True,'guardBridgeAndNeonPublicationPassed':True,'ownedDarwinResolverPassed':True,
 'physicalChromeAndRenPyVerified':False,'IPAProduced':False,
 'sources':{f:hashlib.sha256(Path(f).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for f in files}}
(Path(sys.argv[1])/'child-runtime-check.json').write_text(json.dumps(record,indent=2)+'\n')
PY
echo 'PASS: real host-aligned user stacks, guard aliases, NEON publication and sanitized regressions; nine iPhoneOS units compiled'
