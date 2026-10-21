#!/usr/bin/env python3
"""Rebuild only the BC pager in the verified 894 native archive.

The shader compiler, all other archive objects, Windows payloads and existing
producer source bindings are checked before and after replacement. This does
not authorize refreshing arbitrary sources or merely relabelling an old object.
"""
from pathlib import Path
import argparse
import copy
import importlib.util
import subprocess

BASE_ARCHIVE = 'e9914c5e357e6f185497620437398cf17b6ab57d0a7db147d4d03d5a7abaaefa'
BASE_OBJECT = '469d90de4bd499001f6264d4dcd9e5363b8c5c462546b826e203e4390c772f40'
BASE_PAGER = '61faba3b67cb5397707fea5d6a6c20b68763e2264634730e59a10d53a69f6cb4'
PAGER = 'build/dxmt-ios/texture_residency.m'
ARCHIVE = 'app/Madeira/libdxmt_combined.a'
FOLDER = 'build/dxmt-ios/residency-drain-refresh'
RECEIPT = 'build/dxmt-ios/residency-drain.json'
SOURCES = (PAGER, 'tools/dxmt-residency-drain-bundle.py',
           'build/dxmt-ios/refresh-residency-drain.sh')


def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def validated_base(root, state, parents, receipt, boundary):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    residency = module(root, 'dxmt-residency-bundle')
    optimizer.validate_parents(root, parents, state)
    optimizer.validate_depth_record(root, state, parents)
    optimizer.validate_renpy(root)
    assert state['native_archives'][ARCHIVE] == BASE_ARCHIVE, 'requires verified894 archive'
    assert optimizer.digest(boundary) == BASE_OBJECT, 'unexpected base pager implementation'
    expected_sources = {name: optimizer.source_sha(root / name) for name in residency.SOURCES}
    expected_sources[PAGER] = BASE_PAGER
    assert receipt['format'] == 'madeira-texture-residency-v1'
    assert receipt['sources'] == expected_sources, 'unrelated residency source change'
    assert receipt['archive_sha256'] == BASE_ARCHIVE and receipt['members'] == state['members']
    assert receipt['object_proof'] == residency.object_proof(boundary, optimizer)


