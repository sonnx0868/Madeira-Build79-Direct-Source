#!/usr/bin/env python3
"""Refresh two shader objects while preserving every existing native/PE fix.

--prepare validates the original receipt chain before ar replaces any member.
The only legacy migration is the verified build862 optimizer to the separately
reversible compute-PCF overlay. --stamp validates the entire transition before
writing any receipt. Unchanged producers keep their original provenance.
"""
from pathlib import Path
import argparse
import ast
import copy
import hashlib
import importlib.util
import json
import re
import struct
import subprocess
import tempfile


SOURCES = ('build/dxmt-ios/explicit_lod_guard.hpp', 'patches/dxmt-explicit-lod-guard.patch',
           'dxmt/src/airconv/airconv_context.cpp', 'build/dxmt-ios/refresh-optimizer.sh',
           'build/dxmt-ios/guarded_pcf.hpp', 'patches/dxmt-guarded-pcf.patch',
           'dxmt/src/winemetal/unix/cache.c', 'tools/shader-compiler-identity.py',
           'tools/dxmt-optimizer-bundle.py')
LEGACY_SOURCES = SOURCES[:4]
BASE_COMMIT = '5a40a8c50bb191bf8cea343028125640f6cf9147'
BASE_ARCHIVE_SHA256 = '1f10df6007e7e688a95472715e267973fb6170c222bfbd26e301df7cb6c4bf65'
BASE_NTDLL_SHA256 = '0ce6d741e85252960dfa9c8aaabc96f588de580c184e6b1015ac8a9672e7b24f'
ARCHIVE = 'app/Madeira/libdxmt_combined.a'
RECEIPT = 'build/dxmt-ios/shader-optimizer.json'
CONTEXT = 'dxmt/src/airconv/airconv_context.cpp'
CACHE = 'dxmt/src/winemetal/unix/cache.c'
PATCH = 'patches/dxmt-guarded-pcf.patch'
FOLDER = 'build/dxmt-ios/optimizer-refresh'
RENPY_RECEIPT = 'build/wine-pe/native-callback-context.json'
NTDLL = 'app/Madeira/arm64ec-windows/ntdll.dll'
CHANGED = ['airconv_context.o', 'cache.o']
RECEIPT_DIRS = ('build/dxmt-ios', 'build/dxmt-pe', 'build/wine-pe', 'build/wine-i386',
                'build/win32u-unix', 'build/ntdll-unix', 'build/madsync')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def sha(path):
    with path.open('rb') as stream:
        value = hashlib.sha256()
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(chunk)
        return value.hexdigest()


def source_sha(path):
    return digest(path.read_bytes().replace(b'\r\n', b'\n'))


def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def compiler_id(cache):
    identities = set(re.findall(rb'(?<![0-9a-f])[0-9a-f]{64}(?![0-9a-f])', cache))
    assert len(identities) == 1, 'ambiguous shader compiler identity'
    return next(iter(identities)).decode()


def replacement_proof(data):
    """Require a defined compute pass in the actual ARM64 Mach-O object.

    This is object inclusion/source binding, not a proof of the pass semantics;
    LLVM and real-Metal regression tests exercise the production wrapper.
    """
    assert len(data) >= 32
    magic, cpu, _, kind, commands, size, _, _ = struct.unpack_from('<8I', data)
    assert (magic, cpu, kind) == (0xfeedfacf, 0x100000c, 1), 'optimizer is not an ARM64 Mach-O object'
    assert 32 + size <= len(data)
    at, symbols = 32, []
    for _ in range(commands):
        command, length = struct.unpack_from('<II', data, at)
        assert length >= 8 and at + length <= 32 + size
        if command == 2:
            assert length == 24
            offset, count, strings, string_size = struct.unpack_from('<4I', data, at + 8)
            assert offset + count * 16 <= len(data) and strings + string_size <= len(data)
            for index in range(count):
                name_at, symbol_type, section, _, _ = struct.unpack_from('<IBBHQ', data, offset + index * 16)
                assert name_at < string_size
                end = data.find(b'\0', strings + name_at, strings + string_size)
                assert end >= 0
                name = data[strings + name_at:end].decode('utf-8')
                if not symbol_type & 0xe0 and symbol_type & 0x0e == 0x0e and section:
                    symbols.append(name)
        at += length
    assert at == 32 + size
    selected = sorted(name for name in symbols if 'ComputePCFPass' in name and '3runE' in name)
    assert selected, 'missing defined production ComputePCFPass run implementation'
    for marker in (b'SinkingPass', b'LoopRotatePass', b'pcf.sample', b'pcf.condition'):
        assert marker in data, ('missing prior optimizer or PCF implementation', marker)
    return {'format': 'arm64-mach-o-compute-pcf-v1', 'defined_run_symbols': selected,
            'contextObjectSHA256': digest(data), 'semantics_proved_by_object_scan': False}


