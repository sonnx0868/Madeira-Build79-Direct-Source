#!/usr/bin/env python3
"""Bind the upload scheduler to ARM64EC D3D11, preserving the native renderer.

The incremental path accepts the verified868 renderer only, saves every native
archive/Windows payload/receipt before compilation, and permits exactly one PE
to change. Existing producer commits are never relabelled. The shared context
source binding in gpu-pipeline describes the EC frontend; the native archive
(including its older D3D9 context) stays byte-for-byte unchanged.
"""
from pathlib import Path
import argparse
import copy
import hashlib
import importlib.util
import json
import re
import subprocess
import tempfile


RECEIPT = 'build/dxmt-pe/upload-batching.json'
FOLDER = 'build/dxmt-pe/upload-batching-refresh'
CONTEXT = 'dxmt/src/dxmt/dxmt_context.cpp'
PATCH = 'patches/dxmt-upload-batching.patch'
SOURCES = (PATCH, 'build/dxmt-ios/upload_batching.hpp', CONTEXT,
           'scripts/refresh-upload-batching.sh', 'tools/dxmt-upload-batching-bundle.py')
ARCHIVE = 'app/Madeira/libdxmt_combined.a'
DLL = 'app/Madeira/arm64ec-windows/d3d11.dll'
NTDLL = 'app/Madeira/arm64ec-windows/ntdll.dll'
BASE_ARCHIVE = 'f367ec5d55bc80ca5f92cffec0c226273a460f385c7892692c72837465f166e4'
BASE_DLL = '80368270b936c5c3b84b6f67f87513086339f2e067f7028329672fa16a73ee6c'
BASE_NTDLL = '0ce6d741e85252960dfa9c8aaabc96f588de580c184e6b1015ac8a9672e7b24f'
BASE_COMPILER = '89c0064b50e7807ac53b60bbb0cf93fd15d4d3d6f8c1b1c40e45b78a447fa032'
BASE_CONTEXT = '0aa62f37f5318a9fee3c3e8190cca1222cd17f2909cb63920494450d2c917a3b'
BASE_FLUSH_CODE = '7bb03693a6e8c6b8b4b3c13a6248ff26ded169381f98a06ee085d2179e6cb4cf'


def module(root, name):
    spec = importlib.util.spec_from_file_location(name, root / ('tools/' + name + '.py'))
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


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


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def source_state(root):
    current = {name: source_sha(root / name) for name in SOURCES}
    patch = (root / PATCH).read_bytes().replace(b'\r\n', b'\n')
    assert re.findall(rb'^\+\+\+ b/([^\r\n]+)$', patch, re.M) == [CONTEXT.removeprefix('dxmt/').encode()]
    with tempfile.TemporaryDirectory(prefix='madeira-upload-source-') as work:
        folder = Path(work)
        source = folder / CONTEXT.removeprefix('dxmt/')
        source.parent.mkdir(parents=True)
        source.write_bytes((root / CONTEXT).read_bytes().replace(b'\r\n', b'\n'))
        patch_file = folder / 'upload.patch'
        patch_file.write_bytes(patch)
        subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', str(patch_file)], check=True, capture_output=True)
        previous = source_sha(source)
        subprocess.run(['git', '-C', str(folder), 'apply', str(patch_file)], check=True, capture_output=True)
        assert source_sha(source) == current[CONTEXT], 'upload overlay is not reversible'
    return current, previous


def compiled_context(root):
    """Record the actual ARM64EC flush body, never a marker string alone.

    Inclusion/source provenance is checked here; dependency and ordering
    semantics are exercised by the production helper and native Metal tests.
    """
    swap = module(root, 'dxmt-swapchain-bundle')
    data = (root / DLL).read_bytes()
    pe = swap.PE(data)
    syms = swap.symbols(pe)
    _, ranges, _ = swap.hybrid(pe)
    suffix = '_ZN4dxmt23ArgumentEncodingContext13flushCommandsEN3WMT13CommandBufferEyy'
    candidates = [(name, rva) for name, rva in syms.items()
                  if name in (suffix, '#' + suffix, '#' + suffix + '$hp_target') and
                  any((start & 3) == 1 and (start & ~3) <= rva < (start & ~3) + length
                      for start, length in ranges)]
    addresses = {rva for _, rva in candidates}
    assert len(addresses) == 1, 'missing/ambiguous compiled ARM64EC command scheduler'
    start = addresses.pop()
    limit = min((rva for name, rva in syms.items() if rva > start and
                 (name.startswith('#_Z') or name.startswith('_Z'))), default=start)
    range_end = next((rva & ~3) + length for rva, length in ranges
                     if (rva & 3) == 1 and (rva & ~3) <= start < (rva & ~3) + length)
    end = min(limit, range_end)
    assert 512 <= end - start <= 512 * 1024, ('invalid scheduler body bounds', start, end)
    code = pe.read(pe.offset(start, end - start), end - start)
    return {'format': 'arm64ec-command-scheduler-v1', 'symbols': sorted(name for name, _ in candidates),
            'rva': start, 'size': len(code), 'sha256': digest(code),
            'semantics_proved_by_binary_scan': False}


