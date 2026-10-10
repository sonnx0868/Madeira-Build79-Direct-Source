#!/usr/bin/env python3
"""Apply Madeira's fork integration; retain the upstream FEX source pin."""
from pathlib import Path
import argparse, hashlib, shutil, subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', required=True)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
source = root/'FEX'
patch = root/'build/fex-arm64ec/backend-cache.patch'
check = subprocess.run(['git', '-C', str(source), 'apply', '--check', str(patch)], capture_output=True)
if check.returncode == 0:
    subprocess.run(['git', '-C', str(source), 'apply', str(patch)], check=True)
elif subprocess.run(['git', '-C', str(source), 'apply', '--reverse', '--check', str(patch)], capture_output=True).returncode:
    raise SystemExit('FEX source drift: backend-cache.patch cannot be applied or recognized')
target = source/'FEXCore/Source/Interface/Core/JIT'
helper = root/'build/fex-arm64ec/madeira_code_cache.h'
shutil.copyfile(helper, target/helper.name)
identity = hashlib.sha256()
identity.update(subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD']))
identity.update(subprocess.check_output([args.compiler, '--version']))
for path in [patch, helper, root/'build/fex-arm64ec/build.sh']:
    identity.update(path.read_bytes())
# Include local portability fixes and compiler configuration, not just the
# upstream commit. Cached output must never survive a modified emitter.
identity.update(subprocess.check_output(['git', '-C', str(source), 'diff', 'HEAD', '--', 'FEXCore', 'Source', 'CMakeLists.txt']))
stamp = identity.hexdigest()
(target/'madeira_cpu_identity.h').write_text(
    '#pragma once\ninline constexpr char MadeiraCPUIdentity[] = "'+stamp+'";\n', encoding='utf-8')
print('Madeira backend cache integration ready: '+stamp[:16])
