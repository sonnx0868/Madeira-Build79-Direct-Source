#!/usr/bin/env python3
"""Bind depth-pool retirement to D3D11 without relabelling native code.

For an incremental PE refresh, apply the overlays, --prepare a snapshot before
compiling, then --stamp --base that snapshot. Every shipped native archive and
the other three DXMT frontends must remain byte-identical. A full bootstrap may
use --stamp alone after its existing parent receipts have been produced.
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


RECEIPT = 'build/dxmt-pe/depth-retention.json'
PATCH = 'patches/dxmt-depth-rename-retention.patch'
PATCHED = ('dxmt/src/dxmt/dxmt_allocation.hpp', 'dxmt/src/dxmt/dxmt_texture.hpp',
           'dxmt/src/dxmt/dxmt_texture.cpp', 'dxmt/src/d3d11/d3d11_context_impl.cpp')
SOURCES = (PATCH, 'build/dxmt-ios/depth_rename_retention.hpp', *PATCHED)
ARCHIVE = 'app/Madeira/libdxmt_combined.a'
DLLS = tuple('app/Madeira/arm64ec-windows/' + name + '.dll'
             for name in ('d3d11', 'd3d9', 'dxgi', 'winemetal'))
PE_PARENTS = ('build/dxmt-pe/query-park.json', 'build/dxmt-pe/swapchain-hooks.json',
              'build/dxmt-pe/color-views.json')
NATIVE_PARENTS = tuple('build/dxmt-ios/' + name + '.json' for name in (
    'producer-pacing', 'gpu-pipeline', 'shader-vector-load', 'shader-optimizer',
    'frameflow', 'retirement', 'present-resource'))
PARENTS = (*PE_PARENTS, *NATIVE_PARENTS)
OPTIONAL_PARENTS = frozenset(NATIVE_PARENTS[2:4])
# Extract each existing producer's literal source list, rather than weakening a
# receipt by accepting whatever (possibly incomplete) list happens to be in it.
PRODUCERS = {
    PE_PARENTS[0]: ('dxmt-query-bundle', 'sources'),
    PE_PARENTS[1]: ('dxmt-swapchain-bundle', 'SOURCES'),
    PE_PARENTS[2]: ('dxmt-color-views-bundle', 'SOURCES'),
    NATIVE_PARENTS[0]: ('dxmt-pacing-bundle', None),
    NATIVE_PARENTS[1]: ('dxmt-gpu-bundle', 'SOURCES'),
    NATIVE_PARENTS[2]: ('dxmt-srv-bundle', 'SOURCES'),
    NATIVE_PARENTS[3]: ('dxmt-optimizer-bundle', 'SOURCES'),
    NATIVE_PARENTS[4]: ('dxmt-frameflow-bundle', 'sources'),
    NATIVE_PARENTS[5]: ('dxmt-retirement-bundle', 'sources'),
    NATIVE_PARENTS[6]: ('dxmt-present-resource-bundle', 'sources'),
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def sha_file(path):
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


def producer_sources(root, receipt):
    import ast
    script, variable = PRODUCERS[receipt]
    tree = ast.parse((root / ('tools/' + script + '.py')).read_text(encoding='utf-8'))
    candidates = []
    for node in ast.walk(tree):
        if variable and isinstance(node, ast.Assign) and any(
                isinstance(target, ast.Name) and target.id == variable for target in node.targets):
            candidates.append(ast.literal_eval(node.value))
        elif variable is None and isinstance(node, ast.For) and isinstance(node.iter, ast.List):
            values = ast.literal_eval(node.iter)
            if 'build/dxmt-ios/present_pacing.h' in values:
                candidates.append(values)
    assert len(candidates) == 1, ('ambiguous producer source schema', receipt)
    assert candidates[0] and all(isinstance(s, str) for s in candidates[0])
    return set(candidates[0])


def patch_sources(root):
    """Prove any accepted source delta is precisely this reversible overlay."""
    after = {name: source_sha(root / name) for name in SOURCES}
    text = (root / PATCH).read_bytes().replace(b'\r\n', b'\n')
    targets = re.findall(rb'^\+\+\+ b/([^\r\n]+)$', text, re.M)
    assert set('dxmt/' + name.decode() for name in targets) == set(PATCHED)
    with tempfile.TemporaryDirectory(prefix='madeira-depth-receipt-') as work:
        folder = Path(work)
        for name in PATCHED:
            path = folder / name.removeprefix('dxmt/')
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes((root / name).read_bytes().replace(b'\r\n', b'\n'))
        patch = folder / 'retention.patch'
        patch.write_bytes(text)
        subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', str(patch)],
                       check=True, capture_output=True)
        before = {name: source_sha(folder / name.removeprefix('dxmt/')) for name in PATCHED}
        subprocess.run(['git', '-C', str(folder), 'apply', str(patch)],
                       check=True, capture_output=True)
        assert all(source_sha(folder / name.removeprefix('dxmt/')) == after[name] for name in PATCHED)
    return after, before


def inspect_frontends(root, require_depth):
    swap = module(root, 'dxmt-swapchain-bundle')
    compat = module(root, 'check-compatibility-binaries')
    libraries = {}
    for name in DLLS:
        data = (root / name).read_bytes()
        pe = swap.PE(data)
        assert pe.machine in (0xa641, 0x8664), ('unexpected frontend architecture', name)
        libraries[name] = {'sha256': digest(data), 'exports': sorted(compat.exports(pe)),
                           'imports': sorted(pe.imports())}
    data = (root / DLLS[0]).read_bytes()
    for marker in (b'query-park-v1', b'hookable-swapchain-v1', b'DXMT_TYPELESS_PFV\0',
                   b'color-store-v2', b'upload-flow-v1', b'[PRESENT_WAIT]', b'MADEIRA_LIGHT_EFFECTS'):
        assert marker in data, ('missing prior D3D11 implementation', marker)
    entries = swap.contract(data)
    depth_symbols = []
    if require_depth:
        pe = swap.PE(data)
        _, ranges, _ = swap.hybrid(pe)
        for name, rva in swap.symbols(pe).items():
            if not re.fullmatch(r'#?_ZN4dxmt\d+RenamableTexturePool7getNextEyy(?:\$hp_target)?', name):
                continue
            if any((start & 3) == 1 and (start & ~3) <= rva and
                   rva + 4 <= (start & ~3) + length for start, length in ranges):
                depth_symbols.append({'symbol': name, 'rva': rva})
        assert depth_symbols, 'missing compiled ARM64EC two-argument depth retirement implementation'
    return libraries, entries, sorted(depth_symbols, key=lambda entry: entry['symbol'])


def collect(root, require_depth):
    sources, previous_sources = patch_sources(root)
    pacing = module(root, 'dxmt-pacing-bundle')
    archives = {path.relative_to(root).as_posix(): sha_file(path)
                for path in sorted((root / 'app/Madeira').rglob('*.a'))}
    assert ARCHIVE in archives, 'missing shipped DXMT archive'
    libraries, entries, symbols = inspect_frontends(root, require_depth)
    return {'sources': sources, 'pre_patch_sources': previous_sources,
            'native_archives': archives, 'members': pacing.members(root / ARCHIVE),
            'pe_libraries': libraries, 'swapchain_entries': entries,
            'depth_symbols': symbols}


def read_parents(root):
    return {name: json.loads((root / name).read_text(encoding='utf-8')) for name in PARENTS
            if name not in OPTIONAL_PARENTS or (root / name).exists()}


def validate_parents(root, parents, state, allow_previous_sources=False):
    assert set(PARENTS) - OPTIONAL_PARENTS <= set(parents) <= set(PARENTS), 'missing parent receipt'
    dll = state['pe_libraries'][DLLS[0]]['sha256']
    for name, parent in parents.items():
        assert set(parent['sources']) == producer_sources(root, name), ('incomplete source receipt', name)
        for source, previous in parent['sources'].items():
            current = source_sha(root / source)
            accepted = {current}
            if allow_previous_sources and source in PATCHED:
                accepted.add(state['pre_patch_sources'][source])
            assert previous in accepted, ('unrelated or stale source', name, source)
        if name in NATIVE_PARENTS:
            assert parent['archive_sha256'] == state['native_archives'][ARCHIVE], ('stale native parent', name)
            assert parent['members'] == state['members'], ('stale archive objects', name)
        if name in PE_PARENTS[:2]:
            assert parent['sha256'] == dll, ('stale frontend parent', name)
        elif name == PE_PARENTS[2] or 'd3d11_sha256' in parent:
            assert parent['d3d11_sha256'] == dll, ('stale frontend parent', name)
    assert parents[PE_PARENTS[1]]['entries'] == state['swapchain_entries'], 'stale compiled swapchain ABI'


def prepare(root, output):
    state = collect(root, False)
    parents = read_parents(root)
    validate_parents(root, parents, state, allow_previous_sources=True)
    write_json(output, {'format': 'madeira-depth-retention-base-v1', 'state': state, 'parents': parents})


def incremental_updates(root, snapshot, state, parents, commit):
    assert snapshot['format'] == 'madeira-depth-retention-base-v1'
    old = snapshot['state']
    assert set(parents) == set(snapshot['parents']), 'parent receipt added or removed during refresh'
    assert state['sources'] == old['sources'] and state['pre_patch_sources'] == old['pre_patch_sources'], \
        'depth source changed after preparation'
    assert state['native_archives'] == old['native_archives'], 'frontend refresh changed a native archive'
    assert state['members'] == old['members'], 'frontend refresh changed DXMT archive members'
    for name in DLLS:
        before, after = old['pe_libraries'][name], state['pe_libraries'][name]
        assert before['exports'] == after['exports'] and before['imports'] == after['imports'], \
            ('frontend export/import contract changed', name)
        if name != DLLS[0]:
            assert before == after, ('unbuilt frontend changed', name)
    # Revalidate the original producer bindings against their original binaries
    # and current source; a path allowlist alone must never bless a stale hash.
    validate_parents(root, snapshot['parents'], old, allow_previous_sources=True)
    old_sha = old['pe_libraries'][DLLS[0]]['sha256']
    new_sha = state['pe_libraries'][DLLS[0]]['sha256']
    assert old_sha != new_sha, 'D3D11 was not rebuilt'
    updates = {}
    for name in NATIVE_PARENTS:
        if name not in snapshot['parents']:
            continue
        expected = copy.deepcopy(snapshot['parents'][name])
        # refresh-dxmt-query.sh already performs this narrowly verified rebind.
        # No other change to a native parent's receipt is allowed mid-refresh.
        if name == 'build/dxmt-ios/gpu-pipeline.json' and parents[name] != expected:
            expected['d3d11_sha256'] = new_sha
            expected['frontend_refresh'] = {'commit': commit, 'base_dll_sha256': old_sha,
                                           'receipt': 'build/dxmt-pe/swapchain-hooks.json'}
        assert parents[name] == expected, ('parent changed during frontend refresh', name)
        updated = copy.deepcopy(parents[name])
        changed_sources = {}
        for source, previous in updated['sources'].items():
            current = source_sha(root / source)
            if previous != current:
                assert source in PATCHED and previous == old['pre_patch_sources'][source]
                changed_sources[source] = {'before': previous, 'after': current}
                updated['sources'][source] = current
        if 'd3d11_sha256' in updated:
            updated['d3d11_sha256'] = new_sha
            updated['depth_retention_frontend_refresh'] = {
                'commit': commit, 'base_dll_sha256': old_sha, 'receipt': RECEIPT,
                'native_archive_changed': False, 'changed_sources': changed_sources}
        elif changed_sources:
            raise AssertionError(('native-only parent unexpectedly depends on depth PE source', name))
        updates[name] = updated
    combined = dict(parents, **updates)
    validate_parents(root, combined, state)
    return updates


def record_for(state, parents):
    return {'format': 'madeira-depth-retention-v1',
            'scope': 'arm64ec-d3d11-depth-rename-pool',
            'sources': state['sources'], 'd3d11_sha256': state['pe_libraries'][DLLS[0]]['sha256'],
            # The complete archive inventory is an incremental-transition gate.
            # Persist only our renderer's identity so an unrelated future native
            # input/network refresh does not invalidate this renderer receipt.
            'archive_sha256': state['native_archives'][ARCHIVE], 'members': state['members'],
            'pe_libraries': state['pe_libraries'], 'depth_symbols': state['depth_symbols'],
            'parent_receipts': sorted(parents),
            'active_pool_capacity': 32, 'warm_slots': 2, 'idle_completed_submissions': 300}


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--prepare', type=Path)
    parser.add_argument('--stamp', action='store_true')
    parser.add_argument('--base', type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    assert not args.base or args.stamp, '--base requires --stamp'
    if args.prepare:
        assert not args.stamp and not args.base
        prepare(root, args.prepare)
        print('PASS: original DXMT receipt/binary/source bindings saved before PE refresh')
        return
    state = collect(root, True)
    parents = read_parents(root)
    record = record_for(state, parents)
    if args.stamp:
        commit = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD'], text=True).strip()
        record['producer_commit'] = commit
        updates = {}
        if args.base:
            snapshot = json.loads(args.base.read_text(encoding='utf-8'))
            updates = incremental_updates(root, snapshot, state, parents, commit)
            record['base_d3d11_sha256'] = snapshot['state']['pe_libraries'][DLLS[0]]['sha256']
            record['changed_native_archives'] = []
            record['changed_frontends'] = [DLLS[0]]
        else:
            validate_parents(root, parents, state)
        # No producer receipt is written until every parent, source and binary
        # has passed. Native producer commits/compiler namespaces are retained.
        for name, parent in updates.items():
            write_json(root / name, parent)
        write_json(root / RECEIPT, record)
    else:
        validate_parents(root, parents, state)
        previous = json.loads((root / RECEIPT).read_text(encoding='utf-8'))
        for key, value in record.items():
            assert previous[key] == value, ('stale depth retirement bundle', key)
    print('PASS: compiled depth retirement, exact source, frontend ABI and native archive identities match')


if __name__ == '__main__':
    main()
