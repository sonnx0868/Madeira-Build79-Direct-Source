#!/usr/bin/env python3
"""Exercise receipt migration failures; optionally validate a real base bundle.

The synthetic transition isolates receipt/source/archive checks. It does not
pretend to compile or execute Metal. --baseline-zip additionally parses the
real ARM64EC DLLs/ABI and verifies all original producer bindings in isolation.
"""
from pathlib import Path
import argparse
import copy
import importlib.util
import json
import shutil
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parents[2]


def load(path):
    spec = importlib.util.spec_from_file_location(path.stem.replace('-', '_'), path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result


bundle = load(ROOT / 'tools/dxmt-depth-retention-bundle.py')
pipeline = load(ROOT / 'tests/host/check-dxmt-patches.py')


def rejected(action):
    try:
        action()
    except (AssertionError, KeyError):
        return
    raise AssertionError('invalid receipt transition accepted')


def source_fixture(folder):
    names = set(bundle.SOURCES)
    scripts = {script for script, _ in bundle.PRODUCERS.values()}
    scripts.update(('cpu-aot-survey', 'check-compatibility-binaries'))
    for receipt in bundle.PARENTS:
        names.update(bundle.producer_sources(ROOT, receipt))
    names.update('tools/' + script + '.py' for script in scripts)
    for name in names:
        dest = folder / name
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / name, dest)
    for name, text in pipeline.patched_sources(ROOT).items():
        dest = folder / 'dxmt' / name
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_text(text, encoding='utf-8', newline='\n')


def archive(path):
    path.parent.mkdir(parents=True, exist_ok=True)
    result = bytearray(b'!<arch>\n')
    for name, data in (('winemetal_unix.o', b'unchanged native'), ('other.o', b'one'), ('other.o', b'two')):
        data = name.encode() + data
        header = f'{"#1/" + str(len(name)):<16}{0:<12}{0:<6}{0:<6}{100644:<8}{len(data):<10}`\n'.encode()
        assert len(header) == 60
        result += header + data + (b'\n' if len(data) % 2 else b'')
    path.write_bytes(result)


def synthetic_inspection(root, require_depth):
    libraries = {name: {'sha256': bundle.sha_file(root / name), 'exports': ['entry'], 'imports': ['ntdll.dll']}
                 for name in bundle.DLLS}
    if require_depth:
        assert b'new depth' in (root / bundle.DLLS[0]).read_bytes()
    return libraries, [{'verified': 'fixture ABI'}], [{'symbol': 'controlled fixture', 'rva': 1}] if require_depth else []


