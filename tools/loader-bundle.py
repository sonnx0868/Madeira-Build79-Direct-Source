#!/usr/bin/env python3
"""Bind all PE loaders and the Dock host to their current integration sources."""
from pathlib import Path
import argparse, hashlib, json, struct, subprocess

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--root',type=Path,required=True);parser.add_argument('--stamp',action='store_true')
    args=parser.parse_args();root=args.root.resolve()
    sha=lambda p:hashlib.sha256((root/p).read_bytes()).hexdigest()
    inputs=['wine/dlls/ntdll/loader.c','patches/wine-module-path.patch',
            'patches/madeira-dock-external-launch.patch','build/madeira-dock/build.sh']
    binaries={}
    for folder,machine in [('i386-windows',0x14c),('arm64ec-windows',0xa641),('aarch64-windows',0xaa64)]:
        name='app/Madeira/'+folder+'/ntdll.dll';data=(root/name).read_bytes();off=struct.unpack_from('<I',data,60)[0]
        if data[:2]!=b'MZ' or struct.unpack_from('<H',data,off+4)[0]!=machine or b'module-path-v1' not in data:
            raise SystemExit('Missing current PE module recovery: '+name)
        binaries[name]=sha(name)
    name='app/Madeira/arm64ec-windows/dockhost.exe';data=(root/name).read_bytes();off=struct.unpack_from('<I',data,60)[0]
    if data[:2]!=b'MZ' or struct.unpack_from('<H',data,off+4)[0]!=0x8664 or b'external-process-started' not in data:
        raise SystemExit('Missing external-program Dock host')
    binaries[name]=sha(name)
    content={'inputs':{p:sha(p) for p in inputs},'binaries':binaries}
    receipt=root/'build/wine-pe/module-loader.json'
    if args.stamp:
        content['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
        receipt.write_text(json.dumps(content,indent=2)+'\n',encoding='utf-8')
    else:
        saved=json.loads(receipt.read_text(encoding='utf-8'))
        for key in content:
            if saved.get(key)!=content[key]:raise SystemExit('Stale PE loader/Dock bundle: '+key)
    print('PASS: i386/ARM64EC/ARM64 PE loaders and external-program host match integration sources')

if __name__=='__main__':main()
