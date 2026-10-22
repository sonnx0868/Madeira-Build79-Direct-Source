#!/usr/bin/env python3
"""Save completed compilation for diagnosis after a failed cold build."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import zipfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--root', type=Path, required=True)
parser.add_argument('--out', type=Path, required=True)
args = parser.parse_args()
root = args.root.resolve()
folders = ('app/Madeira', 'FEX/build-ios', 'build/ntdll-unix/obj',
           'build/wineserver/obj', 'build/dxmt-ios/obj', 'build/win32u-unix/obj',
           'wine/build-macos/include', 'wine/build-arm64ec/include')
selected = {}
for folder in folders:
    for path in (root / folder).rglob('*'):
        if path.is_file() and path.suffix.lower() in {'.a', '.dll', '.dylib', '.exe', '.o', '.h', '.inl', '.inc'}:
            selected[path.relative_to(root).as_posix()] = path
for folder in ('build/ntdll-unix', 'build/win32u-unix', 'build/wine-pe', 'build/wine-i386',
               'build/dxmt-ios', 'build/dxmt-pe', 'build/madsync'):
    for path in (root / folder).glob('*.json'):
        selected[path.relative_to(root).as_posix()] = path
for name in ('build/full-native-start.json', 'build/full-native-rebuild.json',
             'wine/build-macos/config.status', 'wine/build-arm64ec/config.status'):
    if (root / name).is_file(): selected[name] = root / name
record = {'format': 'madeira-failed-build-checkpoint-v1', 'releasable': False,
          'commit': subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip(),
          'files': {n: hashlib.sha256(p.read_bytes()).hexdigest() for n, p in selected.items()}}
with zipfile.ZipFile(args.out, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=3) as archive:
    for name, path in selected.items(): archive.write(path, name)
    archive.writestr('checkpoint.json', json.dumps(record, indent=2) + '\n')
print('Saved private non-releasable compilation checkpoint:', len(selected), 'files')
