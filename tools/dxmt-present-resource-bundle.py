#!/usr/bin/env python3
"""Bind native creation pools, Present locking and limited filtering to binaries."""
from pathlib import Path
import argparse,hashlib,importlib.util,json,subprocess
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--root',type=Path,required=True);p.add_argument('--stamp',action='store_true')
p.add_argument('--base',type=Path);p.add_argument('--base-dll',type=Path)
a=p.parse_args();root=a.root.resolve()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def source_sha(path):return hashlib.sha256(path.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
spec=importlib.util.spec_from_file_location('pacing',root/'tools/dxmt-pacing-bundle.py')
pacing=importlib.util.module_from_spec(spec);spec.loader.exec_module(pacing)
archive=root/'app/Madeira/libdxmt_combined.a';dll=root/'app/Madeira/arm64ec-windows/d3d11.dll'
assert b'MADEIRA_LIGHT_EFFECTS' in dll.read_bytes(),'limited filtering frontend missing'
sources=('patches/dxmt-present-resource-lifetime.patch','patches/dxmt-light-graphics.patch',
 'build/dxmt-ios/light_graphics.h','dxmt/src/winemetal/unix/winemetal_unix.c',
 'dxmt/src/d3d11/d3d11_swapchain.cpp','dxmt/src/d3d11/d3d11_state_object.cpp',
 'dxmt/src/d3d11/d3d11_texture_device.cpp')
record={'format':'present-resource-light-v1','archive_sha256':sha(archive),'members':pacing.members(archive),
 'd3d11_sha256':sha(dll),'sources':{name:source_sha(root/name)for name in sources}}
receipt=root/'build/dxmt-ios/present-resource.json'
if a.stamp:
 record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
 if a.base:
  assert a.base_dll,'incremental refresh needs the original frontend'
  before=pacing.members(a.base);after=record['members'];assert len(before)==len(after)
  changed=[new['name']for old,new in zip(before,after)if old!=new]
  assert changed==['winemetal_unix.o'],('unrelated native change',changed)
  record.update(base_archive_sha256=sha(a.base),base_d3d11_sha256=sha(a.base_dll),changed_members=changed,preserved_members=len(before)-1)
  updates=[]
  allowed={'dxmt/src/winemetal/unix/winemetal_unix.c','dxmt/src/d3d11/d3d11_swapchain.cpp'}
  for name in ('producer-pacing.json','gpu-pipeline.json','shader-vector-load.json',
               'shader-optimizer.json','frameflow.json','retirement.json'):
   path=root/'build/dxmt-ios'/name;parent=json.loads(path.read_text(encoding='utf-8'))
   assert parent['archive_sha256']==sha(a.base)and parent['members']==before,('stale parent',name)
   changed_sources={}
   for source,previous in parent.get('sources',{}).items():
    current=source_sha(root/source)
    if previous!=current:
     assert source in allowed,('unrelated source change',name,source)
     changed_sources[source]={'before':previous,'after':current};parent['sources'][source]=current
   if 'd3d11_sha256'in parent:
    assert parent['d3d11_sha256']==sha(a.base_dll),('stale frontend',name)
    parent['d3d11_sha256']=record['d3d11_sha256']
   parent['archive_sha256']=record['archive_sha256'];parent['members']=after
   parent['present_resource_refresh']={'commit':record['producer_commit'],'receipt':'build/dxmt-ios/present-resource.json',
    'base_archive_sha256':sha(a.base),'changed_sources':changed_sources}
   updates.append((path,parent))
  for path,parent in updates:path.write_text(json.dumps(parent,indent=2)+'\n',encoding='utf-8')
 receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
else:
 previous=json.loads(receipt.read_text(encoding='utf-8'))
 for key,value in record.items():assert previous[key]==value,('stale Present/resource runtime',key)
print('PASS: native creation pools, Present wait ordering and original texture policy sources match their binaries')
