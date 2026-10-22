#!/usr/bin/env python3
"""Replace only the verified 883 Metal boundary, preserving every PE payload.

The reversible overlay must match the original producers' source hashes before
compilation. Receipt rebinding happens only after checking the compiled object,
the complete archive inventory and all Windows payloads against that snapshot.
"""
from pathlib import Path
import argparse
import copy
import importlib.util
import re
import struct
import subprocess
import tempfile

ARCHIVE = 'app/Madeira/libdxmt_combined.a'
BASE_ARCHIVE = '44002565405011d3993f0fef418dcba8d0fbc131277879e3e111384b79d8c626'
BOUNDARY = 'dxmt/src/winemetal/unix/winemetal_unix.c'
PATCH = 'patches/dxmt-texture-residency.patch'
RECEIPT = 'build/dxmt-ios/texture-residency.json'
FOLDER = 'build/dxmt-ios/residency-refresh'
SOURCES = (BOUNDARY, PATCH, 'build/dxmt-ios/texture_residency.m',
           'build/dxmt-ios/refresh-residency.sh', 'tools/dxmt-residency-bundle.py')


def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def source_state(root, optimizer):
    sources = {name: optimizer.source_sha(root / name) for name in SOURCES}
    data = (root / PATCH).read_bytes().replace(b'\r\n', b'\n')
    assert re.findall(rb'^\+\+\+ b/([^\r\n]+)$', data, re.M) == [BOUNDARY.removeprefix('dxmt/').encode()]
    with tempfile.TemporaryDirectory(prefix='madeira-residency-overlay-') as work:
        folder = Path(work)
        source = folder / BOUNDARY.removeprefix('dxmt/')
        source.parent.mkdir(parents=True)
        source.write_bytes((root / BOUNDARY).read_bytes().replace(b'\r\n', b'\n'))
        patch = folder / 'residency.patch'
        patch.write_bytes(data)
        padded = b'padded_lighting.m' in source.read_bytes()
        padded_patch = folder / 'padded.patch'
        if padded:
            padded_patch.write_bytes((root / 'patches/dxmt-padded-light-table.patch').read_bytes().replace(b'\r\n', b'\n'))
            subprocess.run(['git', '-C', str(folder), 'apply', '--reverse',
                            '--include=src/winemetal/unix/winemetal_unix.c', str(padded_patch)],
                           check=True, capture_output=True)
        subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', str(patch)], check=True, capture_output=True)
        previous = optimizer.source_sha(source)
        subprocess.run(['git', '-C', str(folder), 'apply', str(patch)], check=True, capture_output=True)
        if padded:
            subprocess.run(['git', '-C', str(folder), 'apply',
                            '--include=src/winemetal/unix/winemetal_unix.c', str(padded_patch)],
                           check=True, capture_output=True)
        assert optimizer.source_sha(source) == sources[BOUNDARY]
    return sources, previous


def collect(root):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    state = optimizer.collect(root)
    state['residency_sources'], state['pre_residency_boundary'] = source_state(root, optimizer)
    return state


def validated_parents(root, parents, state, previous=False):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    adjusted = copy.deepcopy(parents)
    for name, parent in adjusted.items():
        if previous and BOUNDARY in parent['sources']:
            assert parent['sources'][BOUNDARY] == state['pre_residency_boundary'], ('unrelated boundary change', name)
            parent['sources'][BOUNDARY] = state['residency_sources'][BOUNDARY]
    optimizer.validate_parents(root, adjusted, state)
    optimizer.validate_renpy(root)
    return adjusted


def object_proof(data, optimizer):
    assert struct.unpack_from('<4I', data)[:2] == (0xfeedfacf, 0x100000c), 'not ARM64 Mach-O'
    assert struct.unpack_from('<4I', data)[3] == 1, 'not a relocatable object'
    # Object inclusion, not a semantic claim. Native Metal tests exercise the
    # exact included implementation, with indirect views and concurrent writes.
    # The private checkpoint/restore functions may be inlined at -O2. Verify
    # actual defined Objective-C implementations, not optional debug names.
    commands, command_bytes = struct.unpack_from('<2I', data, 16)
    at, defined = 32, set()
    assert at + command_bytes <= len(data)
    for _ in range(commands):
        command, length = struct.unpack_from('<2I', data, at)
        assert length >= 8 and at + length <= 32 + command_bytes
        if command == 2:
            offset, count, strings, string_bytes = struct.unpack_from('<4I', data, at + 8)
            assert offset + count*16 <= len(data) and strings + string_bytes <= len(data)
            for index in range(count):
                name_at, kind, section, _, _ = struct.unpack_from('<IBBHQ', data, offset + index*16)
                assert name_at < string_bytes
                end = data.find(b'\0', strings + name_at, strings + string_bytes)
                assert end >= 0
                if not kind & 0xe0 and kind & 0x0e == 0x0e and section:
                    defined.add(data[strings + name_at:end].decode())
        at += length
    assert at == 32 + command_bytes
    required = {'_OBJC_CLASS_$_MDRTexture', '_OBJC_CLASS_$_MDRLease',
                '-[MDRTexture dropBacking]', '-[MDRLease finish]'}
    assert required <= defined, ('missing defined residency implementation', required-defined)
    assert b'texture-residency-v1' in data
    return {'format': 'arm64-mach-o-residency-v1', 'sha256': optimizer.digest(data),
            'defined_symbols': sorted(required)}


