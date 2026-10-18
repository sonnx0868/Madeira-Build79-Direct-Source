#!/usr/bin/env python3
"""Verify an optimizer refresh, preserving the other native archive objects."""
from pathlib import Path
import argparse, hashlib, importlib.util, json, re, subprocess

SOURCES=('build/dxmt-ios/explicit_lod_guard.hpp','patches/dxmt-explicit-lod-guard.patch',
         'dxmt/src/airconv/airconv_context.cpp','build/dxmt-ios/refresh-optimizer.sh')
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def module(root,name):
    spec=importlib.util.spec_from_file_location(name,root/('tools/'+name+'.py'))
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,required=True);parser.add_argument('--base',type=Path)
    parser.add_argument('--prepare',type=Path);parser.add_argument('--stamp',action='store_true')
    args=parser.parse_args();root=args.root.resolve();folder=root/'build/dxmt-ios/optimizer-refresh'
    srv=module(root,'dxmt-srv-bundle');pacing=module(root,'dxmt-pacing-bundle')
    archive=root/'app/Madeira/libdxmt_combined.a';receipt=root/'build/dxmt-ios/shader-optimizer.json'
    if args.prepare:
        assert args.base
        cache=srv.object_bytes(args.base,'cache.o')
        identities=set(re.findall(rb'(?<![0-9a-f])[0-9a-f]{64}(?![0-9a-f])',cache));assert len(identities)==1
        old=next(iter(identities)).decode()
        replacement=args.prepare.read_bytes();assert b'SinkingPass' in replacement and b'LoopRotatePass' in replacement
        identity=hashlib.sha256(('madeira-explicit-lod-guard-v1\0'+old+'\0'+sha(args.prepare)).encode()).hexdigest()
        folder.mkdir(parents=True,exist_ok=True)
        (folder/'madeira_shader_compiler_identity.h').write_text('#pragma once\n#define MADEIRA_SHADER_COMPILER_ID "'+identity+'"\n')
        (folder/'prepared.json').write_text(json.dumps({'oldCompilerID':old,'compilerID':identity,'contextObjectSHA256':sha(args.prepare)}))
        print('PASS: AIR cache namespace follows actual optimizer object bytes');return
    current=pacing.members(archive)
    record={'format':'madeira-shader-optimizer-v1','archive_sha256':sha(archive),'members':current,
            'sources':{s:hashlib.sha256((root/s).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for s in SOURCES}}
    if args.stamp:
        assert args.base;previous=pacing.members(args.base);assert len(previous)==len(current)
        changed=sorted(new['name'] for old,new in zip(previous,current) if old!=new)
        assert changed==['airconv_context.o','cache.o'],changed
        prepared=json.loads((folder/'prepared.json').read_text())
        assert sha(folder/'airconv_context.o')==prepared['contextObjectSHA256']
        assert hashlib.sha256(srv.object_bytes(archive,'airconv_context.o')).hexdigest()==prepared['contextObjectSHA256']
        assert prepared['compilerID'].encode() in srv.object_bytes(archive,'cache.o')
        record.update(prepared);record.update({'producer_commit':subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip(),
            'base_archive_sha256':sha(args.base),'changed_objects':changed,'preserved_members':len(previous)-2})
        for name in ('producer-pacing.json','gpu-pipeline.json','shader-vector-load.json'):
            path=root/'build/dxmt-ios'/name
            if not path.exists():continue
            parent=json.loads(path.read_text());assert parent['archive_sha256']==sha(args.base) and parent['members']==previous
            parent['optimizer_refresh']={'commit':record['producer_commit'],'receipt':receipt.name,'objects':changed}
            parent['archive_sha256']=record['archive_sha256'];parent['members']=current
            if name=='shader-vector-load.json':parent['compilerID']=prepared['compilerID']
            path.write_text(json.dumps(parent,indent=2)+'\n')
        receipt.write_text(json.dumps(record,indent=2)+'\n')
    else:
        expected=json.loads(receipt.read_text())
        for key,value in record.items():assert expected[key]==value,('stale shader optimizer',key)
        assert expected['compilerID'].encode() in srv.object_bytes(archive,'cache.o')
    print('PASS: actual shader optimizer/cache and preserved native archive match source provenance')
if __name__=='__main__':main()
