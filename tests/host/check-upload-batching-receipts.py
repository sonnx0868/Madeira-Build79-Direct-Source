#!/usr/bin/env python3
"""Exercise upload-refresh provenance without modifying a shipped runtime.

Default uses controlled non-executable payloads to test mutation/rejection
boundaries. --baseline-zip also verifies the actual868 archive, compiled PE,
native callback proof and complete original receipt chain in a scratch tree.
"""
from pathlib import Path
import argparse
import copy
import importlib.util
import json
import shutil
import subprocess
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parents[2]


def load(path):
    spec = importlib.util.spec_from_file_location(path.stem.replace('-', '_'), path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


bundle = load(ROOT / 'tools/dxmt-upload-batching-bundle.py')
depth = load(ROOT / 'tools/dxmt-depth-retention-bundle.py')
optimizer = load(ROOT / 'tools/dxmt-optimizer-bundle.py')
rejections = 0


def rejected(action):
    global rejections
    try:
        action()
    except (AssertionError, KeyError, FileNotFoundError):
        rejections += 1
        return
    raise AssertionError('invalid upload receipt transition accepted')


def source_fixture(folder):
    names = set(bundle.SOURCES) | set(depth.SOURCES)
    for name in depth.PARENTS:
        names.update(depth.producer_sources(ROOT, name))
    names.update(path.relative_to(ROOT).as_posix() for path in (ROOT / 'tools').glob('*.py'))
    names.update(('wine/dlls/ntdll/signal_arm64ec.c', 'patches/wine-native-callback-context.patch'))
    for name in names:
        target = folder / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / name, target)
    # A host checkout can retain an older Wine overlay while DXMT is under
    # review. Reconstruct the build pipeline's exact native-callback input in
    # scratch, rather than changing the user's submodule or weakening its hash.
    pin = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD:wine'], text=True).strip()
    target = folder / 'wine/dlls/ntdll/signal_arm64ec.c'
    target.write_bytes(subprocess.check_output(['git', '-C', str(ROOT / 'wine'), 'show',
                                               pin + ':dlls/ntdll/signal_arm64ec.c']).replace(b'\r\n', b'\n'))
    for index, name in enumerate(('patches/wine-exec-callback.patch', 'patches/wine-native-callback-context.patch',
                                  'patches/wine-stackwalk-jit-alias.patch')):
        patch = folder / (str(index) + '-wine.patch')
        patch.write_bytes((ROOT / name).read_bytes().replace(b'\r\n', b'\n'))
        subprocess.run(['git', '-C', str(folder / 'wine'), 'apply', '--include=dlls/ntdll/signal_arm64ec.c',
                        str(patch)], check=True, capture_output=True)


def archive(path, objects):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('wb') as output:
        output.write(b'!<arch>\n')
        for name, body in objects:
            data = name.encode() + body
            header = f'{"#1/" + str(len(name)):<16}{0:<12}{0:<6}{0:<6}{100644:<8}{len(data):<10}`\n'.encode()
            assert len(header) == 60
            output.write(header + data + (b'\n' if len(data) % 2 else b''))


def synthetic_inspection(root, require_depth):
    libraries = {name: {'sha256': bundle.sha(root / name), 'exports': ['controlled-export'], 'imports': ['controlled-import']}
                 for name in depth.DLLS}
    code = bundle.sha(root / bundle.DLL)
    return libraries, [{'abi': 'controlled-ABI', 'relocated-code': code}], [{'symbol': 'controlled-depth', 'rva': len(code)}]


