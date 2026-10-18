#!/usr/bin/env python3
"""Bind an isolated shader emitter/cache refresh to the actual archive bytes."""
from pathlib import Path
import argparse, hashlib, importlib.util, json, re, subprocess

SOURCES = ('build/dxmt-ios/bounded_srv_load.hpp', 'patches/dxmt-bounded-srv-vector-load.patch',
           'dxmt/src/airconv/nt/dxbc_converter_base.cpp', 'build/dxmt-ios/refresh-srv.sh')

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def object_bytes(path, wanted):
    matches = []
    with path.open('rb') as f:
        assert f.read(8) == b'!<arch>\n'
        while header := f.read(60):
            assert len(header) == 60 and header[-2:] == b'`\n'
            size = int(header[48:58]); data = f.read(size)
            assert len(data) == size
            if size & 1: assert f.read(1)
            name = header[:16].decode('ascii').strip()
            if name.startswith('#1/'):
                n = int(name[3:]); name, data = data[:n].rstrip(b'\0').decode(), data[n:]
            else: name = name.rstrip('/')
            if name == wanted: matches.append(data)
    assert len(matches) == 1, (wanted, len(matches))
    return matches[0]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, required=True)
    p.add_argument('--base', type=Path)
    p.add_argument('--prepare', type=Path, help='Compiled replacement dxbc_converter_base.o')
    p.add_argument('--stamp', action='store_true')
    a = p.parse_args(); root = a.root.resolve()
    folder = root/'build/dxmt-ios/srv-refresh'; receipt = root/'build/dxmt-ios/shader-vector-load.json'
    archive = root/'app/Madeira/libdxmt_combined.a'
    if a.prepare:
        assert a.base
        old_cache = object_bytes(a.base, 'cache.o')
        ids = set(re.findall(rb'(?<![0-9a-f])[0-9a-f]{64}(?![0-9a-f])', old_cache)); assert len(ids) == 1
        assert b'srv4.full' in a.prepare.read_bytes(), 'Missing compiled vector path'
        old_id = next(iter(ids)).decode()
        identity = hashlib.sha256(('madeira-srv-vector-v1\0'+old_id+'\0'+sha(a.prepare)).encode()).hexdigest()
        folder.mkdir(parents=True, exist_ok=True)
        (folder/'madeira_shader_compiler_identity.h').write_text(
            '#pragma once\n#define MADEIRA_SHADER_COMPILER_ID "'+identity+'"\n', encoding='utf-8')
        (folder/'prepared.json').write_text(json.dumps({'oldCompilerID':old_id, 'compilerID':identity,
            'converterObjectSHA256':sha(a.prepare), 'oldCacheObjectSHA256':hashlib.sha256(old_cache).hexdigest()}), encoding='utf-8')
        print('PASS: new AIR cache namespace derives from the actual replacement emitter and previous compiler')
        return
    spec = importlib.util.spec_from_file_location('pacing', root/'tools/dxmt-pacing-bundle.py')
    mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
    members = mod.members(archive)
    record = {'format':'madeira-srv-vector-v1', 'archive_sha256':sha(archive), 'members':members,
              'sources':{s:hashlib.sha256((root/s).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for s in SOURCES}}
    if a.stamp:
        assert a.base
        old = mod.members(a.base); assert len(old) == len(members)
        changed = sorted(new['name'] for before,new in zip(old,members) if before != new)
        assert changed == ['cache.o','dxbc_converter_base.o'], changed
        prepared = json.loads((folder/'prepared.json').read_text(encoding='utf-8'))
        assert hashlib.sha256(object_bytes(archive,'dxbc_converter_base.o')).hexdigest() == prepared['converterObjectSHA256']
        assert prepared['compilerID'].encode() in object_bytes(archive,'cache.o')
        record.update(prepared)
        record.update({'producer_commit':subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip(),
            'base_archive_sha256':sha(a.base),'changed_objects':changed,'preserved_members':len(old)-2})
        for name in ('producer-pacing.json','gpu-pipeline.json'):
            path = root/'build/dxmt-ios'/name; previous = json.loads(path.read_text(encoding='utf-8'))
            assert previous['archive_sha256'] == sha(a.base) and previous['members'] == old
            previous['shader_emitter_refresh'] = {'commit':record['producer_commit'], 'receipt':receipt.name,
                'base_archive_sha256':sha(a.base),'objects':changed}
            previous['archive_sha256'] = record['archive_sha256']; previous['members'] = members
            path.write_text(json.dumps(previous,indent=2)+'\n',encoding='utf-8')
        receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    else:
        expected = json.loads(receipt.read_text(encoding='utf-8'))
        for k,v in record.items(): assert expected[k] == v, ('stale shader refresh',k)
        assert expected['compilerID'].encode() in object_bytes(archive,'cache.o')
    print('PASS: actual shader emitter/cache and preserved native archive match source provenance')

if __name__ == '__main__': main()
