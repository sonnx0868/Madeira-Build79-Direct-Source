#!/usr/bin/env python3
"""Exercise the complete optimizer receipt transition and its failure gates.

Default uses controlled Mach-O/PE fixtures, not compiled shader behavior.
--baseline-zip additionally validates every real862 archive/PE/source receipt,
including the compiled native callback bypass. All writes stay in a temporary
directory. This test never replaces the production renderer or game runtime.
"""
from pathlib import Path
import argparse
import copy
import importlib.util
import json
import shutil
import struct
import subprocess
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parents[2]


def load(path):
    spec = importlib.util.spec_from_file_location(path.stem.replace('-', '_'), path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


bundle = load(ROOT / 'tools/dxmt-optimizer-bundle.py')
depth = load(ROOT / 'tools/dxmt-depth-retention-bundle.py')
pipeline = load(ROOT / 'tests/host/check-dxmt-patches.py')
counter = 0


def rejected(action):
    global counter
    try:
        action()
    except (AssertionError, KeyError, FileNotFoundError):
        counter += 1
        return
    raise AssertionError('invalid optimizer transition accepted')


def object_fixture(production_symbol=True):
    # Controlled Mach-O object with a defined run symbol. It is not executable
    # shader code; actual production object/compiler tests run separately.
    name = b'__ZN4llvm6detail9PassModelINS_8FunctionEN4dxmt9shaderopt14ComputePCFPassEE3runE\0'
    strings = b'\0' + name
    payload = b'SinkingPass\0LoopRotatePass\0pcf.sample\0pcf.condition\0fixture-only\0'
    header = struct.pack('<8I', 0xfeedfacf, 0x100000c, 0, 1, 1, 24, 0, 0)
    command = struct.pack('<6I', 2, 24, 56, 1, 72, len(strings))
    symbol = struct.pack('<IBBHQ', 1, 0x0f if production_symbol else 1, 1 if production_symbol else 0, 0, 0)
    return header + command + symbol + strings + payload


def archive(path, objects):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('wb') as output:
        output.write(b'!<arch>\n')
        for name, body in objects:
            data = name.encode() + body
            header = f'{"#1/" + str(len(name)):<16}{0:<12}{0:<6}{0:<6}{100644:<8}{len(data):<10}`\n'.encode()
            assert len(header) == 60
            output.write(header + data + (b'\n' if len(data) % 2 else b''))


def replace_objects(base, output, replacements):
    # Preserve each member verbatim, including duplicate names and archive
    # metadata. Only designated member payloads are rebuilt as BSD ar entries.
    with base.open('rb') as source, output.open('wb') as target:
        assert source.read(8) == b'!<arch>\n'
        target.write(b'!<arch>\n')
        changed = set()
        while header := source.read(60):
            size = int(header[48:58])
            data = source.read(size)
            padding = source.read(1) if size & 1 else b''
            name = header[:16].decode().strip()
            if name.startswith('#1/'):
                length = int(name[3:])
                name = data[:length].rstrip(b'\0').decode()
            else:
                name = name.rstrip('/')
            if name in replacements:
                assert name not in changed
                changed.add(name)
                data = name.encode() + replacements[name]
                header = f'{"#1/" + str(len(name)):<16}{0:<12}{0:<6}{0:<6}{100644:<8}{len(data):<10}`\n'.encode()
                padding = b'\n' if len(data) & 1 else b''
            target.write(header + data + padding)
        assert changed == set(replacements)


def source_fixture(folder):
    names = set(bundle.SOURCES) | set(depth.SOURCES)
    for receipt in depth.PARENTS:
        names.update(depth.producer_sources(ROOT, receipt))
    for path in (ROOT / 'tools').glob('*.py'):
        names.add(path.relative_to(ROOT).as_posix())
    names.add('patches/wine-native-callback-context.patch')
    for name in names:
        target = folder / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / name, target)
    for name, source in pipeline.patched_sources(ROOT).items():
        target = folder / 'dxmt' / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(source, encoding='utf-8', newline='\n')
    # Baseline source reconstruction itself runs against the real frozen git
    # commits once; the scratch directory has no .git and is never trusted as
    # a substitute for those source hashes.
    expected = bundle.legacy_source_hashes(ROOT)
    bundle.legacy_source_hashes = lambda root: expected
    pin = subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', bundle.BASE_COMMIT + ':wine'], text=True).strip()
    target = folder / 'wine/dlls/ntdll/signal_arm64ec.c'
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(bundle.git_bytes(ROOT / 'wine', pin, 'dlls/ntdll/signal_arm64ec.c').replace(b'\r\n', b'\n'))
    for index, name in enumerate(('patches/wine-exec-callback.patch', 'patches/wine-native-callback-context.patch',
                                  'patches/wine-stackwalk-jit-alias.patch')):
        patch = folder / (str(index) + '-wine.patch')
        patch.write_bytes(bundle.git_bytes(ROOT, bundle.BASE_COMMIT, name).replace(b'\r\n', b'\n'))
        subprocess.run(['git', '-C', str(folder / 'wine'), 'apply', '--include=dlls/ntdll/signal_arm64ec.c',
                        str(patch)], check=True, capture_output=True)
    return expected


