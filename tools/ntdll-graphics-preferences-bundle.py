#!/usr/bin/env python3
"""Bind the opt-in, version-checked graphics preference adapter to registry.o."""
from pathlib import Path
import argparse,hashlib,importlib.util,json,subprocess
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--root',type=Path,required=True);p.add_argument('--archive',type=Path)
p.add_argument('--stamp',action='store_true');p.add_argument('--base',type=Path)
a=p.parse_args();root=a.root.resolve();archive=a.archive or root/'app/Madeira/libntdll_unix.a'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def source_sha(path):return hashlib.sha256(path.read_bytes().replace(b'\r\n',b'\n')).hexdigest()
spec=importlib.util.spec_from_file_location('sync_bundle',root/'tools/nutssync-bundle.py')
sync=importlib.util.module_from_spec(spec);spec.loader.exec_module(sync)
members=sync.members(root,archive)
assert b'game-effects-v1'in archive.read_bytes(),'graphics preference producer absent'
sources=('build/ntdll-unix/registry_ios.c','build/ntdll-unix/ios_graphics_preferences.h','build/ntdll-unix/ios_graphics_enumeration.h',
         'wine/dlls/ntdll/unix/registry.c','build/game-profiles/liarsbar-hdrp-v1.json')
record={'format':'game-graphics-preferences-v1','archive_sha256':sha(archive),
        'registry_object_sha256':members['registry.o'],'sources':{s:source_sha(root/s)for s in sources}}
receipt=root/'build/ntdll-unix/game-graphics-preferences.json'
if a.stamp:
    record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
    if a.base:
        before=sync.members(root,a.base);assert members.keys()==before.keys()
        changed=sorted(k for k in members if members[k]!=before[k]);assert changed==['registry.o'],changed
        record.update(base_archive_sha256=sha(a.base),changed_members=changed,preserved_members=len(members)-1)
        updates=[]
        for name in ('lrcpc2-alias.json','runtime-scheduling-loader.json'):
            path=root/'build/ntdll-unix'/name;parent=json.loads(path.read_text())
            assert parent['archive_sha256']==sha(a.base),('stale parent',name)
            if name=='lrcpc2-alias.json':assert parent['object_sha256']==members['signal_arm64.o']
            else:
                for obj,digest in parent['objects'].items():assert members[obj]==digest,('runtime object changed',obj)
                for source,digest in parent['inputs'].items():assert source_sha(root/source)==digest,('runtime source changed',source)
            parent['archive_sha256']=record['archive_sha256'];parent['graphics_preferences_refresh']=record['producer_commit']
            updates.append((path,parent))
        path=root/'build/madsync/native-receipt.json';parent=json.loads(path.read_text())
        assert parent['archives']['ntdll']['sha256']==sha(a.base)and parent['archives']['ntdll']['members']==before
        server=root/'app/Madeira/libwineserver.a'
        assert parent['archives']['server']['sha256']==sha(server)and parent['archives']['server']['members']==sync.members(root,server)
        for source,digest in parent['sources'].items():
            current=source_sha(root/source)
            if current!=digest:
                assert source=='build/ntdll-unix/build.sh',('unrelated sync source changed',source)
                parent['sources'][source]=current
        parent['archives']['ntdll']['sha256']=record['archive_sha256'];parent['archives']['ntdll']['members']=members
        parent['graphics_preferences_refresh']=record['producer_commit'];updates.append((path,parent))
        for path,parent in updates:path.write_text(json.dumps(parent,indent=2)+'\n')
    receipt.write_text(json.dumps(record,indent=2)+'\n')
else:
    expected=json.loads(receipt.read_text())
    for key,value in record.items():assert expected[key]==value,('stale graphics preference runtime',key)
print('PASS: version-scoped graphics preference source, registry object and preserved native runtime match')
