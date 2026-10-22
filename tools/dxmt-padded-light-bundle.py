#!/usr/bin/env python3
"""Replace four native objects in verified896; preserve all guest payloads.

Accept only the explicitly reversible native/writer/verifier overlays. Existing
producer identities remain, with this new component binding the replacement
objects, actual compiler namespace and complete before/after inventory.
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
BASE_ARCHIVE = '967211af9fbd3619f41b5a96624f5236d9db36e17f6b25b00163f985dcf04356'
FOLDER = 'build/dxmt-ios/padded-light-refresh'
RECEIPT = 'build/dxmt-ios/padded-lighting.json'
CONTEXT = 'dxmt/src/airconv/airconv_context.cpp'
BOUNDARY = 'dxmt/src/winemetal/unix/winemetal_unix.c'
WRITER = 'dxmt/src/airconv/metallib_writer.cpp'
CHANGED = ('airconv_context.o', 'cache.o', 'metallib_writer.o', 'winemetal_unix.o')
BASE_SOURCES = {
    CONTEXT: 'cf62c823875ec1aac79864d1e3fe543c95e4cb1479809033fb6132ad7b58d66c',
    BOUNDARY: 'fd06b5fd78b281387441177b3f6962c7e0b6a559ef5b3d2d71a60c8f18ae56dc',
    WRITER: '0d893c8e92fd5fd9d19d1442a19849376174bf22b0ae1d87ec11cd3a013b1239',
    'tools/dxmt-residency-bundle.py': 'c8d54f8ce98a6883eded1eb710902ceb939d984100163345babf618c9b0cf1a7',
    'tools/dxmt-optimizer-bundle.py': 'e11aa586ad3b2838b18ff58267948457b191eabf27011906653ed48fc3942148'}
SOURCES = ('build/dxmt-ios/padded_lighting.hpp', 'build/dxmt-ios/padded_lighting.m',
           'patches/dxmt-padded-light-table.patch', 'patches/dxmt-multikernel-metallib.patch',
           'patches/dxmt-padded-verifier-hooks.patch',
           'dxmt/src/airconv/airconv_context.cpp', 'dxmt/src/airconv/metallib_writer.cpp',
           'dxmt/src/winemetal/unix/winemetal_unix.c', 'tools/dxmt-residency-bundle.py',
           'tools/dxmt-optimizer-bundle.py',
           'tools/dxmt-padded-light-bundle.py', 'build/dxmt-ios/refresh-padded-light.sh',
           'tests/host/padded-lighting-driver.cpp', 'tests/host/padded-lighting.m',
           'scripts/check-padded-lighting.sh')


def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result); return result


def source_state(root, optimizer):
    after = {name: optimizer.source_sha(root / name) for name in SOURCES}
    with tempfile.TemporaryDirectory(prefix='madeira-padded-overlay-') as work:
        folder = Path(work)
        for name in BASE_SOURCES:
            target = folder / name.removeprefix('dxmt/')
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes((root / name).read_bytes().replace(b'\r\n', b'\n'))
        patches = ('patches/dxmt-padded-light-table.patch', 'patches/dxmt-multikernel-metallib.patch',
                   'patches/dxmt-padded-verifier-hooks.patch')
        for index, name in enumerate(patches):
            target = folder / ('overlay-' + str(index) + '.patch')
            target.write_bytes((root / name).read_bytes().replace(b'\r\n', b'\n'))
            subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', str(target)], check=True, capture_output=True)
        before = {name: optimizer.source_sha(folder / name.removeprefix('dxmt/')) for name in BASE_SOURCES}
        assert before == BASE_SOURCES, 'native overlay hides an unrelated change to verified896'
        for index in reversed(range(len(patches))):
            subprocess.run(['git', '-C', str(folder), 'apply', str(folder / ('overlay-' + str(index) + '.patch'))],
                           check=True, capture_output=True)
        assert all(optimizer.source_sha(folder / name.removeprefix('dxmt/')) == after[name] for name in BASE_SOURCES)
    return after


def rebound_sources(root, record, optimizer):
    result = copy.deepcopy(record)
    for name, old in result.get('sources', {}).items():
        current = optimizer.source_sha(root / name)
        if old != current:
            assert name in BASE_SOURCES and old == BASE_SOURCES[name], ('unrelated input source changed', name)
            result['sources'][name] = current
    return result


def defined_symbols(data):
    assert len(data) >= 32
    magic, cpu, _, kind, count, size, _, _ = struct.unpack_from('<8I', data)
    assert (magic, cpu, kind) == (0xfeedfacf, 0x100000c, 1), 'expected ARM64 Mach-O object'
    at = 32; names = []
    for _ in range(count):
        command, length = struct.unpack_from('<II', data, at)
        assert length >= 8 and at + length <= 32 + size <= len(data)
        if command == 2:
            offset, number, strings, string_size = struct.unpack_from('<4I', data, at + 8)
            assert offset + number * 16 <= len(data) and strings + string_size <= len(data)
            for index in range(number):
                name_at, symbol_type, section, _, _ = struct.unpack_from('<IBBHQ', data, offset + index * 16)
                assert name_at < string_size
                end = data.find(b'\0', strings + name_at, strings + string_size); assert end >= 0
                if not symbol_type & 0xe0 and symbol_type & 0x0e == 0x0e and section:
                    names.append(data[strings + name_at:end].decode('utf8'))
        at += length
    assert at == 32 + size
    return sorted(names)


def object_proof(objects, optimizer):
    result = {'format': 'madeira-native-padded-light-objects-v1',
              'sha256': {name: optimizer.digest(data) for name, data in objects.items()},
              'semantics_proved_by_object_scan': False}
    context = objects['airconv_context.o']
    optimizer.replacement_proof(context)
    result['padded_run_symbols'] = [n for n in defined_symbols(context) if 'ComputePaddedLightTablePass' in n and '3runE' in n]
    assert result['padded_run_symbols'], 'missing compiled optional padded pass'
    assert b'madeira_lighting_padded_f4c2c563' in context
    boundary = objects['winemetal_unix.o']; symbols = defined_symbols(boundary)
    for name in ('MPLPlan', 'MPLLease', 'MPLAllocation', 'MPLEncoder'):
        assert '_OBJC_CLASS_$_' + name in symbols, ('missing native ownership implementation', name)
    for marker in (b'madeira_prepare_padded_lights', b'madeira_lighting_padded_f4c2c563',
                   b'padded light table active', b'UIApplicationDidReceiveMemoryWarningNotification'):
        assert marker in boundary, ('missing native padded-light implementation', marker)
    assert any('MetallibWriter5Write' in n for n in defined_symbols(objects['metallib_writer.o']))
    result['residency'] = module(Path(__file__).resolve().parents[1], 'dxmt-residency-bundle').object_proof(boundary, optimizer)
    return result


def read_inputs(root, optimizer):
    depth = module(root, 'dxmt-depth-retention-bundle')
    records = depth.read_parents(root)
    for name in (depth.RECEIPT, 'build/dxmt-pe/upload-batching.json',
                 'build/dxmt-ios/texture-residency.json', 'build/dxmt-ios/residency-drain.json'):
        records[name] = optimizer.read_json(root / name)
    return records


def validate_base(root, state, records, optimizer):
    assert state['native_archives'][ARCHIVE] == BASE_ARCHIVE, 'requires complete verified896 native input'
    depth = module(root, 'dxmt-depth-retention-bundle'); srv = module(root, 'dxmt-srv-bundle')
    parents = {name: rebound_sources(root, records[name], optimizer) for name in depth.PARENTS}
    optimizer.validate_parents(root, parents, state)
    optimizer.validate_depth_record(root, state, parents, records[depth.RECEIPT])
    optimizer.validate_renpy(root)
    data = srv.object_bytes(root / ARCHIVE, 'airconv_context.o')
    old = records[optimizer.RECEIPT]
    assert old['object_proof'] == optimizer.replacement_proof(data)
    assert old['compilerID'] == optimizer.digest(('madeira-compute-pcf-v1\0' + old['oldCompilerID'] + '\0' + optimizer.digest(data)).encode())
    residency = module(root, 'dxmt-residency-bundle'); saved = records[residency.RECEIPT]
    adjusted = rebound_sources(root, saved, optimizer)
    assert adjusted['sources'] == {name: optimizer.source_sha(root / name) for name in residency.SOURCES}
    boundary = srv.object_bytes(root / ARCHIVE, 'winemetal_unix.o')
    assert saved['archive_sha256'] == BASE_ARCHIVE and saved['members'] == state['members']
    assert saved['object_proof'] == residency.object_proof(boundary, optimizer)
    drain = module(root, 'dxmt-residency-drain-bundle'); saved = records[drain.RECEIPT]
    assert saved['sources'] == {name: optimizer.source_sha(root / name) for name in drain.SOURCES}
    assert saved['archive_sha256'] == BASE_ARCHIVE and saved['object_sha256'] == optimizer.digest(boundary)
    upload = module(root, 'dxmt-upload-batching-bundle')
    assert all(records[upload.RECEIPT].get(k) == v for k, v in upload.record_for(root, upload.collect(root)).items())


def prepare(root, context):
    optimizer = module(root, 'dxmt-optimizer-bundle')
    sources = source_state(root, optimizer); state = optimizer.collect(root); records = read_inputs(root, optimizer)
    validate_base(root, state, records, optimizer)
    data = context.read_bytes(); proof = optimizer.replacement_proof(data)
    assert any('ComputePaddedLightTablePass' in n and '3runE' in n for n in defined_symbols(data))
    old = records[optimizer.RECEIPT]['compilerID']
    identity = optimizer.digest(('madeira-compute-pcf-v1\0' + old + '\0' + proof['contextObjectSHA256']).encode())
    assert identity != old
    snapshot = {'format': 'madeira-padded-light-base-v1', 'sources': sources, 'state': state,
                'records': records, 'context_sha256': proof['contextObjectSHA256'],
                'oldCompilerID': old, 'compilerID': identity}
    optimizer.write_json(root / FOLDER / 'prepared.json', snapshot)
    (root / FOLDER / 'madeira_shader_compiler_identity.h').write_text(
        '#pragma once\n#define MADEIRA_SHADER_COMPILER_ID "' + identity + '"\n', encoding='utf8')


def stamp(root, base):
    optimizer = module(root, 'dxmt-optimizer-bundle'); srv = module(root, 'dxmt-srv-bundle')
    depth = module(root, 'dxmt-depth-retention-bundle'); upload = module(root, 'dxmt-upload-batching-bundle')
    residency = module(root, 'dxmt-residency-bundle'); drain = module(root, 'dxmt-residency-drain-bundle')
    snapshot = optimizer.read_json(root / FOLDER / 'prepared.json'); old = snapshot['state']; records = snapshot['records']
    assert snapshot['format'] == 'madeira-padded-light-base-v1'
    assert optimizer.sha(base) == BASE_ARCHIVE and module(root, 'dxmt-pacing-bundle').members(base) == old['members']
    assert source_state(root, optimizer) == snapshot['sources']
    assert read_inputs(root, optimizer) == records, 'producer records changed during compile'
    state = optimizer.collect(root)
    for key in old:
        if key not in ('members', 'native_archives'):
            assert state[key] == old[key], ('unrelated input or payload changed', key)
    assert set(state['native_archives']) == set(old['native_archives'])
    assert sorted(n for n in old['native_archives'] if state['native_archives'][n] != old['native_archives'][n]) == [ARCHIVE]
    before, after = old['members'], state['members']
    assert [(m['name'], m['occurrence']) for m in before] == [(m['name'], m['occurrence']) for m in after]
    assert sorted(new['name'] for previous, new in zip(before, after) if previous != new) == list(CHANGED)
    objects = {}
    for name in CHANGED:
        assert sum(m['name'] == name for m in after) == 1
        objects[name] = srv.object_bytes(root / ARCHIVE, name)
        assert objects[name] == (root / FOLDER / name).read_bytes(), ('actual archive differs from compiled output', name)
    proof = object_proof(objects, optimizer)
    assert optimizer.digest(objects['airconv_context.o']) == snapshot['context_sha256']
    assert optimizer.compiler_id(objects['cache.o']) == snapshot['compilerID']
    commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
    binding = {'receipt': RECEIPT, 'commit': commit, 'base_archive_sha256': BASE_ARCHIVE, 'changed_objects': list(CHANGED)}
    updates = {}
    for name in depth.PARENTS:
        updated = rebound_sources(root, records[name], optimizer)
        if name in depth.NATIVE_PARENTS:
            updated.update(archive_sha256=state['native_archives'][ARCHIVE], members=after, padded_light_refresh=binding)
        if name == optimizer.RECEIPT:
            updated.update(oldCompilerID=snapshot['oldCompilerID'], compilerID=snapshot['compilerID'],
                           contextObjectSHA256=snapshot['context_sha256'],
                           object_proof=optimizer.replacement_proof(objects['airconv_context.o']))
        if name == 'build/dxmt-ios/shader-vector-load.json': updated['compilerID'] = snapshot['compilerID']
        updates[name] = updated
    optimizer.validate_parents(root, updates, state)
    updates[depth.RECEIPT] = dict(records[depth.RECEIPT], **depth.record_for(state, updates), padded_light_refresh=binding)
    for name in (upload.RECEIPT, residency.RECEIPT, drain.RECEIPT):
        updated = rebound_sources(root, records[name], optimizer)
        updated.update(archive_sha256=state['native_archives'][ARCHIVE], padded_light_refresh=binding)
        if 'members' in updated: updated['members'] = after
        if name == residency.RECEIPT: updated['object_proof'] = proof['residency']
        if name == drain.RECEIPT: updated['object_sha256'] = optimizer.digest(objects['winemetal_unix.o'])
        updates[name] = updated
    updates[RECEIPT] = {'format': 'madeira-padded-lighting-v1', 'producer_commit': commit,
        'sources': snapshot['sources'], 'archive_sha256': state['native_archives'][ARCHIVE], 'objects': proof,
        'base_archive_sha256': BASE_ARCHIVE, 'changed_objects': list(CHANGED),
        'compilerID': snapshot['compilerID'], 'preserved_pe_payloads': len(state['pe_payloads']),
        'preserved_archive_members': len(after) - 4, 'output_resolution_changed': False,
        'texture_quality_changed': False, 'arithmetic_changed': False,
        'max_pending_scratch_buffers': 7, 'max_idle_scratch_buffers': 1,
        'bytes_per_scratch_buffer': 14680080, 'gpu_completion_required_for_reuse': True}
    upload.write_updates(root, updates)
    verify(root)


def verify(root):
    optimizer = module(root, 'dxmt-optimizer-bundle'); srv = module(root, 'dxmt-srv-bundle')
    module(root, 'dxmt-residency-bundle').verify(root)
    saved = optimizer.read_json(root / RECEIPT)
    assert saved['format'] == 'madeira-padded-lighting-v1'
    assert saved['sources'] == source_state(root, optimizer)
    assert saved['archive_sha256'] == optimizer.sha(root / ARCHIVE)
    objects = {name: srv.object_bytes(root / ARCHIVE, name) for name in CHANGED}
    assert saved['objects'] == object_proof(objects, optimizer)
    assert saved['compilerID'] == optimizer.compiler_id(objects['cache.o'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True); parser.add_argument('--prepare', type=Path)
    parser.add_argument('--stamp', action='store_true'); parser.add_argument('--base', type=Path)
    args = parser.parse_args(); root = args.root.resolve()
    assert not (args.prepare and args.stamp)
    if args.prepare: prepare(root, args.prepare)
    elif args.stamp:
        assert args.base; stamp(root, args.base)
    else: verify(root)
    print('PASS: padded lighting source/object/cache/receipt identities and unchanged guest payloads verified')


if __name__ == '__main__': main()