def synthetic_inspection(root, require_depth):
    libraries = {name: {'sha256': bundle.sha(root / name), 'exports': ['fixture'], 'imports': ['ntdll.dll']}
                 for name in depth.DLLS}
    return libraries, [{'abi': 'fixture'}], [{'symbol': 'fixture-depth', 'rva': 1}]


def synthetic_baseline(folder, legacy_sources):
    depth.inspect_frontends = synthetic_inspection
    real_module = bundle.module
    bundle.module = lambda root, name: depth if name == 'dxmt-depth-retention-bundle' else real_module(root, name)
    bundle.validate_renpy = lambda root: None  # Real compiled proof is exercised by --baseline-zip.
    objects = [('winemetal_unix.o', b'unchanged'), ('airconv_context.o', b'old optimizer'),
               ('cache.o', b'old cache\0' + b'a' * 64 + b'\0'), ('duplicate.o', b'first'), ('duplicate.o', b'second')]
    archive(folder / bundle.ARCHIVE, objects)
    archive(folder / 'app/Madeira/unrelated.a', [('winemetal_unix.o', b'unrelated')])
    for name in (*depth.DLLS, bundle.NTDLL):
        path = folder / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b'controlled PE ' + name.encode())
    bundle.BASE_ARCHIVE_SHA256 = bundle.sha(folder / bundle.ARCHIVE)
    bundle.BASE_NTDLL_SHA256 = bundle.sha(folder / bundle.NTDLL)
    bundle.write_json(folder / bundle.RENPY_RECEIPT, {'fixture': 'unchanged native callback receipt'})
    state = depth.collect(folder, True)
    parents = {}
    for name in depth.PARENTS:
        parent = {'sources': {s: bundle.source_sha(folder / s) for s in depth.producer_sources(folder, name)},
                  'producer_commit': 'original producer ' + name}
        if name in depth.NATIVE_PARENTS:
            parent.update(archive_sha256=state['native_archives'][bundle.ARCHIVE], members=state['members'])
        if name in depth.PE_PARENTS[:2]:
            parent['sha256'] = state['pe_libraries'][depth.DLLS[0]]['sha256']
        elif name == depth.PE_PARENTS[2] or name in (depth.NATIVE_PARENTS[1], *depth.NATIVE_PARENTS[4:]):
            parent['d3d11_sha256'] = state['pe_libraries'][depth.DLLS[0]]['sha256']
        if name == depth.PE_PARENTS[1]:
            parent['entries'] = state['swapchain_entries']
        if name == bundle.RECEIPT:
            parent.update(format='madeira-shader-optimizer-v1', sources={s: legacy_sources[s] for s in bundle.LEGACY_SOURCES},
                          contextObjectSHA256=bundle.digest(b'old optimizer'), compilerID='a' * 64)
        if name == 'build/dxmt-ios/shader-vector-load.json':
            parent['compilerID'] = 'a' * 64
        parents[name] = parent
        bundle.write_json(folder / name, parent)
    record = depth.record_for(state, parents)
    record['producer_commit'] = 'original depth producer'
    bundle.write_json(folder / depth.RECEIPT, record)