def git_bytes(root, revision, name):
    return subprocess.check_output(['git', '-C', str(root), 'show', revision + ':' + name])


def legacy_source_hashes(root):
    """Reconstruct old context/cache from frozen git + frozen patch order.

    The current dirty submodule and the current patch list are never the old
    source authority. A missing baseline commit fails instead of accepting a
    stale receipt merely because it contains the expected filenames.
    """
    script = git_bytes(root, BASE_COMMIT, 'tests/host/check-dxmt-patches.py')
    tree = ast.parse(script)
    lists = [ast.literal_eval(node.value) for node in ast.walk(tree)
             if isinstance(node, ast.Assign) and any(isinstance(target, ast.Name) and
             target.id == 'PATCH_NAMES' for target in node.targets)]
    assert len(lists) == 1 and PATCH not in lists[0]
    pin = subprocess.check_output(['git', '-C', str(root), 'rev-parse', BASE_COMMIT + ':dxmt'], text=True).strip()
    hashes = {name: digest(git_bytes(root, BASE_COMMIT, name).replace(b'\r\n', b'\n'))
              for name in LEGACY_SOURCES if name != CONTEXT}
    with tempfile.TemporaryDirectory(prefix='madeira-optimizer-base-') as work:
        folder = Path(work)
        for name in (CONTEXT, CACHE):
            relative = name.removeprefix('dxmt/')
            path = folder / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(git_bytes(root / 'dxmt', pin, relative).replace(b'\r\n', b'\n'))
        for index, name in enumerate(lists[0]):
            data = git_bytes(root, BASE_COMMIT, name).replace(b'\r\n', b'\n')
            if not any(('+++ b/' + target.removeprefix('dxmt/') + '\n').encode() in data
                       for target in (CONTEXT, CACHE)):
                continue
            patch = folder / (str(index) + '.patch')
            patch.write_bytes(data)
            subprocess.run(['git', '-C', str(folder), 'apply', '--include=src/airconv/airconv_context.cpp',
                            '--include=src/winemetal/unix/cache.c', str(patch)], check=True, capture_output=True)
        hashes.update({name: source_sha(folder / name.removeprefix('dxmt/')) for name in (CONTEXT, CACHE)})
    return hashes


def validate_legacy(root, parent, state):
    assert parent['format'] == 'madeira-shader-optimizer-v1'
    assert set(parent['sources']) == set(LEGACY_SOURCES)
    assert state['native_archives'][ARCHIVE] == BASE_ARCHIVE_SHA256, 'legacy optimizer requires verified862 native renderer'
    assert sha(root / NTDLL) == BASE_NTDLL_SHA256, 'legacy optimizer input lost the RenPy862 ntdll fix'
    expected = legacy_source_hashes(root)
    assert parent['sources'] == {name: expected[name] for name in LEGACY_SOURCES}, 'legacy source receipts differ from frozen862'
    for name in ('build/dxmt-ios/explicit_lod_guard.hpp', 'patches/dxmt-explicit-lod-guard.patch', CACHE):
        assert source_sha(root / name) == expected[name], ('unrebuilt or unrelated shader source changed', name)
    data = (root / PATCH).read_bytes().replace(b'\r\n', b'\n')
    assert re.findall(rb'^\+\+\+ b/([^\r\n]+)$', data, re.M) == [CONTEXT.removeprefix('dxmt/').encode()]
    with tempfile.TemporaryDirectory(prefix='madeira-optimizer-overlay-') as work:
        folder = Path(work)
        source = folder / CONTEXT.removeprefix('dxmt/')
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_bytes((root / CONTEXT).read_bytes().replace(b'\r\n', b'\n'))
        patch = folder / 'pcf.patch'
        patch.write_bytes(data)
        subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', str(patch)], check=True, capture_output=True)
        assert source_sha(source) == expected[CONTEXT], 'PCF overlay hides an unrelated optimizer change'
        subprocess.run(['git', '-C', str(folder), 'apply', str(patch)], check=True, capture_output=True)
        assert source_sha(source) == source_sha(root / CONTEXT), 'PCF source transition is not reversible'


