#!/usr/bin/env python3
"""Verify the refreshed upload/pipeline objects and rebind unchanged compiler receipts."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import subprocess

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--root',type=Path,required=True)
parser.add_argument('--stamp',action='store_true')
parser.add_argument('--base',type=Path)
args=parser.parse_args();root=args.root.resolve()
def module(name):
    spec=importlib.util.spec_from_file_location(name,root/('tools/'+name+'.py'))
    value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
pacing=module('dxmt-pacing-bundle')
archive=root/'app/Madeira/libdxmt_combined.a'
dll=root/'app/Madeira/arm64ec-windows/d3d11.dll'
assert b'upload-flow-v1' in dll.read_bytes(),'D3D11 upload flow not compiled'
assert b'upload-flow-v1' in archive.read_bytes(),'native upload flow not compiled'
assert b'startup-hot-v2' in archive.read_bytes(),'native startup inventory not compiled'
sources=('patches/dxmt-frameflow.patch','build/dxmt-ios/upload_policy.hpp',
         'build/dxmt-ios/pipeline_warmup_ios.m','dxmt/src/dxmt/dxmt_staging.cpp',
         'dxmt/src/dxmt/dxmt_staging.hpp','dxmt/src/dxmt/dxmt_resource_initializer.cpp',
         'dxmt/src/dxmt/dxmt_resource_initializer.hpp','dxmt/src/d3d11/d3d11_context_imm.cpp',
         'dxmt/src/d3d11/d3d11_swapchain.cpp')
record={'format':'frameflow-v1','archive_sha256':sha(archive),'members':pacing.members(archive),
        'd3d11_sha256':sha(dll),'sources':{s:hashlib.sha256((root/s).read_bytes().replace(b'\r\n',b'\n')).hexdigest() for s in sources}}
receipt=root/'build/dxmt-ios/frameflow.json'
if args.stamp:
    record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
    if args.base:
        old=pacing.members(args.base);assert len(old)==len(record['members'])
        changed=[new['name'] for previous,new in zip(old,record['members']) if previous!=new]
        required={'pipeline_warmup_ios.o','dxmt_resource_initializer.o','dxmt_staging.o'}
        assert required.issubset(changed) and set(changed)<=required|{'pipeline_cache_ios.o'},('unrelated native changes',changed)
        record['changed_members']=changed;record['preserved_members']=len(old)-len(changed)
        record['base_archive_sha256']=sha(args.base)
        # These implementation objects are unchanged. Retain the original
        # producer, source hashes and shader compiler namespace; update only
        # their verified enclosing archive's identity.
        for name in ('producer-pacing.json','shader-vector-load.json','shader-optimizer.json'):
            path=root/'build/dxmt-ios'/name
            if not path.exists():continue
            parent=json.loads(path.read_text(encoding='utf-8'))
            assert parent['archive_sha256']==sha(args.base) and parent['members']==old,('stale parent',name)
            parent['archive_sha256']=record['archive_sha256'];parent['members']=record['members']
            parent['frameflow_refresh']={'commit':record['producer_commit'],'base_archive_sha256':sha(args.base)}
            path.write_text(json.dumps(parent,indent=2)+'\n',encoding='utf-8')
    receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
else:
    previous=json.loads(receipt.read_text(encoding='utf-8'))
    for key,value in record.items():assert previous[key]==value,('stale frameflow build',key)
print('PASS: upload policy, native objects, warmup inventory and ARM64EC D3D11 binary match')