def collect(root):
    depth = module(root, 'dxmt-depth-retention-bundle')
    optimizer = module(root, 'dxmt-optimizer-bundle')
    state = depth.collect(root, True)
    sources, previous = source_state(root)
    state.update(upload_sources=sources, pre_upload_context=previous,
                 pe_payloads=optimizer.payload_hashes(root),
                 receipt_hashes={name: value for name, value in optimizer.receipt_hashes(root).items()
                                 if name != RECEIPT}, compiled_context=compiled_context(root))
    return state


def validate_parents(root, parents, state, previous=False):
    depth = module(root, 'dxmt-depth-retention-bundle')
    optimizer = module(root, 'dxmt-optimizer-bundle')
    srv = module(root, 'dxmt-srv-bundle')
    assert set(parents) == set(depth.PARENTS), 'upload refresh requires the complete renderer receipt chain'
    adjusted = copy.deepcopy(parents)
    for name, parent in adjusted.items():
        if CONTEXT in parent['sources'] and previous:
            assert parent['sources'][CONTEXT] == state['pre_upload_context'], ('unrelated pre-upload context source', name)
            parent['sources'][CONTEXT] = state['upload_sources'][CONTEXT]
    depth.validate_parents(root, adjusted, state)
    identity = optimizer.compiler_id(srv.object_bytes(root / ARCHIVE, 'cache.o'))
    for name in ('build/dxmt-ios/shader-vector-load.json', optimizer.RECEIPT):
        assert parents[name]['compilerID'] == identity, ('shader cache namespace changed', name)
    assert parents[optimizer.RECEIPT]['contextObjectSHA256'] == digest(srv.object_bytes(root / ARCHIVE, 'airconv_context.o'))
    optimizer.validate_renpy(root)


def validate_depth(root, state, parents, record):
    depth = module(root, 'dxmt-depth-retention-bundle')
    for key, value in depth.record_for(state, parents).items():
        assert record[key] == value, ('stale inherited depth receipt', key)


def prepare(root, output):
    depth = module(root, 'dxmt-depth-retention-bundle')
    state = collect(root)
    parents = depth.read_parents(root)
    assert state['native_archives'][ARCHIVE] == BASE_ARCHIVE, 'incremental upload refresh requires verified868 native archive'
    assert state['pe_payloads'][DLL] == BASE_DLL, 'incremental upload refresh requires verified868 D3D11'
    assert state['pe_payloads'][NTDLL] == BASE_NTDLL, 'incremental upload refresh lost RenPy862 callback fix'
    assert state['pre_upload_context'] == BASE_CONTEXT, 'unrelated context change before upload overlay'
    assert parents['build/dxmt-ios/shader-optimizer.json']['compilerID'] == BASE_COMPILER
    validate_parents(root, parents, state, previous=True)
    old_depth = read_json(root / depth.RECEIPT)
    validate_depth(root, state, parents, old_depth)
    snapshot = {'format': 'madeira-upload-batching-base-v1', 'state': state,
                'parents': parents, 'depth_receipt': old_depth}
    write_json(output, snapshot)
    return snapshot