def prepare(root):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    depth = module(root, 'dxmt-depth-retention-bundle')
    upload = module(root, 'dxmt-upload-batching-bundle')
    state = collect(root)
    assert state['native_archives'][ARCHIVE] == BASE_ARCHIVE, 'requires the verified883 native input'
    parents = depth.read_parents(root)
    validated_parents(root, parents, state, previous=True)
    optimizer.validate_depth_record(root, state, parents)
    saved_upload = optimizer.read_json(root / upload.RECEIPT)
    expected_upload = upload.record_for(root, upload.collect(root))
    assert all(saved_upload.get(k) == v for k, v in expected_upload.items()), 'stale upload implementation'
    snapshot = {'format': 'madeira-residency-base-v1', 'state': state, 'parents': parents,
                'depth': optimizer.read_json(root / depth.RECEIPT), 'upload': saved_upload}
    optimizer.write_json(root / FOLDER / 'prepared.json', snapshot)


def transition(root, snapshot, state, parents, base, compiled, commit):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    depth = module(root, 'dxmt-depth-retention-bundle')
    upload = module(root, 'dxmt-upload-batching-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    pacing = module(root, 'dxmt-pacing-bundle')
    assert snapshot['format'] == 'madeira-residency-base-v1'
    old = snapshot['state']
    assert optimizer.sha(base) == old['native_archives'][ARCHIVE] == BASE_ARCHIVE
    assert pacing.members(base) == old['members']
    assert parents == snapshot['parents'], 'parent receipts changed during compilation'
    for key in ('receipt_hashes', 'pe_payloads', 'pe_libraries', 'swapchain_entries', 'sources',
                'pre_patch_sources', 'depth_symbols', 'optimizer_sources', 'residency_sources',
                'pre_residency_boundary'):
        assert state[key] == old[key], ('unrelated state changed', key)
    assert set(state['native_archives']) == set(old['native_archives'])
    assert [name for name in sorted(state['native_archives'])
            if state['native_archives'][name] != old['native_archives'][name]] == [ARCHIVE]
    before, after = old['members'], state['members']
    assert [(m['name'], m['occurrence']) for m in before] == [(m['name'], m['occurrence']) for m in after]
    changed = [new['name'] for previous, new in zip(before, after) if previous != new]
    assert changed == ['winemetal_unix.o'], ('unexpected changed objects', changed)
    assert sum(m['name'] == 'winemetal_unix.o' for m in after) == 1
    data = srv.object_bytes(root / ARCHIVE, 'winemetal_unix.o')
    assert data == compiled.read_bytes(), 'archive does not contain the freshly compiled boundary'
    proof = object_proof(data, optimizer)
    validated_parents(root, parents, old, previous=True)
    optimizer.validate_depth_record(root, old, parents, snapshot['depth'])
    binding = {'commit': commit, 'receipt': RECEIPT, 'base_archive_sha256': BASE_ARCHIVE,
               'changed_objects': changed}
    updates = {}
    for name, parent in parents.items():
        updated = copy.deepcopy(parent)
        if BOUNDARY in updated['sources']:
            assert updated['sources'][BOUNDARY] == old['pre_residency_boundary']
            updated['sources'][BOUNDARY] = state['residency_sources'][BOUNDARY]
        if name in depth.NATIVE_PARENTS:
            updated.update(archive_sha256=state['native_archives'][ARCHIVE], members=after)
        if updated != parent:
            updated['residency_refresh'] = binding
            updates[name] = updated
    combined = dict(parents, **updates)
    validated_parents(root, combined, state)
    for name, original, expected in (
        (depth.RECEIPT, snapshot['depth'], depth.record_for(state, combined)),
        (upload.RECEIPT, snapshot['upload'], upload.record_for(root, upload.collect(root)))):
        assert optimizer.read_json(root / name) == original
        for key, value in expected.items():
            if key not in ('archive_sha256', 'members'):
                assert original[key] == value, ('unrelated inherited implementation changed', name, key)
        updates[name] = dict(original, archive_sha256=state['native_archives'][ARCHIVE],
                             members=after, residency_refresh=binding)
    updates[RECEIPT] = {'format': 'madeira-texture-residency-v1', 'producer_commit': commit,
        'sources': state['residency_sources'], 'archive_sha256': state['native_archives'][ARCHIVE],
        'members': after, 'object_proof': proof, 'base_archive_sha256': BASE_ARCHIVE,
        'changed_objects': changed, 'preserved_pe_payloads': len(state['pe_payloads']),
        'preserved_archive_members': len(after)-1, 'shader_namespace_changed': False}
    return updates


def verify(root):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    optimizer.verify(root)
    state = collect(root)
    saved = optimizer.read_json(root / RECEIPT)
    assert saved['format'] == 'madeira-texture-residency-v1'
    for key, value in {'sources': state['residency_sources'],
                       'archive_sha256': state['native_archives'][ARCHIVE], 'members': state['members'],
                       'object_proof': object_proof(srv.object_bytes(root / ARCHIVE, 'winemetal_unix.o'), optimizer)}.items():
        assert saved[key] == value, ('stale residency receipt', key)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--prepare', action='store_true')
    parser.add_argument('--stamp', action='store_true')
    parser.add_argument('--base', type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    assert not (args.prepare and args.stamp)
    optimizer = module(root, 'dxmt-optimizer-bundle')
    if args.prepare:
        prepare(root)
    elif args.stamp:
        assert args.base
        snapshot = optimizer.read_json(root / FOLDER / 'prepared.json')
        parents = module(root, 'dxmt-depth-retention-bundle').read_parents(root)
        commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        updates = transition(root, snapshot, collect(root), parents, args.base,
                             root / FOLDER / 'winemetal_unix.o', commit)
        module(root, 'dxmt-upload-batching-bundle').write_updates(root, updates)
        verify(root)
    else:
        verify(root)
    print('PASS: residency boundary and inherited native/PE/source identities verified')


if __name__ == '__main__':
    main()
