#!/usr/bin/env python3
"""Exercise the production isolated-refresh gate, including duplicate ar names."""
from pathlib import Path
import importlib.util
import json
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
tool = root/'tools/dxmt-pacing-bundle.py'
spec = importlib.util.spec_from_file_location('pacing', tool)
pacing = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pacing)
parents = ('gpu-pipeline.json', 'shader-vector-load.json', 'shader-optimizer.json')


def archive(path, changed=False, unrelated=False):
    data = bytearray(b'!<arch>\n')
    entries = [('cache.o', b'cache compiler-identity'), ('duplicate.o', b'first'),
               ('winemetal_unix.o', b'new native' if changed else b'old native'),
               ('duplicate.o', b'CHANGED' if unrelated else b'second')]
    for name, content in entries:
        ar_name = name+'/'
        if len(ar_name) > 16:
            ar_name = '#1/'+str(len(name)); content = name.encode('ascii')+content
        header = f'{ar_name:<16}{0:<12}{0:<6}{0:<6}{100644:<8}{len(content):<10}`\n'.encode('ascii')
        assert len(header) == 60
        data.extend(header); data.extend(content)
        if len(content) & 1: data.extend(b'\n')
    path.write_bytes(data)


with tempfile.TemporaryDirectory(prefix='boundary-lineage-', dir=root/'build') as temporary:
    folder = Path(temporary)
    base = folder/'base.a'; candidate = folder/'candidate.a'
    archive(base); archive(candidate, changed=True)
    # Supply all exact source files required by the production receipt maker.
    sources = ('dxmt/src/winemetal/unix/winemetal_unix.c', 'build/dxmt-ios/present_pacing.h',
               'patches/dxmt-producer-pacing.patch', 'patches/dxmt-gpu-foreground.patch',
               'patches/dxmt-gpu-trace.patch', 'patches/dxmt-gpu-stages.patch',
               'patches/dxmt-texture-headroom.patch', 'build/dxmt-ios/texture_headroom.h')
    for name in sources:
        destination = folder/name; destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes((root/name).read_bytes())
    old = pacing.members(base)
    assert [item['occurrence'] for item in old if item['name'] == 'duplicate.o'] == [1, 2]
    records = folder/'build/dxmt-ios'
    def reset(stale=False):
        for name in parents:
            value = {'archive_sha256': pacing.sha_file(base), 'members': old,
                     'producer_commit': 'original-shader-producer', 'compilerID': 'unchanged-cache-namespace'}
            if stale and name == parents[-1]: value['archive_sha256'] = 'stale'
            (records/name).write_text(json.dumps(value), encoding='utf-8')
    def run():
        return subprocess.run([sys.executable, str(tool), '--root', str(folder), '--archive', str(candidate),
                               '--stamp', '--base', str(base)], capture_output=True, text=True)
    reset(); result = run(); assert result.returncode == 0, result.stdout+result.stderr
    for name in parents:
        value = json.loads((records/name).read_text())
        assert value['archive_sha256'] == pacing.sha_file(candidate) and value['members'] == pacing.members(candidate)
        assert value['producer_commit'] == 'original-shader-producer'
        assert value['compilerID'] == 'unchanged-cache-namespace'
        assert value['boundary_refresh']['base_archive_sha256'] == pacing.sha_file(base)
    # Refuse both unrelated duplicate-member edits and a stale shader parent.
    # A failure must not update even an earlier valid parent receipt.
    for stale, unrelated in ((False, True), (True, False)):
        archive(candidate, changed=True, unrelated=unrelated); reset(stale)
        before = {name: (records/name).read_bytes() for name in parents}
        result = run(); assert result.returncode != 0, 'invalid refresh unexpectedly accepted'
        assert all((records/name).read_bytes() == data for name, data in before.items())
print('PASS: isolated native refresh preserves all shader objects, producers/cache IDs and duplicate members; invalid parents fail before receipt writes')
