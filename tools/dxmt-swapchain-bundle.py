#!/usr/bin/env python3
"""Verify compiled swapchain vtables expose hookable x64 entries, not ARM64 bodies."""
from pathlib import Path
import argparse, hashlib, importlib.util, json, re, struct, subprocess

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('swapchain_pe',ROOT/'tools/cpu-aot-survey.py')
pe_module=importlib.util.module_from_spec(spec);spec.loader.exec_module(pe_module)
PE=pe_module.PE
FFS=bytes.fromhex('488bc448895820555de9')
SOURCES=('build/dxmt-ios/swapchain_hook_abi.h','patches/dxmt-swapchain-hook-abi.patch',
         'dxmt/src/d3d11/d3d11_swapchain.cpp')
LAYOUT={'Present':8,'ResizeBuffers':13,'Present1':22,'ResizeBuffers1':39}
def digest(data):return hashlib.sha256(data).hexdigest()
def symbols(pe):
    at=pe.u32(60);table,count=struct.unpack('<II',pe.read(at+12,8))
    assert table and 0<count<=1000000,'missing/bad COFF symbols for build verification'
    pe.read(table,count*18);strings=table+count*18;length=pe.u32(strings)
    assert 4<=length<=16*1024*1024,'unbounded COFF names';pe.read(strings,length)
    result={};i=0
    while i<count:
        raw=pe.read(table+i*18,18);name=raw[:8]
        if name[:4]==b'\0'*4:
            offset=struct.unpack('<I',name[4:])[0];assert 4<=offset<length
            end=pe.data.find(b'\0',strings+offset,min(strings+length,strings+offset+8192));assert end>=0
            name=pe.data[strings+offset:end]
        else:name=name.split(b'\0',1)[0]
        value,section=struct.unpack_from('<Ih',raw,8)
        if 0<section<=len(pe.sections):result[name.decode('ascii')]=pe.sections[section-1]['rva']+value
        i+=1+raw[17]
    return result
def hybrid(pe):
    opt=pe.u32(60)+24;base=struct.unpack('<Q',pe.read(opt+24,8))[0]
    rva,size=pe.directory(10);assert size>=208,'missing EC load config'
    pointer=struct.unpack('<Q',pe.read(pe.offset(rva+200,8),8))[0];assert pointer>=base
    fields=struct.unpack('<14I',pe.read(pe.offset(pointer-base,56),56))
    assert fields[0] in (1,2) and 0<fields[2]<=16384 and 0<fields[13]<=100000
    ranges=list(struct.iter_unpack('<II',pe.read(pe.offset(fields[1],fields[2]*8),fields[2]*8)))
    redirect=dict(struct.iter_unpack('<II',pe.read(pe.offset(fields[4],fields[13]*8),fields[13]*8)))
    return base,ranges,redirect
def verify_entries(pe,prefixes,layout):
    syms=symbols(pe);base,ranges,redirect=hybrid(pe);entries=[]
    def kind(rva,n):
        return next((start&3 for start,length in ranges if (start&~3)<=rva and rva+n<=(start&~3)+length),None)
    for function_prefix,vtable_name in prefixes:
        vtable=syms[vtable_name]
        for method,slot in layout.items():
            candidates=[name for name in syms if name.startswith(function_prefix+str(len(method))+method+'E') and '$' not in name]
            assert len(candidates)==1,('missing/ambiguous swapchain method',method,candidates)
            name=candidates[0];entry=syms[name]
            assert kind(entry,16)==2,('COM entry is not x64',name)
            assert pe.read(pe.offset(entry,10),10)==FFS,('not a canonical x64 FFS',name)
            pointer=struct.unpack('<Q',pe.read(pe.offset(vtable+16+slot*8,8),8))[0]
            assert pointer==base+entry,('vtable bypasses the x64 entry',name)
            delta=struct.unpack('<i',pe.read(pe.offset(entry+10,4),4))[0];body=entry+14+delta
            assert body==syms['#'+name+'$hp_target'] and kind(body,4)==1,('missing native implementation',name)
            assert redirect.get(entry)==body,('wrong EC redirection',name)
            dispatch=syms['#'+name+'$hybpatch_thunk'];assert kind(dispatch,4)==1,'missing native hook dispatcher'
            entries.append({'method':method,'symbol':name,'x64_entry_rva':entry,'native_body_rva':body,'native_dispatch_rva':dispatch})
    return entries
def contract(data):
    pe=PE(data);syms=symbols(pe)
    prefixes=[]
    for b in ('0','1'):
        name='_ZTVN4dxmt17MTLD3D11SwapChainILb'+b+'EEE'
        if name in syms:prefixes.append(('_ZN4dxmt17MTLD3D11SwapChainILb'+b+'EE',name))
    assert prefixes,'no compiled swapchain vtable'
    return verify_entries(pe,prefixes,LAYOUT)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--root',type=Path,default=ROOT)
    p.add_argument('--stamp',action='store_true');p.add_argument('--base',type=Path);a=p.parse_args();r=a.root.resolve()
    dll=r/'app/Madeira/arm64ec-windows/d3d11.dll';data=dll.read_bytes()
    assert b'hookable-swapchain-v1' in data
    record={'sha256':digest(data),'entries':contract(data),'sources':{
        name:digest((r/name).read_bytes().replace(b'\r\n',b'\n')) for name in SOURCES}}
    path=r/'build/dxmt-pe/swapchain-hooks.json'
    if a.stamp:
        record['producer_commit']=subprocess.check_output(['git','-C',str(r),'rev-parse','HEAD'],text=True).strip()
        if a.base:
            record['base_dll_sha256']=digest(a.base.read_bytes())
            spec=importlib.util.spec_from_file_location('swapchain_compat',r/'tools/check-compatibility-binaries.py')
            compat=importlib.util.module_from_spec(spec);spec.loader.exec_module(compat)
            assert compat.exports(PE(a.base.read_bytes()))==compat.exports(PE(data)),'changed D3D11 export contract'
            gpu_path=r/'build/dxmt-ios/gpu-pipeline.json'
            if gpu_path.exists():
                # The native GPU objects retain their original producer. Only
                # the shared PE's identity and frontend provenance are updated.
                gpu=json.loads(gpu_path.read_text(encoding='utf-8'))
                assert gpu['d3d11_sha256']==record['base_dll_sha256']
                if gpu['archive_sha256']==digest((r/'app/Madeira/libdxmt_combined.a').read_bytes()):
                    gpu['d3d11_sha256']=record['sha256']
                    gpu['frontend_refresh']={'commit':record['producer_commit'],'base_dll_sha256':record['base_dll_sha256'],
                                             'receipt':'build/dxmt-pe/swapchain-hooks.json'}
                    gpu_path.write_text(json.dumps(gpu,indent=2)+'\n',encoding='utf-8')
                else:print('Native GPU archive changed; GPU producer must restamp its receipt before packaging')
        path.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    else:
        expected=json.loads(path.read_text(encoding='utf-8'))
        for key,value in record.items():assert expected[key]==value,('stale swapchain ABI receipt',key)
    print('PASS: compiled swapchain vtables point to x64 FFS entries with native bodies, dispatch thunks and EC redirection')
if __name__=='__main__':main()
