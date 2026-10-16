#!/usr/bin/env python3
"""Bind the two native pipeline objects and the EC color-store implementation."""
from pathlib import Path
import argparse, hashlib, importlib.util, json, subprocess

SOURCES=('build/dxmt-ios/pipeline_cache_ios.m','build/dxmt-ios/pipeline_cache_ios.h',
         'build/dxmt-ios/pipeline_warmup_ios.m','build/dxmt-ios/pipeline_warmup_ios.h',
         'build/dxmt-ios/color_store_policy.hpp','patches/dxmt-dead-color-store.patch',
         'dxmt/src/dxmt/dxmt_context.cpp')
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,required=True);p.add_argument('--stamp',action='store_true')
    p.add_argument('--base',type=Path);p.add_argument('--base-dll',type=Path)
    a=p.parse_args();r=a.root.resolve()
    spec=importlib.util.spec_from_file_location('dxmt_pacing',r/'tools/dxmt-pacing-bundle.py')
    pacing=importlib.util.module_from_spec(spec);spec.loader.exec_module(pacing)
    archive=r/'app/Madeira/libdxmt_combined.a';dll=r/'app/Madeira/arm64ec-windows/d3d11.dll'
    data=archive.read_bytes();assert b'live-pipeline-v2' in data and b'learned-pipeline-replay-v1' in data
    assert b'color-store-v2' in dll.read_bytes(),'missing compiled EC full-clear policy'
    record={'archive_sha256':sha(archive),'members':pacing.members(archive),'d3d11_sha256':sha(dll),
            'sources':{s:hashlib.sha256((r/s).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for s in SOURCES}}
    receipt=r/'build/dxmt-ios/gpu-pipeline.json'
    if a.stamp:
        record['producer_commit']=subprocess.check_output(['git','-C',str(r),'rev-parse','HEAD'],text=True).strip()
        if a.base:
            old=pacing.members(a.base);assert len(old)==len(record['members'])
            changed=[new['name'] for prev,new in zip(old,record['members']) if prev!=new]
            assert sorted(changed)==['pipeline_cache_ios.o','pipeline_warmup_ios.o'],('unrelated archive changes',changed)
            record['base_archive_sha256']=sha(a.base);record['preserved_members']=len(old)-2
            # Preserve the original winemetal producer and its object identity;
            # record the container refresh without relabelling old code as new.
            path=r/'build/dxmt-ios/producer-pacing.json'
            previous=json.loads(path.read_text(encoding='utf-8'))
            assert previous['archive_sha256']==sha(a.base) and previous['members']==old
            previous['pipeline_refresh']={'commit':record['producer_commit'],'parent_archive_sha256':sha(a.base)}
            previous['archive_sha256']=record['archive_sha256'];previous['members']=record['members']
            path.write_text(json.dumps(previous,indent=2)+'\n',encoding='utf-8')
        if a.base_dll:
            spec=importlib.util.spec_from_file_location('compatibility',r/'tools/check-compatibility-binaries.py')
            compat=importlib.util.module_from_spec(spec);spec.loader.exec_module(compat)
            pe=compat.load_pe(r)
            assert compat.exports(pe(a.base_dll.read_bytes()))==compat.exports(pe(dll.read_bytes())),'changed D3D11 export contract'
            record['base_d3d11_sha256']=sha(a.base_dll)
        receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    else:
        expected=json.loads(receipt.read_text(encoding='utf-8'))
        for key,value in record.items():assert expected[key]==value,('stale GPU pipeline provenance',key)
    print('PASS: GPU pipeline source, both native objects, complete duplicate-name archive and EC color-store PE match')
if __name__=='__main__':main()