def payload_hashes(root):
    payloads = {}
    for folder in sorted((root / 'app/Madeira').glob('*-windows')):
        for path in sorted(folder.rglob('*')):
            if path.is_file():
                payloads[path.relative_to(root).as_posix()] = sha(path)
    assert NTDLL in payloads, 'missing Windows runtime'
    return payloads


def receipt_hashes(root):
    paths = [path for name in RECEIPT_DIRS for path in (root / name).glob('*.json')]
    paths += [root / name for name in ('app/Madeira/gl/mesa-patches.json',
                                     'app/Madeira/translation-lab/receipt.json') if (root / name).exists()]
    return {path.relative_to(root).as_posix(): sha(path) for path in sorted(paths)}


def validate_renpy(root):
    tool = module(root, 'ntdll-native-callback-bundle')
    data = (root / NTDLL).read_bytes()
    receipt = read_json(root / RENPY_RECEIPT)
    assert receipt['sha256'] == digest(data), 'stale RenPy native callback receipt'
    assert receipt['proof'] == tool.verify_binary(root, data), 'RenPy compiled callback bypass missing'
    assert receipt['sources'] == {name: source_sha(root / name) for name in tool.SOURCES}, 'RenPy source binding changed'


def collect(root):
    depth = module(root, 'dxmt-depth-retention-bundle')
    state = depth.collect(root, True)
    state.update(pe_payloads=payload_hashes(root), receipt_hashes=receipt_hashes(root),
                 optimizer_sources={name: source_sha(root / name) for name in SOURCES})
    return state


