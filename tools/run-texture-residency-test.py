"""Bound native tests and retain thread stacks if a regression stalls."""
import os
from pathlib import Path
import subprocess
import sys

out = Path(sys.argv[1])
with (out/'texture-residency.json').open('w') as result:
    proc = subprocess.Popen([str(out/'texture-residency')], stdout=result,
                            env=dict(os.environ, MTL_DEBUG_LAYER='1'))
    try:
        code = proc.wait(timeout=90)
    except subprocess.TimeoutExpired:
        subprocess.run(['/usr/bin/sample', str(proc.pid), '1', '1', '-file', str(out/'timeout-stacks.txt')], timeout=15)
        proc.kill()
        code = 124
print((out/'texture-residency.json').read_text())
sys.exit(code)
