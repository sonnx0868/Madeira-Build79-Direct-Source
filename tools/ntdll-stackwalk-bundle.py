#!/usr/bin/env python3
"""Verify the compiled ARM64EC walker calls the registered alias translator."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import struct
import subprocess

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,required=True)
    parser.add_argument('--stamp',action='store_true')
    parser.add_argument('--base',type=Path)
    args=parser.parse_args();root=args.root.resolve()
    def module(name):
        spec=importlib.util.spec_from_file_location(name,root/('tools/'+name+'.py'))
        result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
    abi=module('dxmt-swapchain-bundle');compat=module('check-compatibility-binaries')
    path=root/'app/Madeira/arm64ec-windows/ntdll.dll';data=path.read_bytes();pe=abi.PE(data)
    syms=abi.symbols(pe)
    start=syms['#RtlWalkFrameChain'];target=syms['#xlate_ios_jit_rev']
    end=min(value for value in syms.values() if value>start)
    assert 64<=end-start<=4096,('unexpected frame walker bounds',start,end)
    calls=[]
    for at in range(start,end,4):
        word=struct.unpack('<I',pe.read(pe.offset(at,4),4))[0]
        if word & 0xfc000000 == 0x94000000:
            delta=word & 0x3ffffff
            if delta & 0x2000000:delta-=0x4000000
            if at+delta*4==target:calls.append(at)
    assert len(calls)>=2,('compiled frame walker lacks alias lookup and returned-frame normalization',calls)
    sha=lambda blob:hashlib.sha256(blob).hexdigest()
    record={'format':'madeira-stackwalk-alias-v1','sha256':sha(data),'alias_call_rvas':calls,
            'sources':{name:sha((root/name).read_bytes().replace(b'\r\n',b'\n')) for name in
                       ('wine/dlls/ntdll/signal_arm64ec.c','patches/wine-stackwalk-jit-alias.patch')}}
    receipt=root/'build/wine-pe/stackwalk-alias.json'
    if args.stamp:
        record['producer_commit']=subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD'],text=True).strip()
        if args.base:
            old=args.base.read_bytes();record['base_sha256']=sha(old)
            assert compat.exports(abi.PE(old))==compat.exports(pe),'changed ntdll export contract'
            parent_path=root/'build/wine-pe/module-loader.json'
            parent=json.loads(parent_path.read_text(encoding='utf-8'))
            key='app/Madeira/arm64ec-windows/ntdll.dll'
            assert parent['binaries'][key]==record['base_sha256'],'stale parent PE loader'
            for name,digest in parent['binaries'].items():
                if name!=key:assert sha((root/name).read_bytes())==digest,('unrelated loader changed',name)
            for name,digest in parent['inputs'].items():
                assert sha((root/name).read_bytes())==digest,('loader source changed',name)
            parent['binaries'][key]=record['sha256']
            parent['stackwalk_refresh']={'commit':record['producer_commit'],'receipt':receipt.name,
                                        'base_arm64ec_ntdll_sha256':record['base_sha256']}
            parent_path.write_text(json.dumps(parent,indent=2)+'\n',encoding='utf-8')
        receipt.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    else:
        expected=json.loads(receipt.read_text(encoding='utf-8'))
        for key,value in record.items():assert expected[key]==value,('stale compiled stack walker',key)
    print('PASS: actual ARM64EC frame walker calls alias translation for lookup and returned frames; source/PE identity match')

if __name__=='__main__':main()
