#!/usr/bin/env python3
"""Exercise the production display-link mailbox and actual Metal presentation."""
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
if sys.platform != 'darwin':
    if '--require-darwin' in sys.argv: raise SystemExit('Apple QuartzCore/Metal runtime required')
    print('SKIP: display-link runtime needs macOS')
    raise SystemExit(0)
with tempfile.TemporaryDirectory(prefix='madeira-frame-scheduler-') as scratch:
    binary = Path(scratch) / 'check'
    subprocess.run(['xcrun', 'clang', '-fobjc-arc', '-fblocks', '-O2', '-mmacosx-version-min=14.0',
                    '-I' + str(root / 'tests/host'), '-framework', 'AppKit', '-framework', 'Foundation',
                    '-framework', 'QuartzCore', '-framework', 'Metal',
                    str(root / 'tests/host/metal-frame-scheduler.m'), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True, timeout=25)
