#!/usr/bin/env python3
"""Bind the PE-only builtin loader and timed-sleep changes to native objects."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

def sha(data):
    return hashlib.sha256(data).hexdigest()

def verify(root, archive=None, stamp=False, base=None, sync_refresh=False):
    archive = archive or root / 'app/Madeira/libntdll_unix.a'
    spec = importlib.util.spec_from_file_location('signal_bundle', root / 'tools/ntdll-signal-bundle.py')
    helper = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(helper)
    objects = helper.members(archive)
    for marker in (b'builtin-pe-v1', b'timed-sleep-v1', b'darwin-yield-v1', b'tid-alert-park-v1', b'yield-budget-v3',
                   b'madeira_session_priority_poll_current'):
        assert marker in archive.read_bytes(), 'native archive predates ' + marker.decode()
    files = ['build/ntdll-unix/loader_ios.c', 'build/ntdll-unix/ios_builtin_pe.h', 'build/ntdll-unix/ios_scheduler_yield.h',
             'build/ntdll-unix/ios_yield_policy.h', 'patches/wine-yield-budget.patch',
             'build/ntdll-unix/ios_session_priority.h', 'patches/wine-session-priority.patch',
             'wine/dlls/ntdll/unix/sync.c', 'patches/wine-timed-sleep.patch',
             'build/ntdll-unix/shims/ios_tid_alert.h', 'patches/wine-tid-alert-parking.patch']
    expected = {'archive_sha256': sha(archive.read_bytes()),
                'objects': {name: objects[name] for name in ('loader.o', 'sync.o')},
                'inputs': {name: sha((root/name).read_bytes().replace(b'\r\n', b'\n')) for name in files}}
    receipt = root / 'build/ntdll-unix/runtime-scheduling-loader.json'
    if not stamp:
        actual = json.loads(receipt.read_text(encoding='utf-8'))
        assert all(actual.get(k) == v for k, v in expected.items()), 'stale runtime loader/scheduling receipt'
        print('PASS: timed-sleep and builtin PE source, native objects and archive match')
        return actual
    document = dict(expected, producer_commit=subprocess.check_output(
        ['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip())
    if base:
        previous = helper.members(base)
        assert objects.keys() == previous.keys(), 'runtime refresh changed archive member set'
        changed = sorted(n for n in objects if objects[n] != previous[n])
        allowed = {'madsync.o', 'sync.o', 'thread.o'} if sync_refresh else {'loader.o', 'sync.o'}
        assert 'sync.o' in changed and set(changed) <= allowed, ('unexpected native refresh', changed)
        document['base_archive_sha256'] = sha(base.read_bytes())
        document['changed_members'] = changed
        document['preserved_members'] = len(objects)-len(changed)
    # The old signal-handler producer remains authoritative. Only its archive
    # container changed; require its exact compiled object to be preserved.
    signal_path = root / 'build/ntdll-unix/lrcpc2-alias.json'
    signal = json.loads(signal_path.read_text(encoding='utf-8'))
    assert signal['object_sha256'] == objects['signal_arm64.o'], 'signal handler changed'
    assert signal['archive_sha256'] == sha((base or archive).read_bytes()), 'unexpected signal receipt container'
    signal['archive_sha256'] = expected['archive_sha256']
    signal['container_updated_by'] = document['producer_commit']
    signal_path.write_text(json.dumps(signal, indent=2)+'\n', encoding='utf-8')
    receipt.write_text(json.dumps(document, indent=2)+'\n', encoding='utf-8')
    print('PASS: only scheduling/builtin loader objects refreshed; all other archive objects preserved' if base else
          'Stamped freshly built timed-sleep and builtin PE runtime')
    return document

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--archive', type=Path)
    parser.add_argument('--base', type=Path)
    parser.add_argument('--stamp', action='store_true')
    parser.add_argument('--sync-refresh', action='store_true')
    args = parser.parse_args()
    try:
        verify(args.root, args.archive, args.stamp, args.base, args.sync_refresh)
    except (AssertionError, OSError, ValueError, subprocess.CalledProcessError) as error:
        raise SystemExit('INVALID scheduling/builtin native bundle: '+str(error))
