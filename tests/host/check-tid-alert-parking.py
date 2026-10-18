#!/usr/bin/env python3
"""Check the shipping alert protocol with real OS waits and a lost-wake control."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--cc', default=shutil.which('clang++') or shutil.which('g++'))
parser.add_argument('--require-darwin', action='store_true')
args = parser.parse_args()
if args.require_darwin and sys.platform != 'darwin':
    raise SystemExit('Darwin kernel validation requires macOS.')
if not args.cc:
    raise SystemExit('Pass --cc with a C++20 compiler.')
root = Path(__file__).resolve().parents[2]
with tempfile.TemporaryDirectory(prefix='madeira-alert-check-') as temporary:
    executable = Path(temporary) / ('alert-check.exe' if sys.platform == 'win32' else 'alert-check')
    command = [args.cc, '-std=c++20', '-O2', '-Wall', '-Wextra', '-Werror',
               str(root/'tests/host/tid-alert-parking.cpp'), '-o', str(executable)]
    if sys.platform == 'win32':
        command += ['-static', '-D_WIN32_WINNT=0x0A00', '-lsynchronization']
    subprocess.run(command, check=True)
    good = subprocess.run([str(executable)], capture_output=True, text=True, timeout=60)
    print(good.stdout, end='')
    print(good.stderr, end='', file=sys.stderr)
    if good.returncode:
        raise SystemExit('Production handshake failed: ' + str(good.returncode))
    broken = subprocess.run([str(executable), '--broken-wake'], capture_output=True, text=True, timeout=15)
    if broken.returncode != 3 or 'lost wake: published work did not run' not in broken.stderr:
        print(broken.stdout, broken.stderr)
        raise SystemExit('The lost-wake control did not fail as expected; liveness check is invalid.')
    print('PASS: missing kernel wake control fails the liveness watchdog')
