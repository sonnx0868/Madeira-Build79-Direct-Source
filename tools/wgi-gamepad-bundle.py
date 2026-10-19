#!/usr/bin/env python3
"""Bind the WGI host adapter's compiled DLL to its owned patch and Wine pin."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--root', type=Path, required=True)
parser.add_argument('--stamp', action='store_true')
args = parser.parse_args()
root = args.root.resolve()
receipt = root / 'build/wine-pe/wgi-host-gamepad.json'
binary = root / 'app/Madeira/arm64ec-windows/windows.gaming.input.dll'
patch = root / 'patches/wine-wgi-host-gamepad.patch'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


try:
    data = binary.read_bytes()
    if b'host-gamepad-v1' not in data or b'NtUserCallTwoParam\0' not in data:
        raise ValueError('WGI DLL has no compiled host provider; rebuild with build-wgi.sh')
    wine_pin = subprocess.check_output(['git', '-C', str(root / 'wine'), 'rev-parse', 'HEAD'], text=True).strip()
    expected = {'schema': 1, 'policy': 'host-gamepad-v1', 'wineCommit': wine_pin,
                'patchSHA256': digest(patch), 'dllSHA256': digest(binary)}
    if args.stamp:
        subprocess.run(['git', '-C', str(root / 'wine'), 'apply', '--reverse', '--check', str(patch)],
                       check=True, stdout=subprocess.DEVNULL)
        receipt.write_text(json.dumps(expected, indent=2) + '\n', encoding='utf-8')
    elif json.loads(receipt.read_text(encoding='utf-8')) != expected:
        raise ValueError('WGI receipt does not match the current patch and compiled DLL; rebuild with build-wgi.sh')
except (OSError, ValueError, subprocess.CalledProcessError) as error:
    raise SystemExit(f'INVALID WGI host gamepad bundle: {error}')
print('PASS: compiled WGI host adapter, Wine pin, patch and DLL digest')