def synthetic_baseline(folder):
    depth.inspect_frontends = synthetic_inspection
    real_module = bundle.module
    def fake_module(root, name):
        if name == 'dxmt-depth-retention-bundle':
            return depth
        if name == 'dxmt-optimizer-bundle':
            return optimizer
        return real_module(root, name)
    bundle.module = fake_module
    optimizer.validate_renpy = lambda root: None  # Actual proof runs with --baseline-zip.
    bundle.compiled_context = lambda root: {'format': 'controlled-fixture', 'sha256': bundle.sha(root / bundle.DLL)}
    archive(folder / bundle.ARCHIVE, [('winemetal_unix.o', b'unchanged boundary'),
            ('airconv_context.o', b'unchanged PCF'), ('cache.o', b'cache\0' + b'a' * 64 + b'\0'),
            ('dxmt_context.o', b'old native context'), ('duplicate.o', b'first'), ('duplicate.o', b'second')])
    archive(folder / 'app/Madeira/libunrelated.a', [('winemetal_unix.o', b'unrelated native')])
    for name in (*depth.DLLS, bundle.NTDLL, 'app/Madeira/i386-windows/ntdll.dll'):
        target = folder / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(('controlled executable fixture ' + name).encode())
    bundle.write_json(folder / optimizer.RENPY_RECEIPT, {'fixture': 'unchanged callback receipt'})
    state = bundle.collect(folder)
    parents = {}
    for name in depth.PARENTS:
        parent = {'sources': {s: bundle.source_sha(folder / s) for s in depth.producer_sources(folder, name)},
                  'producer_commit': 'original producer ' + name}
        if bundle.CONTEXT in parent['sources']:
            parent['sources'][bundle.CONTEXT] = state['pre_upload_context']
        if name in depth.NATIVE_PARENTS:
            parent.update(archive_sha256=state['native_archives'][bundle.ARCHIVE], members=state['members'])
        if name in depth.PE_PARENTS[:2]:
            parent['sha256'] = state['pe_payloads'][bundle.DLL]
        elif name == depth.PE_PARENTS[2] or name in (depth.NATIVE_PARENTS[1], *depth.NATIVE_PARENTS[4:]):
            parent['d3d11_sha256'] = state['pe_payloads'][bundle.DLL]
        if name == depth.PE_PARENTS[1]:
            parent['entries'] = state['swapchain_entries']
        if name in (optimizer.RECEIPT, 'build/dxmt-ios/shader-vector-load.json'):
            parent['compilerID'] = 'a' * 64
        if name == optimizer.RECEIPT:
            parent['contextObjectSHA256'] = bundle.digest(b'unchanged PCF')
        parents[name] = parent
        bundle.write_json(folder / name, parent)
    record = depth.record_for(state, parents)
    record['producer_commit'] = 'original depth producer'
    bundle.write_json(folder / depth.RECEIPT, record)
    bundle.BASE_ARCHIVE = state['native_archives'][bundle.ARCHIVE]
    bundle.BASE_DLL = state['pe_payloads'][bundle.DLL]
    bundle.BASE_NTDLL = state['pe_payloads'][bundle.NTDLL]
    bundle.BASE_COMPILER = 'a' * 64
    bundle.BASE_CONTEXT = state['pre_upload_context']