def synthetic_transition(folder):
    bundle.inspect_frontends = synthetic_inspection
    archive(folder / bundle.ARCHIVE)
    archive(folder / 'app/Madeira/unrelated.a')
    for name in bundle.DLLS:
        path = folder / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b'old fixture ' + name.encode())
    state = bundle.collect(folder, False)
    parents = {}
    for name in bundle.PARENTS:
        parent = {'sources': {s: bundle.source_sha(folder / s) for s in bundle.producer_sources(folder, name)},
                  'producer_commit': 'original-native-producer'}
        if name in bundle.NATIVE_PARENTS:
            parent.update(archive_sha256=state['native_archives'][bundle.ARCHIVE], members=state['members'])
        if name in bundle.PE_PARENTS[:2]:
            parent['sha256'] = state['pe_libraries'][bundle.DLLS[0]]['sha256']
        elif name == bundle.PE_PARENTS[2] or name in (bundle.NATIVE_PARENTS[1], *bundle.NATIVE_PARENTS[4:]):
            parent['d3d11_sha256'] = state['pe_libraries'][bundle.DLLS[0]]['sha256']
        if name == bundle.PE_PARENTS[1]:
            parent['entries'] = state['swapchain_entries']
        parents[name] = parent
        bundle.write_json(folder / name, parent)
    checkpoint = folder / 'base.json'
    bundle.prepare(folder, checkpoint)
    snapshot = json.loads(checkpoint.read_text())
    (folder / bundle.DLLS[0]).write_bytes(b'new depth fixture')
    current = bundle.collect(folder, True)
    now = copy.deepcopy(parents)
    old_sha = state['pe_libraries'][bundle.DLLS[0]]['sha256']
    new_sha = current['pe_libraries'][bundle.DLLS[0]]['sha256']
    for name in bundle.PE_PARENTS:
        now[name]['sha256' if name in bundle.PE_PARENTS[:2] else 'd3d11_sha256'] = new_sha
    gpu = 'build/dxmt-ios/gpu-pipeline.json'
    now[gpu]['d3d11_sha256'] = new_sha
    now[gpu]['frontend_refresh'] = {'commit': 'new-commit', 'base_dll_sha256': old_sha,
                                    'receipt': 'build/dxmt-pe/swapchain-hooks.json'}
    def transition(base=snapshot, after=current, receipts=now):
        return bundle.incremental_updates(folder, base, after, receipts, 'new-commit')
    updates = transition()
    for name, parent in updates.items():
        assert parent['producer_commit'] == parents[name]['producer_commit']
        assert parent['archive_sha256'] == parents[name]['archive_sha256']
        if 'd3d11_sha256' in parent:
            assert parent['d3d11_sha256'] == new_sha
    assert not ('native_archives' in bundle.record_for(current, now))
    assert bundle.record_for(current, now)['archive_sha256'] == state['native_archives'][bundle.ARCHIVE]
    bad = copy.deepcopy(snapshot)
    bad['parents'][bundle.PE_PARENTS[0]]['sha256'] = 'stale'
    rejected(lambda: transition(base=bad))
    bad = copy.deepcopy(snapshot)
    bad['parents'][bundle.PE_PARENTS[0]]['sources'].pop(next(iter(bad['parents'][bundle.PE_PARENTS[0]]['sources'])))
    rejected(lambda: transition(base=bad))
    bad = copy.deepcopy(snapshot)
    key = next(iter(bad['parents'][bundle.PE_PARENTS[0]]['sources']))
    bad['parents'][bundle.PE_PARENTS[0]]['sources'][key] = '0' * 64
    rejected(lambda: transition(base=bad))
    for group, key in (('native_archives', 'app/Madeira/unrelated.a'), ('sources', bundle.PATCH)):
        bad = copy.deepcopy(current)
        bad[group][key] = 'changed'
        rejected(lambda: transition(after=bad))
    for key in ('sha256', 'exports', 'imports'):
        bad = copy.deepcopy(current)
        bad['pe_libraries'][bundle.DLLS[1]][key] = 'changed'
        rejected(lambda: transition(after=bad))
    bad = copy.deepcopy(now)
    bad[gpu]['producer_commit'] = 'falsely-new-native-producer'
    rejected(lambda: transition(receipts=bad))
    bad = copy.deepcopy(now)
    bad.pop(next(iter(bundle.OPTIONAL_PARENTS)))
    rejected(lambda: transition(receipts=bad))
    # Cold builds omit these two incremental shader receipts. Their absence
    # must be consistent across the entire transition and retained in receipt.
    cold = copy.deepcopy(snapshot)
    cold_now = copy.deepcopy(now)
    for name in bundle.OPTIONAL_PARENTS:
        cold['parents'].pop(name)
        cold_now.pop(name)
    transition(base=cold, receipts=cold_now)
    rejected(lambda: transition(base=cold))
    print('PASS: strict old-source/PE/native binding; no missing parent, source drift, archive mutation or unbuilt PE accepted')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline-zip', type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='madeira-depth-bundle-test-') as work:
        folder = Path(work)
        source_fixture(folder)
        if args.baseline_zip:
            with zipfile.ZipFile(args.baseline_zip) as stored:
                wanted = set(bundle.PARENTS) | set(bundle.DLLS)
                for entry in stored.infolist():
                    if entry.filename in wanted or (entry.filename.startswith('app/Madeira/') and entry.filename.endswith('.a')):
                        dest = (folder / entry.filename).resolve()
                        assert dest.is_relative_to(folder), 'unsafe archive path'
                        dest.parent.mkdir(parents=True, exist_ok=True)
                        with stored.open(entry) as source, dest.open('wb') as out:
                            shutil.copyfileobj(source, out)
            bundle.prepare(folder, folder / 'verified-base.json')
            # A source-only patch must not stamp an old DLL as newly compiled.
            rejected(lambda: bundle.inspect_frontends(folder, True))
            print('PASS: actual supplied base archive, ARM64EC swapchain ABI and all source receipts validate; old DLL rejected as depth rebuild')
        else:
            synthetic_transition(folder)


if __name__ == '__main__':
    main()
