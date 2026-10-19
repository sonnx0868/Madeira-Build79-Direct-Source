#!/usr/bin/env python3
"""Bind the retirement fix to one native object and the exact Present PE."""
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
parser.add_argument('--base-dll',type=Path)
args=parser.parse_args();root=args.root.resolve()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def source_sha(path):return hashlib.sha256(path.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
spec=importlib.util.spec_from_file_location('pacing',root/'tools/dxmt-pacing-bundle.py')
pacing=importlib.util.module_from_spec(spec);spec.loader.exec_module(pacing)
archive=root/'app/Madeira/libdxmt_combined.a'
dll=root/'app/Madeira/arm64ec-windows/d3d11.dll'
assert b'[PRESENT_WAIT]' in dll.read_bytes(),'Present attribution was not rebuilt'
sources=('patches/dxmt-retirement-lifetime.patch','build/dxmt-ios/present_waits.hpp',
         'dxmt/src/dxmt/dxmt_command_queue.cpp','dxmt/src/d3d11/d3d11_swapchain.cpp')
record={'format':'gpu-retirement-v1','archive_sha256':sha(archive),'members':pacing.members(archive),
        'd3d11_sha256':sha(dll),'sources':{name:source_sha(root/name) for name in sources}}
receipt=root/'build/dxmt-ios/retirement.json'
if args.stamp:
    record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
    if args.base:
        assert args.base_dll,'base PE is required for an incremental native refresh'
        old=pacing.members(args.base);new=record['members']
        assert len(old)==len(new)
        changed=[b['name'] for a,b in zip(old,new) if a!=b]
        assert changed==['dxmt_command_queue.o'],('unrelated native changes',changed)
        record.update(base_archive_sha256=sha(args.base),base_d3d11_sha256=sha(args.base_dll),
                      changed_members=changed,preserved_members=len(old)-1)
        # Validate every parent before writing. Their original producers remain
        # recorded; only the enclosing archive and the rebuilt PE are rebound.
        # The sole changed parent source is Present, explicitly covered here.
        updates=[]
        for name in ('producer-pacing.json','shader-vector-load.json','shader-optimizer.json',
                     'frameflow.json','gpu-pipeline.json'):
            path=root/'build/dxmt-ios'/name
            parent=json.loads(path.read_text(encoding='utf-8'))
            assert parent['archive_sha256']==sha(args.base) and parent['members']==old,('stale parent',name)
            changed_sources={}
            for source,previous in parent.get('sources',{}).items():
                current=source_sha(root/source)
                if current!=previous:
                    assert name=='frameflow.json' and source=='dxmt/src/d3d11/d3d11_swapchain.cpp',('unrelated source',name,source)
                    changed_sources[source]={'before':previous,'after':current}
                    parent['sources'][source]=current
            if 'd3d11_sha256' in parent:
                assert parent['d3d11_sha256']==sha(args.base_dll),('stale frontend',name)
                parent['d3d11_sha256']=record['d3d11_sha256']
            parent['archive_sha256']=record['archive_sha256'];parent['members']=new
            parent['retirement_refresh']={'commit':record['producer_commit'],
                'base_archive_sha256':sha(args.base),'receipt':'build/dxmt-ios/retirement.json',
                'changed_sources':changed_sources}
            updates.append((path,parent))
        for path,parent in updates:path.write_text(json.dumps(parent,indent=2)+'\n',encoding='utf-8')
    receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
else:
    previous=json.loads(receipt.read_text(encoding='utf-8'))
    for key,value in record.items():assert previous[key]==value,('stale retirement runtime',key)
print('PASS: production retirement worker, Present wait sites and unchanged native object identities match')