def exercise(folder):
    snapshot = bundle.prepare(folder, folder / bundle.FOLDER / 'prepared.json')
    original_receipts = optimizer.receipt_hashes(folder)
    old = snapshot['state']
    parents = snapshot['parents']
    assert old['native_archives'][bundle.ARCHIVE] == bundle.BASE_ARCHIVE
    bad = copy.deepcopy(parents)
    bad['build/dxmt-ios/gpu-pipeline.json']['sources'][bundle.CONTEXT] = 'stale'
    rejected(lambda: bundle.validate_parents(folder, bad, old, previous=True))
    bad = copy.deepcopy(parents)
    bad['build/dxmt-ios/gpu-pipeline.json']['sources'].pop(bundle.CONTEXT)
    rejected(lambda: bundle.validate_parents(folder, bad, old, previous=True))
    rejected(lambda: bundle.transition(folder, snapshot, old, parents, 'new'))
    dll = folder / bundle.DLL
    dll.write_bytes(dll.read_bytes() + b' new scheduler')
    state = bundle.collect(folder)
    def transition(after=state, prepared=snapshot, inherited=parents):
        return bundle.transition(folder, prepared, after, inherited, 'new-upload-producer')
    updates = transition()
    assert optimizer.receipt_hashes(folder) == original_receipts, 'transition wrote before validation completed'
    for name, updated in updates.items():
        original = snapshot['depth_receipt'] if name == depth.RECEIPT else parents[name]
        assert updated['producer_commit'] == original['producer_commit'], 'old code relabelled as new'
        if 'members' in original:
            assert updated['members'] == original['members']
            assert updated['archive_sha256'] == original['archive_sha256']
        if 'compilerID' in original:
            assert updated['compilerID'] == original['compilerID']
        binding = updated['upload_batching_frontend_refresh']
        assert binding['scope'] == 'arm64ec-d3d11-only'
        assert binding['native_archive_changed'] is False and binding['native_context_recompiled'] is False
    for key in ('sources', 'pre_patch_sources', 'upload_sources', 'receipt_hashes', 'native_archives'):
        bad = copy.deepcopy(state)
        bad[key][next(iter(bad[key]))] = 'changed'
        rejected(lambda bad=bad: transition(after=bad))
    for key in ('pe_payloads', 'native_archives', 'receipt_hashes'):
        bad = copy.deepcopy(state)
        bad[key].pop(next(iter(bad[key])))
        rejected(lambda bad=bad: transition(after=bad))
    bad = copy.deepcopy(state)
    bad['compiled_context'] = old['compiled_context']
    rejected(lambda: transition(after=bad))
    for name in (bundle.NTDLL, depth.DLLS[1], 'app/Madeira/i386-windows/ntdll.dll'):
        bad = copy.deepcopy(state)
        bad['pe_payloads'][name] = 'changed'
        rejected(lambda bad=bad: transition(after=bad))
    for field in ('imports', 'exports'):
        bad = copy.deepcopy(state)
        bad['pe_libraries'][bundle.DLL][field].append('new ABI')
        rejected(lambda bad=bad: transition(after=bad))
    bad = copy.deepcopy(state)
    bad['members'][-1]['sha256'] = 'duplicate native member changed'
    rejected(lambda: transition(after=bad))
    bad = copy.deepcopy(parents)
    bad['build/dxmt-ios/shader-optimizer.json']['producer_commit'] = 'relabelled'
    rejected(lambda: transition(inherited=bad))
    bad = copy.deepcopy(snapshot)
    bad['parents']['build/dxmt-ios/shader-optimizer.json']['compilerID'] = 'changed'
    rejected(lambda: transition(prepared=bad, inherited=bad['parents']))
    source = folder / bundle.CONTEXT
    previous = source.read_bytes()
    source.write_bytes(previous + b'\n// unrelated context change\n')
    rejected(lambda: bundle.prepare(folder, folder / bundle.FOLDER / 'invalid.json'))
    source.write_bytes(previous)
    assert optimizer.receipt_hashes(folder) == original_receipts, 'rejected update modified receipts'
    bundle.write_updates(folder, updates)
    combined = depth.read_parents(folder)
    bundle.validate_parents(folder, combined, state)
    bundle.validate_depth(folder, state, combined, bundle.read_json(folder / depth.RECEIPT))
    record = bundle.record_for(folder, state)
    assert record['d3d11_sha256'] != old['pe_payloads'][bundle.DLL]
    assert record['archive_sha256'] == old['native_archives'][bundle.ARCHIVE]
    assert record['renpy_ntdll_sha256'] == old['pe_payloads'][bundle.NTDLL]


def actual_baseline(path):
    global bundle, depth, optimizer
    # Reload pristine modules, removing fixture-only inspectors/pins.
    bundle = load(ROOT / 'tools/dxmt-upload-batching-bundle.py')
    depth = load(ROOT / 'tools/dxmt-depth-retention-bundle.py')
    optimizer = load(ROOT / 'tools/dxmt-optimizer-bundle.py')
    with tempfile.TemporaryDirectory(prefix='madeira-upload-real868-') as work:
        folder = Path(work)
        source_fixture(folder)
        with zipfile.ZipFile(path) as archive:
            for member in archive.infolist():
                name = member.filename
                if member.is_dir() or not (name.startswith('app/Madeira/') or
                   (name.startswith('build/') and name.endswith('.json'))):
                    continue
                target = (folder / name).resolve()
                assert target.is_relative_to(folder.resolve()), 'archive path escape'
                target.parent.mkdir(parents=True, exist_ok=True)
                with archive.open(member) as source, target.open('wb') as output:
                    shutil.copyfileobj(source, output)
        snapshot = bundle.prepare(folder, folder / bundle.FOLDER / 'prepared.json')
        assert snapshot['state']['compiled_context']['size'] >= 512
        print('PASS: actual868 native archives, PE contracts, flush code, PCF cache namespace and RenPy callback proof verified')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline-zip', type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='madeira-upload-receipts-') as work:
        folder = Path(work)
        source_fixture(folder)
        synthetic_baseline(folder)
        exercise(folder)
    print(f'PASS: upload receipt transition and {rejections} stale/mutated input rejection cases; all native objects and unrelated PE preserved')
    if args.baseline_zip:
        actual_baseline(args.baseline_zip)


if __name__ == '__main__':
    main()