def validate_parents(root, parents, state, legacy=False):
    depth = module(root, 'dxmt-depth-retention-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    assert set(parents) == set(depth.PARENTS), 'optimizer refresh requires the complete verified parent chain'
    adjusted = copy.deepcopy(parents)
    if legacy:
        validate_legacy(root, parents[RECEIPT], state)
        adjusted[RECEIPT]['sources'] = state['optimizer_sources']
    depth.validate_parents(root, adjusted, state)
    identity = compiler_id(srv.object_bytes(root / ARCHIVE, 'cache.o'))
    assert parents[RECEIPT]['compilerID'] == identity
    assert parents['build/dxmt-ios/shader-vector-load.json']['compilerID'] == identity
    assert parents[RECEIPT]['contextObjectSHA256'] == digest(srv.object_bytes(root / ARCHIVE, 'airconv_context.o'))


def validate_depth_record(root, state, parents, record=None):
    depth = module(root, 'dxmt-depth-retention-bundle')
    saved = record if record is not None else read_json(root / depth.RECEIPT)
    for key, value in depth.record_for(state, parents).items():
        assert saved[key] == value, ('stale depth receipt', key)


def prepare(root, base, replacement):
    assert sha(base) == sha(root / ARCHIVE), 'base archive is not the original restored archive'
    depth = module(root, 'dxmt-depth-retention-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    state = collect(root)
    parents = depth.read_parents(root)
    legacy = set(parents[RECEIPT]['sources']) == set(LEGACY_SOURCES)
    validate_parents(root, parents, state, legacy=legacy)
    validate_depth_record(root, state, parents)
    validate_renpy(root)
    proof = replacement_proof(replacement.read_bytes())
    old = compiler_id(srv.object_bytes(base, 'cache.o'))
    identity = digest(('madeira-compute-pcf-v1\0' + old + '\0' + proof['contextObjectSHA256']).encode())
    assert identity != old
    prepared = {'format': 'madeira-shader-optimizer-base-v2', 'state': state, 'parents': parents,
                'depth_receipt': read_json(root / depth.RECEIPT), 'legacy_migration': legacy,
                'oldCompilerID': old, 'compilerID': identity, 'object_proof': proof,
                'contextObjectSHA256': proof['contextObjectSHA256']}
    folder = root / FOLDER
    folder.mkdir(parents=True, exist_ok=True)
    write_json(folder / 'prepared.json', prepared)
    (folder / 'madeira_shader_compiler_identity.h').write_text(
        '#pragma once\n#define MADEIRA_SHADER_COMPILER_ID "' + identity + '"\n', encoding='utf-8')
    return prepared


def transition(root, base, snapshot, state, parents, commit):
    """Return every planned receipt write; validation failures have no writes."""
    depth = module(root, 'dxmt-depth-retention-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    assert snapshot['format'] == 'madeira-shader-optimizer-base-v2'
    old = snapshot['state']
    assert sha(base) == old['native_archives'][ARCHIVE], 'base archive changed after preparation'
    assert module(root, 'dxmt-pacing-bundle').members(base) == old['members']
    assert parents == snapshot['parents'], 'parent receipt changed during rebuild'
    assert state['receipt_hashes'] == old['receipt_hashes'], 'receipt set/content changed during rebuild'
    assert state['pe_payloads'] == old['pe_payloads'], 'optimizer rebuild changed or removed a Windows payload'
    assert state['pe_libraries'] == old['pe_libraries'] and state['swapchain_entries'] == old['swapchain_entries']
    for key in ('sources', 'pre_patch_sources', 'depth_symbols', 'optimizer_sources'):
        assert state[key] == old[key], ('source or depth implementation changed during rebuild', key)
    before, after = old['native_archives'], state['native_archives']
    assert set(before) == set(after), 'native archive added or removed during rebuild'
    assert [name for name in sorted(before) if before[name] != after[name]] == [ARCHIVE], 'unrelated native archive changed'
    assert after[ARCHIVE] == sha(root / ARCHIVE), 'stale replacement archive hash'
    previous, current = old['members'], state['members']
    assert current == module(root, 'dxmt-pacing-bundle').members(root / ARCHIVE), 'stale replacement archive members'
    assert [(m['name'], m['occurrence']) for m in previous] == [(m['name'], m['occurrence']) for m in current], 'archive member order/identity changed'
    changed = sorted(new['name'] for old_member, new in zip(previous, current) if old_member != new)
    assert changed == CHANGED, ('unrelated archive objects changed', changed)
    assert all(sum(member['name'] == name for member in current) == 1 for name in CHANGED)
    context = srv.object_bytes(root / ARCHIVE, 'airconv_context.o')
    assert replacement_proof(context) == snapshot['object_proof']
    assert digest(context) == snapshot['contextObjectSHA256']
    assert compiler_id(srv.object_bytes(root / ARCHIVE, 'cache.o')) == snapshot['compilerID']
    assert snapshot['compilerID'] == digest(('madeira-compute-pcf-v1\0' + snapshot['oldCompilerID'] + '\0' + digest(context)).encode())
    assert compiler_id(srv.object_bytes(base, 'cache.o')) == snapshot['oldCompilerID']
    assert digest(srv.object_bytes(root / ARCHIVE, 'cache.o')) == sha(root / FOLDER / 'cache.o'), 'archive cache differs from compiled cache object'
    assert digest(context) == sha(root / FOLDER / 'airconv_context.o'), 'archive optimizer differs from compiled context object'
    # Recheck the original bindings against their original archive even after
    # replacement; validate_parents reads current object identities separately.
    adjusted = copy.deepcopy(parents)
    if snapshot['legacy_migration']:
        validate_legacy(root, parents[RECEIPT], old)
        adjusted[RECEIPT]['sources'] = old['optimizer_sources']
    depth.validate_parents(root, adjusted, old)
    assert parents[RECEIPT]['compilerID'] == snapshot['oldCompilerID']
    assert parents[RECEIPT]['contextObjectSHA256'] == digest(srv.object_bytes(base, 'airconv_context.o'))
    validate_depth_record(root, old, parents, snapshot['depth_receipt'])
    validate_renpy(root)
    record = {'format': 'madeira-shader-optimizer-v2', 'archive_sha256': after[ARCHIVE], 'members': current,
              'sources': state['optimizer_sources'], 'producer_commit': commit,
              'base_archive_sha256': before[ARCHIVE], 'changed_objects': changed,
              'preserved_members': len(previous) - 2, 'preserved_pe_payloads': len(state['pe_payloads']),
              'preserved_native_archives': len(before) - 1, 'contextObjectSHA256': digest(context),
              'oldCompilerID': snapshot['oldCompilerID'], 'compilerID': snapshot['compilerID'],
              'object_proof': snapshot['object_proof'], 'renpy_ntdll_sha256': state['pe_payloads'][NTDLL]}
    updates = {RECEIPT: record}
    binding = {'commit': commit, 'receipt': RECEIPT, 'objects': changed, 'base_archive_sha256': before[ARCHIVE]}
    for name in depth.NATIVE_PARENTS:
        if name == RECEIPT:
            continue
        updated = copy.deepcopy(parents[name])
        updated.update(archive_sha256=after[ARCHIVE], members=current, optimizer_refresh=binding)
        if name == 'build/dxmt-ios/shader-vector-load.json':
            updated['compilerID'] = snapshot['compilerID']
        updates[name] = updated
    final_parents = dict(parents, **updates)
    validate_parents(root, final_parents, state)
    depth_record = copy.deepcopy(snapshot['depth_receipt'])
    depth_record.update(archive_sha256=after[ARCHIVE], members=current, optimizer_refresh=binding)
    validate_depth_record(root, state, final_parents, depth_record)
    updates[depth.RECEIPT] = depth_record
    return updates


def verify(root):
    depth = module(root, 'dxmt-depth-retention-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    state = collect(root)
    parents = depth.read_parents(root)
    validate_parents(root, parents, state)
    validate_depth_record(root, state, parents)
    validate_renpy(root)
    saved = parents[RECEIPT]
    assert saved['format'] == 'madeira-shader-optimizer-v2'
    proof = replacement_proof(srv.object_bytes(root / ARCHIVE, 'airconv_context.o'))
    assert saved['object_proof'] == proof
    assert saved['compilerID'] == digest(('madeira-compute-pcf-v1\0' + saved['oldCompilerID'] + '\0' + proof['contextObjectSHA256']).encode())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--base', type=Path)
    parser.add_argument('--prepare', type=Path)
    parser.add_argument('--stamp', action='store_true')
    args = parser.parse_args()
    root = args.root.resolve()
    assert not (args.prepare and args.stamp)
    if args.prepare:
        assert args.base
        prepare(root, args.base, args.prepare)
        print('PASS: verified original862 source/receipt/native/PE bindings; new cache namespace follows the actual ComputePCFPass object')
        return
    if args.stamp:
        assert args.base
        depth = module(root, 'dxmt-depth-retention-bundle')
        snapshot = read_json(root / FOLDER / 'prepared.json')
        state = collect(root)
        parents = depth.read_parents(root)
        commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        updates = transition(root, args.base, snapshot, state, parents, commit)
        # All validations finish before the first write. os.replace gives each
        # receipt an atomic commit; interruption leaves verification failing.
        staged = []
        try:
            for name, value in updates.items():
                target = root / name
                temporary = target.with_suffix('.json.optimizer-tmp')
                write_json(temporary, value)
                staged.append((temporary, target))
            for temporary, target in staged:
                temporary.replace(target)
        finally:
            for temporary, _ in staged:
                temporary.unlink(missing_ok=True)
    else:
        assert not args.base
        verify(root)
    print('PASS: ComputePCF optimizer/cache, all renderer receipt parents and RenPy context proof match; unrelated native objects and Windows payloads preserved')


if __name__ == '__main__':
    main()