def prepare(root):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    residency = module(root, 'dxmt-residency-bundle')
    depth = module(root, 'dxmt-depth-retention-bundle')
    upload = module(root, 'dxmt-upload-batching-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    state = optimizer.collect(root)
    parents = depth.read_parents(root)
    receipt = optimizer.read_json(root / residency.RECEIPT)
    validated_base(root, state, parents, receipt, srv.object_bytes(root / ARCHIVE, 'winemetal_unix.o'))
    saved_upload = optimizer.read_json(root / upload.RECEIPT)
    expected_upload = upload.record_for(root, upload.collect(root))
    assert all(saved_upload.get(k) == v for k, v in expected_upload.items()), 'stale upload input'
    optimizer.write_json(root / FOLDER / 'prepared.json', {
        'format': 'madeira-residency-drain-base-v1', 'state': state, 'parents': parents,
        'residency': receipt, 'upload': saved_upload,
        'sources': {name: optimizer.source_sha(root / name) for name in SOURCES}})


def transition(root, snapshot, base, compiled, commit):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    residency = module(root, 'dxmt-residency-bundle')
    depth = module(root, 'dxmt-depth-retention-bundle')
    upload = module(root, 'dxmt-upload-batching-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    pacing = module(root, 'dxmt-pacing-bundle')
    assert snapshot['format'] == 'madeira-residency-drain-base-v1'
    old = snapshot['state']
    assert optimizer.sha(base) == BASE_ARCHIVE
    assert pacing.members(base) == old['members']
    validated_base(root, old, snapshot['parents'], snapshot['residency'],
                   srv.object_bytes(base, 'winemetal_unix.o'))
    assert snapshot['sources'] == {name: optimizer.source_sha(root / name) for name in SOURCES}
    parents = depth.read_parents(root)
    assert parents == snapshot['parents'], 'producer receipts changed during compile'
    assert optimizer.read_json(root / residency.RECEIPT) == snapshot['residency']
    assert optimizer.read_json(root / upload.RECEIPT) == snapshot['upload']
    state = optimizer.collect(root)
    for key in old:
        if key not in ('native_archives', 'members'):
            assert state[key] == old[key], ('unrelated state changed', key)
    assert set(state['native_archives']) == set(old['native_archives'])
    assert [name for name in sorted(old['native_archives'])
            if old['native_archives'][name] != state['native_archives'][name]] == [ARCHIVE]
    before, after = old['members'], state['members']
    assert [(m['name'], m['occurrence']) for m in before] == [(m['name'], m['occurrence']) for m in after]
    assert [new['name'] for previous, new in zip(before, after) if previous != new] == ['winemetal_unix.o']
    assert sum(m['name'] == 'winemetal_unix.o' for m in after) == 1
    boundary = srv.object_bytes(root / ARCHIVE, 'winemetal_unix.o')
    assert boundary == compiled.read_bytes() and optimizer.digest(boundary) != BASE_OBJECT
    proof = residency.object_proof(boundary, optimizer)
    # The exact compiled boundary is bound to this source snapshot; native Metal
    # tests verify the scheduler, resource pinning and restoration semantics.
    binding = {'commit': commit, 'receipt': RECEIPT, 'base_archive_sha256': BASE_ARCHIVE,
               'changed_objects': ['winemetal_unix.o']}
    updates = {}
    for name in depth.NATIVE_PARENTS:
        updated = copy.deepcopy(parents[name])
        updated.update(archive_sha256=state['native_archives'][ARCHIVE], members=after,
                       residency_drain_refresh=binding)
        updates[name] = updated
    combined = dict(parents, **updates)
    optimizer.validate_parents(root, combined, state)
    optimizer.validate_renpy(root)
    updates[depth.RECEIPT] = dict(optimizer.read_json(root / depth.RECEIPT),
                                 **depth.record_for(state, combined),
                                 residency_drain_refresh=binding)
    upload_record = upload.record_for(root, upload.collect(root))
    for key, value in upload_record.items():
        if key not in ('archive_sha256', 'members'):
            assert snapshot['upload'][key] == value, ('unrelated upload implementation changed', key)
    updates[upload.RECEIPT] = dict(snapshot['upload'], archive_sha256=state['native_archives'][ARCHIVE],
                                  members=after, residency_drain_refresh=binding)
    updates[residency.RECEIPT] = dict(snapshot['residency'],
        sources={name: optimizer.source_sha(root / name) for name in residency.SOURCES},
        archive_sha256=state['native_archives'][ARCHIVE], members=after,
        object_proof=proof, residency_drain_refresh=binding)
    updates[RECEIPT] = {'format': 'madeira-residency-drain-v1', 'producer_commit': commit,
        'sources': snapshot['sources'], 'archive_sha256': state['native_archives'][ARCHIVE],
        'object_sha256': proof['sha256'], 'base_archive_sha256': BASE_ARCHIVE,
        'changed_objects': ['winemetal_unix.o'], 'max_textures_per_turn': 8,
        'max_reclaimed_bytes_per_turn': 128*1024*1024, 'turn_time_budget_ms': 25,
        'continuation_delay_ms': 50, 'shader_namespace_changed': False}
    return updates


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
        commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        updates = transition(root, snapshot, args.base, root / FOLDER / 'winemetal_unix.o', commit)
        module(root, 'dxmt-upload-batching-bundle').write_updates(root, updates)
        module(root, 'dxmt-residency-bundle').verify(root)
    else:
        module(root, 'dxmt-residency-bundle').verify(root)
        record = optimizer.read_json(root / RECEIPT)
        assert record['format'] == 'madeira-residency-drain-v1'
        assert record['sources'] == {name: optimizer.source_sha(root / name) for name in SOURCES}
        assert record['archive_sha256'] == optimizer.sha(root / ARCHIVE)
        boundary = module(root, 'dxmt-srv-bundle').object_bytes(root / ARCHIVE, 'winemetal_unix.o')
        assert record['object_sha256'] == optimizer.digest(boundary)
    print('PASS: bounded BC drain and preserved native/PE/source identities verified')


if __name__ == '__main__':
    main()