def transition(root, snapshot, state, parents, commit):
    """Validate all input/output invariants before returning any receipt writes."""
    depth = module(root, 'dxmt-depth-retention-bundle')
    assert snapshot['format'] == 'madeira-upload-batching-base-v1'
    old = snapshot['state']
    assert parents == snapshot['parents'], 'parent receipt changed during frontend compilation'
    assert state['receipt_hashes'] == old['receipt_hashes'], 'receipt content/set changed before validation'
    assert state['native_archives'] == old['native_archives'], 'frontend refresh changed a native archive'
    assert state['members'] == old['members'], 'frontend refresh changed a native archive member'
    for key in ('sources', 'pre_patch_sources', 'upload_sources', 'pre_upload_context'):
        assert state[key] == old[key], ('source changed after preparation', key)
    assert set(state['pe_payloads']) == set(old['pe_payloads']), 'Windows payload added or removed'
    changed = sorted(name for name in old['pe_payloads'] if old['pe_payloads'][name] != state['pe_payloads'][name])
    assert changed == [DLL], ('unexpected changed Windows payloads', changed)
    assert state['compiled_context']['sha256'] != old['compiled_context']['sha256'], 'command scheduler body was not rebuilt'
    for name in depth.DLLS:
        before, after = old['pe_libraries'][name], state['pe_libraries'][name]
        assert before['exports'] == after['exports'] and before['imports'] == after['imports'], ('frontend ABI changed', name)
        if name != DLL:
            assert before == after, ('unbuilt frontend changed', name)
    validate_parents(root, snapshot['parents'], old, previous=True)
    validate_depth(root, old, snapshot['parents'], snapshot['depth_receipt'])
    before, after = old['pe_payloads'][DLL], state['pe_payloads'][DLL]
    binding = {'commit': commit, 'receipt': RECEIPT, 'base_dll_sha256': before,
               'd3d11_sha256': after, 'scope': 'arm64ec-d3d11-only',
               'native_archive_changed': False, 'native_context_recompiled': False}
    updates = {}
    for name, parent in parents.items():
        updated = copy.deepcopy(parent)
        changed_source = {}
        if CONTEXT in updated['sources']:
            assert updated['sources'][CONTEXT] == old['pre_upload_context']
            changed_source[CONTEXT] = {'before': old['pre_upload_context'], 'after': state['upload_sources'][CONTEXT],
                                       'scope': 'arm64ec-d3d11-workspace-source'}
            updated['sources'][CONTEXT] = state['upload_sources'][CONTEXT]
        if name in depth.PE_PARENTS[:2]:
            updated['sha256'] = after
        elif 'd3d11_sha256' in updated:
            updated['d3d11_sha256'] = after
        if name == depth.PE_PARENTS[1]:
            updated['entries'] = state['swapchain_entries']
        if updated != parent:
            updated['upload_batching_frontend_refresh'] = dict(binding, changed_sources=changed_source)
            updates[name] = updated
    combined = dict(parents, **updates)
    validate_parents(root, combined, state)
    updated_depth = copy.deepcopy(snapshot['depth_receipt'])
    updated_depth.update(depth.record_for(state, combined))
    updated_depth['upload_batching_frontend_refresh'] = binding
    validate_depth(root, state, combined, updated_depth)
    updates[depth.RECEIPT] = updated_depth
    return updates


def record_for(root, state):
    return {'format': 'madeira-upload-batching-v1', 'scope': 'arm64ec-d3d11-only',
            'sources': state['upload_sources'], 'pre_upload_context': state['pre_upload_context'],
            'd3d11_sha256': state['pe_payloads'][DLL], 'compiled_context': state['compiled_context'],
            'archive_sha256': state['native_archives'][ARCHIVE], 'members': state['members'],
            'pe_libraries': state['pe_libraries'], 'renpy_ntdll_sha256': state['pe_payloads'][NTDLL],
            'lookback_limit': 64, 'input_commands_limit': 64,
            'resolution_or_texture_quality_changed': False}


def write_updates(root, updates):
    staged = []
    try:
        for name, value in updates.items():
            target = root / name
            temporary = target.with_suffix('.json.upload-tmp')
            write_json(temporary, value)
            staged.append((temporary, target))
        for temporary, target in staged:
            temporary.replace(target)
    finally:
        for temporary, _ in staged:
            temporary.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--prepare', type=Path)
    parser.add_argument('--stamp', action='store_true')
    parser.add_argument('--base', type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    assert not (args.prepare and args.stamp) and (not args.base or args.stamp)
    if args.prepare:
        prepare(root, args.prepare)
        print('PASS: verified868 source/binary/receipt bindings saved before ARM64EC upload refresh')
        return
    depth = module(root, 'dxmt-depth-retention-bundle')
    state = collect(root)
    parents = depth.read_parents(root)
    assert state['pe_payloads'][DLL] != BASE_DLL and state['compiled_context']['sha256'] != BASE_FLUSH_CODE, \
        'restored868 D3D11 cannot be stamped or consumed as the upload scheduler'
    record = record_for(root, state)
    if args.stamp:
        commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        record['producer_commit'] = commit
        if args.base:
            snapshot = read_json(args.base)
            updates = transition(root, snapshot, state, parents, commit)
            record.update(base_d3d11_sha256=snapshot['state']['pe_payloads'][DLL],
                          changed_frontends=[DLL], changed_native_archives=[],
                          preserved_windows_payloads=len(state['pe_payloads']) - 1,
                          native_context_recompiled=False)
        else:
            # Full bootstrap is allowed only after all original producer gates
            # bind the newly built artifacts and exact current source.
            validate_parents(root, parents, state)
            validate_depth(root, state, parents, read_json(root / depth.RECEIPT))
            updates = {}
            record['build_mode'] = 'full-bootstrap'
        updates[RECEIPT] = record
        write_updates(root, updates)
    else:
        validate_parents(root, parents, state)
        validate_depth(root, state, parents, read_json(root / depth.RECEIPT))
        saved = read_json(root / RECEIPT)
        for key, value in record.items():
            assert saved[key] == value, ('stale upload batching bundle', key)
    print('PASS: ARM64EC D3D11 upload scheduler source/binary and inherited renderer/RenPy identities verified')


if __name__ == '__main__':
    main()
