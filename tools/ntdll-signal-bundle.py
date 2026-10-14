#!/usr/bin/env python3
"""Bind the LRCPC2 consumer receipt to the compiled signal object and source."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

MARKER = b'lrcpc2-alias-v1'
UPSTREAM = 'ce34c10dd2c59700855ec3b859fd8166e33e9170'

def sha(data):
    return hashlib.sha256(data).hexdigest()

def ar_command():
    if os.environ.get('AR'):
        return [os.environ['AR']]
    if sys.platform == 'darwin':
        return ['xcrun', 'ar']
    found = shutil.which('llvm-ar') or shutil.which('ar')
    if not found:
        raise ValueError('ar is required to verify the compiled signal object')
    return [found]

def members(archive):
    command = ar_command()
    names = subprocess.check_output([*command, 't', str(archive)], text=True).splitlines()
    assert names and len(names) == len(set(names)), 'duplicate/empty archive members'
    assert 'signal_arm64.o' in names, 'archive has no ARM64 signal handler'
    return {name: sha(subprocess.check_output([*command, 'p', str(archive), name])) for name in names}

def verify(root, archive=None, stamp=False, base=None):
    archive = archive or root / 'app/Madeira/libntdll_unix.a'
    source = root / 'build/ntdll-unix/signal_arm64_ios.c'
    receipt = root / 'build/ntdll-unix/lrcpc2-alias.json'
    assert MARKER in archive.read_bytes(), 'archive lacks the LRCPC2 consumer'
    current = members(archive)
    expected = dict(archive_sha256=sha(archive.read_bytes()),
                    source_sha256=sha(source.read_bytes().replace(b'\r\n', b'\n')),
                    object_sha256=current['signal_arm64.o'], decoder=MARKER.decode(), upstream=UPSTREAM)
    if stamp:
        document = dict(expected, producer_commit=subprocess.check_output(
            ['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip())
        if base:
            previous = members(base)
            assert current.keys() == previous.keys(), 'refresh changed archive member set'
            changed = [name for name in current if current[name] != previous[name]]
            assert changed == ['signal_arm64.o'], ('refresh changed other objects', changed)
            document['base_archive_sha256'] = sha(base.read_bytes())
            document['preserved_members'] = len(current) - 1
        receipt.write_text(json.dumps(document, indent=2) + '\n', encoding='utf-8')
        print('Stamped LRCPC2 signal consumer; other archive members verified' if base else
              'Stamped freshly built LRCPC2 signal consumer')
    else:
        document = json.loads(receipt.read_text(encoding='utf-8'))
        assert all(document.get(key) == value for key, value in expected.items()), 'stale LRCPC2 signal receipt'
        print('PASS: LRCPC2 source, archive and compiled signal object match')
    return document if not stamp else expected

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--archive', type=Path)
    parser.add_argument('--base', type=Path)
    parser.add_argument('--stamp', action='store_true')
    args = parser.parse_args()
    try:
        verify(args.root, args.archive, args.stamp, args.base)
    except (AssertionError, OSError, ValueError, subprocess.CalledProcessError) as error:
        raise SystemExit('INVALID LRCPC2 signal bundle: ' + str(error))
