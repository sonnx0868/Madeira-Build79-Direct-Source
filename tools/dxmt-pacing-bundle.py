#!/usr/bin/env python3
"""Bind producer pacing to the one refreshed object; preserve duplicate ar names."""
from pathlib import Path
import argparse, collections, hashlib, json, subprocess

def sha_file(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
    return h.hexdigest()

def members(path):
    result=[];long_names=b'';seen=collections.Counter()
    with path.open('rb') as f:
        assert f.read(8)==b'!<arch>\n','not a regular ar archive'
        while True:
            header=f.read(60)
            if not header:break
            assert len(header)==60 and header[-2:]==b'`\n','invalid ar member'
            name=header[:16].decode('ascii').strip();size=int(header[48:58]);data=f.read(size)
            assert len(data)==size,'truncated ar member'
            if size&1:assert len(f.read(1))==1
            if name.startswith('#1/'):
                n=int(name[3:]);assert n<=len(data)
                name=data[:n].rstrip(b'\0').decode('utf-8');data=data[n:]
            elif name=='//':long_names=data;continue
            elif name.startswith('/') and name[1:].isdigit():
                offset=int(name[1:]);assert offset<len(long_names)
                name=long_names[offset:].split(b'/\n',1)[0].decode('utf-8')
            else:name=name.rstrip('/')
            if name.startswith('__.SYMDEF') or name in ('','/','/SYM64'):continue
            assert name.endswith('.o'),('unexpected archive member',name)
            seen[name]+=1
            result.append({'name':name,'occurrence':seen[name],'sha256':hashlib.sha256(data).hexdigest()})
    assert sum(x['name']=='winemetal_unix.o' for x in result)==1
    return result

def main():
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--archive',type=Path)
    p.add_argument('--stamp',action='store_true');p.add_argument('--base',type=Path);a=p.parse_args()
    root=a.root.resolve();archive=a.archive or root/'app/Madeira/libdxmt_combined.a'
    record={'archive_sha256':sha_file(archive),'members':members(archive),'sources':{}}
    for name in ['dxmt/src/winemetal/unix/winemetal_unix.c','build/dxmt-ios/present_pacing.h',
                 'patches/dxmt-producer-pacing.patch','patches/dxmt-gpu-foreground.patch','patches/dxmt-gpu-trace.patch',
                 'patches/dxmt-gpu-stages.patch','patches/dxmt-texture-headroom.patch',
                 'build/dxmt-ios/texture_headroom.h']:
        record['sources'][name]=hashlib.sha256((root/name).read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    receipt=root/'build/dxmt-ios/producer-pacing.json'
    if a.stamp:
        record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
        if a.base:
            previous=members(a.base)
            assert len(previous)==len(record['members'])
            changed=[new['name'] for old,new in zip(previous,record['members']) if old!=new]
            assert changed==['winemetal_unix.o'],('changed unrelated archive objects',changed)
            record['base_archive_sha256']=sha_file(a.base);record['preserved_members']=len(previous)-1
            # Validate every parent before writing any. These receipts bind the
            # whole container even though their own shader/cache objects did
            # not change. Preserve their producer and compiler namespace.
            parents=[]
            for name in ('gpu-pipeline.json','shader-vector-load.json','shader-optimizer.json'):
                path=root/'build/dxmt-ios'/name
                if not path.exists():continue
                parent=json.loads(path.read_text(encoding='utf-8'))
                assert parent['archive_sha256']==record['base_archive_sha256'] and parent['members']==previous,('stale native container receipt',name)
                parent['archive_sha256']=record['archive_sha256'];parent['members']=record['members']
                parent['boundary_refresh']={'commit':record['producer_commit'],'base_archive_sha256':record['base_archive_sha256'],
                                            'receipt':'build/dxmt-ios/producer-pacing.json'}
                parents.append((path,parent))
            for path,parent in parents:
                path.write_text(json.dumps(parent,indent=2)+'\n',encoding='utf-8')
        receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
        print('PASS: stamped producer pacing; '+str(record.get('preserved_members','all'))+' other DXMT/LLVM objects preserved')
    else:
        expected=json.loads(receipt.read_text(encoding='utf-8'))
        for key,value in record.items():assert expected[key]==value,('stale producer pacing receipt',key)
        print('PASS: producer pacing source, compiled object and complete DXMT/LLVM archive match')

if __name__=='__main__':main()
