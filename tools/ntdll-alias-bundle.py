#!/usr/bin/env python3
"""Bind the native alias filter to virtual.o and preserve other native objects."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

BASE_ARCHIVE = '07251e0dcc42f0828e98372dff727a5f0fae0ca04aa219874289406436323e45'
RECEIPT = 'build/ntdll-unix/fex-alias-policy.json'
SOURCES = ('build/ntdll-unix/virtual_ios.c', 'build/ntdll-unix/ios_fex_alias_policy.h',
           'build/ntdll-unix/build.sh')


def sha(path, source=False):
    data = path.read_bytes()
    return hashlib.sha256(data.replace(b'\r\n', b'\n') if source else data).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, required=True)
    p.add_argument('--archive', type=Path)
    p.add_argument('--base', type=Path)
    p.add_argument('--stamp', action='store_true')
    a = p.parse_args()
    root = a.root.resolve()
    archive = a.archive or root / 'app/Madeira/libntdll_unix.a'
    spec = importlib.util.spec_from_file_location('sync', root / 'tools/nutssync-bundle.py')
    sync = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(sync)
    objects = sync.members(root, archive)
    assert b'fex-alias-native-filter-v1' in archive.read_bytes(), 'native alias filter missing'
    record = dict(format='fex-alias-native-filter-v1', archive_sha256=sha(archive),
                  virtual_object_sha256=objects['virtual.o'],
                  sources={s: sha(root/s, True) for s in SOURCES})
    target = root / RECEIPT
    if not a.stamp:
        saved = json.loads(target.read_text())
        for key, value in record.items():
            assert saved[key] == value, ('stale alias filter', key)
        print('PASS: native alias policy source and compiled virtual.o match')
        return
    record['producer_commit'] = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
    updates = []
    if a.base:
        assert sha(a.base) == BASE_ARCHIVE, 'incremental alias refresh requires verified873 ntdll'
        before = sync.members(root, a.base)
        assert before.keys() == objects.keys(), 'archive member set changed'
        changed = sorted(k for k in before if before[k] != objects[k])
        assert changed == ['virtual.o'], ('unexpected native change', changed)
        record.update(base_archive_sha256=sha(a.base), changed_members=changed, preserved_members=len(objects)-1)
        binding = dict(commit=record['producer_commit'], receipt=RECEIPT, changed_members=changed)
        for name in ('lrcpc2-alias.json', 'runtime-scheduling-loader.json', 'game-graphics-preferences.json'):
            path = root/'build/ntdll-unix'/name
            parent = json.loads(path.read_text())
            assert parent['archive_sha256'] == sha(a.base), ('stale parent archive', name)
            if name == 'lrcpc2-alias.json':
                assert parent['object_sha256'] == objects['signal_arm64.o']
            elif name == 'runtime-scheduling-loader.json':
                for obj, digest in parent['objects'].items(): assert objects[obj] == digest
            else:
                assert parent['registry_object_sha256'] == objects['registry.o']
            for source, digest in parent.get('inputs', parent.get('sources', {})).items():
                assert sha(root/source, True) == digest, ('unrelated parent source changed', name, source)
            parent['archive_sha256'] = record['archive_sha256']
            parent['alias_policy_refresh'] = binding
            updates.append((path, parent))
        path = root/'build/madsync/native-receipt.json'
        parent = json.loads(path.read_text())
        assert parent['archives']['ntdll']['sha256'] == sha(a.base)
        assert parent['archives']['ntdll']['members'] == before
        server = root/'app/Madeira/libwineserver.a'
        assert parent['archives']['server']['sha256'] == sha(server)
        assert parent['archives']['server']['members'] == sync.members(root, server)
        for source, digest in parent['sources'].items():
            current = sha(root/source, True)
            if current != digest:
                assert source == 'build/ntdll-unix/build.sh', ('unrelated sync source changed', source)
                parent['sources'][source] = current
        parent['archives']['ntdll']['sha256'] = record['archive_sha256']
        parent['archives']['ntdll']['members'] = objects
        parent['alias_policy_refresh'] = binding
        updates.append((path, parent))
    updates.append((target, record))
    for path, content in updates:
        path.write_text(json.dumps(content, indent=2)+'\n', encoding='utf-8')
    print('PASS: alias filter stamped; virtual.o is the only refreshed archive member' if a.base else
          'PASS: full source alias-filter build stamped')


if __name__ == '__main__':
    main()
