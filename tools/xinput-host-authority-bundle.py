#!/usr/bin/env python3
"""Bind the XInput host authority patch to its PE DLLs and native win32u archive."""
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
receipt = root / 'build/wine-pe/xinput-host-authority.json'
patch = root / 'patches/wine-xinput-host-authority.patch'
driver = root / 'build/win32u-unix/driver_ios.c'
native = root / 'app/Madeira/libwin32u_unix.a'
marker = b'xinput-host-authority-v1'
modules = ('xinput1_1', 'xinput1_2', 'xinput1_3', 'xinput1_4', 'xinput9_1_0')


def digest(path, source=False):
    data = path.read_bytes()
    if source:
        data = data.replace(b'\r\n', b'\n')
    return hashlib.sha256(data).hexdigest()


try:
    dlls = {}
    for name in modules:
        binary = root / f'app/Madeira/arm64ec-windows/{name}.dll'
        data = binary.read_bytes()
        if name == 'xinput9_1_0':
            if 'xinput1_4.dll\0'.encode('utf-16-le') not in data or b'XInputGetState\0' not in data:
                raise ValueError('xinput9_1_0 has no xinput1_4 forwarding implementation')
        elif marker not in data or b'NtUserCallTwoParam\0' not in data:
            raise ValueError(f'{name}.dll has no compiled host authority guard')
        dlls[name + '.dll'] = digest(binary)
    if marker not in native.read_bytes():
        raise ValueError('native win32u has no backend capability; refresh driver_ios.c first')
    pin = subprocess.check_output(['git', '-C', str(root / 'wine'), 'rev-parse', 'HEAD'], text=True).strip()
    expected = {'schema': 1, 'policy': marker.decode('ascii'), 'wineCommit': pin,
                'patchSHA256': digest(patch, True), 'driverSHA256': digest(driver, True),
                'nativeArchiveSHA256': digest(native), 'dllSHA256': dlls}
    if args.stamp:
        subprocess.run(['git', '-C', str(root / 'wine'), 'apply', '--reverse', '--check', str(patch)],
                       check=True, stdout=subprocess.DEVNULL)
        receipt.write_text(json.dumps(expected, indent=2) + '\n', encoding='utf-8')
    elif json.loads(receipt.read_text(encoding='utf-8')) != expected:
        raise ValueError('receipt differs from current source/native/PE; rebuild the controller bridge')
except (OSError, ValueError, subprocess.CalledProcessError) as error:
    raise SystemExit(f'INVALID XInput host authority bundle: {error}')
print('PASS: XInput host authority native/PE markers, source receipts and binary digests')