def exercise(folder):
    original_proof = bundle.replacement_proof(object_fixture())
    assert original_proof['defined_run_symbols']
    rejected(lambda: bundle.replacement_proof(object_fixture(False)))
    rejected(lambda: bundle.replacement_proof(object_fixture().replace(b'pcf.sample', b'not.sample')))
    out = folder / bundle.FOLDER
    out.mkdir(parents=True, exist_ok=True)
    base = out / 'base.a'
    shutil.copyfile(folder / bundle.ARCHIVE, base)
    context = out / 'airconv_context.o'
    context.write_bytes(object_fixture())
    snapshot = bundle.prepare(folder, base, context)
    receipt_before = bundle.receipt_hashes(folder)
    parents = copy.deepcopy(snapshot['parents'])
    # Preparation must reject a stale/missing source field before writes.
    old = copy.deepcopy(parents[bundle.RECEIPT])
    bad = copy.deepcopy(old)
    bad['sources'][bundle.CONTEXT] = '0' * 64
    rejected(lambda: bundle.validate_legacy(folder, bad, snapshot['state']))
    bad['sources'] = {key: value for key, value in old['sources'].items() if key != bundle.CONTEXT}
    rejected(lambda: bundle.validate_legacy(folder, bad, snapshot['state']))
    rejected(lambda: bundle.validate_legacy(folder, old, dict(snapshot['state'], native_archives={})))
    source = folder / bundle.CONTEXT
    before_source = source.read_bytes()
    source.write_bytes(before_source + b'\n// unrelated source modification\n')
    rejected(lambda: bundle.validate_legacy(folder, old, snapshot['state']))
    source.write_bytes(before_source)
    cache = out / 'cache.o'
    cache.write_bytes(b'rebuilt controlled cache\0' + snapshot['compilerID'].encode() + b'\0')
    replace_objects(base, folder / bundle.ARCHIVE, {'airconv_context.o': context.read_bytes(), 'cache.o': cache.read_bytes()})
    state = bundle.collect(folder)
    def transition(after=state, receipts=parents, prepared=snapshot):
        return bundle.transition(folder, base, prepared, after, receipts, 'new-optimizer-producer')
    updates = transition()
    assert set(updates) == set(depth.NATIVE_PARENTS) | {depth.RECEIPT}
    for name, updated in updates.items():
        if name == bundle.RECEIPT:
            assert updated['producer_commit'] == 'new-optimizer-producer'
            continue
        original = snapshot['depth_receipt'] if name == depth.RECEIPT else parents[name]
        assert updated['producer_commit'] == original['producer_commit']
        expected = copy.deepcopy(original)
        expected.update(archive_sha256=state['native_archives'][bundle.ARCHIVE], members=state['members'],
                        optimizer_refresh=updated['optimizer_refresh'])
        if name == 'build/dxmt-ios/shader-vector-load.json':
            expected['compilerID'] = snapshot['compilerID']
        assert updated == expected, ('unrelated receipt content changed', name)
    assert bundle.receipt_hashes(folder) == receipt_before, 'transition wrote before caller committed updates'
    for key in ('sources', 'pre_patch_sources', 'optimizer_sources', 'pe_payloads', 'receipt_hashes', 'native_archives'):
        bad = copy.deepcopy(state)
        name = next(iter(bad[key]))
        bad[key][name] = 'changed'
        rejected(lambda bad=bad: transition(after=bad))
    for key in ('pe_payloads', 'receipt_hashes', 'native_archives'):
        bad = copy.deepcopy(state)
        bad[key].pop(next(iter(bad[key])))
        rejected(lambda bad=bad: transition(after=bad))
    for member in ('winemetal_unix.o', 'duplicate.o'):
        bad = copy.deepcopy(state)
        matches = [m for m in bad['members'] if m['name'] == member]
        if matches:
            matches[-1]['sha256'] = 'changed'
            rejected(lambda bad=bad: transition(after=bad))
    bad = copy.deepcopy(state)
    bad['members'].reverse()
    rejected(lambda: transition(after=bad))
    bad_parents = copy.deepcopy(parents)
    bad_parents.pop('build/dxmt-ios/retirement.json')
    rejected(lambda: transition(receipts=bad_parents))
    bad_parents = copy.deepcopy(parents)
    bad_parents['build/dxmt-ios/gpu-pipeline.json']['producer_commit'] = 'relabelled'
    rejected(lambda: transition(receipts=bad_parents))
    for key in ('compilerID', 'oldCompilerID', 'contextObjectSHA256'):
        bad = copy.deepcopy(snapshot)
        bad[key] = 'changed'
        rejected(lambda bad=bad: transition(prepared=bad))
    for label in ('cache.o', 'airconv_context.o'):
        path = out / label
        before = path.read_bytes()
        path.write_bytes(before + b'changed after compile')
        rejected(transition)
        path.write_bytes(before)
    assert bundle.receipt_hashes(folder) == receipt_before, 'rejected transition mutated receipts'
    for name, updated in updates.items():
        bundle.write_json(folder / name, updated)
    bundle.verify(folder)
    # Source binding continues to reject changes after an accepted transition.
    header = folder / 'build/dxmt-ios/guarded_pcf.hpp'
    old_header = header.read_bytes()
    header.write_bytes(old_header + b'\n// uncompiled change\n')
    rejected(lambda: bundle.verify(folder))
    header.write_bytes(old_header)
    bundle.verify(folder)
    print(f'PASS: optimizer receipt transition; {counter} invalid source/member/PE/cache/receipt changes rejected before any write; original producers and RenPy payload preserved')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline-zip', type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='madeira-optimizer-receipts-') as work:
        folder = Path(work)
        legacy_sources = source_fixture(folder)
        if args.baseline_zip:
            assert bundle.sha(args.baseline_zip) == 'a4ed742120877cb9789ad0c01195d29f3293e829967f3881576c8a764809f1c7', 'expected verified862 native bundle'
            with zipfile.ZipFile(args.baseline_zip) as archive_file:
                for entry in archive_file.infolist():
                    parts = Path(entry.filename).parts
                    wanted = entry.filename.startswith('app/Madeira/') and (
                        entry.filename.endswith(('.a', '.json')) or (len(parts) > 2 and parts[2].endswith('-windows')))
                    wanted |= entry.filename.endswith('.json') and any(entry.filename.startswith(name + '/') for name in bundle.RECEIPT_DIRS)
                    if not wanted or entry.is_dir():
                        continue
                    target = (folder / entry.filename).resolve()
                    assert target.is_relative_to(folder)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with archive_file.open(entry) as source, target.open('wb') as out:
                        shutil.copyfileobj(source, out)
            print('Real862 fixture: checking all native objects, all Windows payloads, depth ABI and compiled native callback bypass')
        else:
            synthetic_baseline(folder, legacy_sources)
        exercise(folder)


if __name__ == '__main__':
    main()
