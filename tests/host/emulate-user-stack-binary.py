#!/usr/bin/env python3
"""Execute the actual packaged i386 RtlCreateUserStack; mock only NT/diagnostic calls.

Requires pefile and Unicorn. This tests real PE instructions and the WoW32
ULONG transport, not complete Chrome or an iPad process.
"""
from pathlib import Path
import argparse
import collections
import hashlib
import json
import struct
import sys

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--dll',type=Path,required=True)
parser.add_argument('--expect',choices=['host','legacy'],required=True)
parser.add_argument('--out',type=Path,required=True)
parser.add_argument('--unicorn-dir',type=Path)
args=parser.parse_args()
if args.unicorn_dir:sys.path.insert(0,str(args.unicorn_dir))
import pefile
from unicorn import Uc,UC_ARCH_X86,UC_MODE_32,UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_EAX,UC_X86_REG_EIP,UC_X86_REG_ESP

pe=pefile.PE(str(args.dll));assert pe.FILE_HEADER.Machine==0x14c
base=pe.OPTIONAL_HEADER.ImageBase
exports={x.name.decode():base+x.address for x in pe.DIRECTORY_ENTRY_EXPORT.symbols if x.name and not x.forwarder}
entry=exports['RtlCreateUserStack']
apis={'NtQuerySystemInformation':4,'NtSetInformationProcess':4,'NtAllocateVirtualMemory':6,'NtFreeVirtualMemory':4}
hooks={exports[n]:(n,count) for n,count in apis.items()}
hooks.update({address:(name,0) for name,address in exports.items() if name.startswith('__wine_dbg_')})


def run(unit,fail=None,query_status=0,defaults=False,alignment=65536):
    uc=Uc(UC_ARCH_X86,UC_MODE_32)
    uc.mem_map(base,(pe.OPTIONAL_HEADER.SizeOfImage+4095)&~4095)
    uc.mem_write(base,pe.get_memory_mapped_image())
    for addr,size in [(0,4096),(0x100000,65536),(0x200000,4096),(0x210000,4096),
                      (0x220000,4096),(0x230000,4096),(0x300000,4096)]:uc.mem_map(addr,size)
    put=lambda address,value:uc.mem_write(address,struct.pack('<I',value&0xffffffff))
    get=lambda address:struct.unpack('<I',bytes(uc.mem_read(address,4)))[0]
    # Model the normal FS TEB/PEB reads used by default stack sizes.
    put(0x18,0x210000);put(0x30,0x220000);put(0x210030,0x220000);put(0x220008,0x230000)
    uc.mem_write(0x230000,b'MZ');put(0x23003c,0x80);uc.mem_write(0x230080,b'PE\0\0')
    uc.mem_write(0x230098,struct.pack('<H',0x10b));put(0x230098+72,0x400000);put(0x230098+76,4096)
    out=0x200000;uc.mem_write(out,b'\xa5'*20)
    stop=0x300000;sp=0x10f000
    values=[stop,0 if defaults else 4096,0 if defaults else 0x400000,0,4096,alignment,out]
    uc.mem_write(sp,struct.pack('<7I',*values));uc.reg_write(UC_X86_REG_ESP,sp)
    state={'query':[],'reserve':None,'commits':[],'frees':0}
    recent=collections.deque(maxlen=12)

    def callback(uc,address,size,unused):
        recent.append(address)
        if address not in hooks:return
        name,count=hooks[address];esp=uc.reg_read(UC_X86_REG_ESP);ret=get(esp)
        parameters=[get(esp+4+i*4) for i in range(count)]
        result=0
        if name=='NtQuerySystemInformation':
            cls,ptr,length,retlen=parameters
            assert cls==1001 and length==4 and ptr,parameters
            state['query'].append({'class':cls,'length':length})
            result=query_status
            if not result:
                put(ptr,unit)
                if retlen:put(retlen,4)
        elif name=='NtSetInformationProcess':
            handle,cls,info,length=parameters
            assert handle==0xffffffff and cls==41 and length==12,parameters
            state['reserve']=get(info)
            if fail=='reserve':result=0xc0000017
            else:put(info+8,0x1680000)
        elif name=='NtAllocateVirtualMemory':
            handle,ptr,zero,size_ptr,kind,protection=parameters
            assert handle==0xffffffff and not zero and kind==4096,parameters
            record={'address':get(ptr),'bytes':get(size_ptr),'protection':protection}
            state['commits'].append(record)
            if fail==('guard' if len(state['commits'])==1 else 'data'):result=0xc0000017
        elif name=='NtFreeVirtualMemory':
            handle,ptr,size_ptr,kind=parameters
            assert handle==0xffffffff and get(ptr)==0x1680000 and get(size_ptr)==0 and kind==32768,parameters
            state['frees']+=1
        # Debug functions are cdecl (leave their arguments for the caller).
        uc.reg_write(UC_X86_REG_EAX,result)
        uc.reg_write(UC_X86_REG_ESP,esp+4+count*4)
        uc.reg_write(UC_X86_REG_EIP,ret)

    uc.hook_add(UC_HOOK_CODE,callback)
    try:uc.emu_start(entry,stop,count=200000)
    except Exception as exc:
        raise AssertionError({'error':str(exc),'pc':hex(uc.reg_read(UC_X86_REG_EIP)),
                              'recent':[hex(x) for x in recent],'state':state}) from exc
    assert uc.reg_read(UC_X86_REG_EIP)==stop,'instruction limit reached'
    assert uc.reg_read(UC_X86_REG_ESP)==sp+28,'RtlCreateUserStack stdcall ABI'
    status=uc.reg_read(UC_X86_REG_EAX)
    output=bytes(uc.mem_read(out,20))
    if fail:
        assert status==0xc0000017 and output==b'\xa5'*20,state
        assert state['frees']==(fail!='reserve'),state
    else:
        assert status==0 and state['frees']==0,state
        guard=4096 if args.expect=='legacy' or query_status or unit<4096 or unit>65536 or unit&(unit-1) else unit
        oldbase,oldlimit,stackbase,stacklimit,deallocation=struct.unpack('<5I',output)
        assert not oldbase and not oldlimit and deallocation==0x1680000
        assert stackbase==deallocation+state['reserve'] and stacklimit==deallocation+2*guard,state
        assert state['commits']==[
            {'address':deallocation+guard,'bytes':guard,'protection':260},
            {'address':deallocation+2*guard,'bytes':state['reserve']-2*guard,'protection':4}],state
        assert stackbase-stacklimit>=0x400000-8192
        assert not(state['reserve']&(alignment-1))
    assert len(state['query'])==(args.expect=='host'),state
    return {'hostUnit':unit,'failure':fail,'queryStatus':query_status,'defaults':defaults,
            'alignment':alignment,'status':hex(status),'calls':state,'stack':output.hex()}


cases=[]
if args.expect=='legacy':
    cases.append(run(16384))
    cases.append(run(16384,defaults=True))
    cases.append(run(16384,alignment=0x100000))
    print('REPRODUCED: actual932 i386 DLL commits4KB guard at+1000 then data at+2000; native16KB union blocks legal+3ff4')
else:
    for unit in (4096,16384,65536):
        cases.append(run(unit));cases.append(run(unit,defaults=True))
    for failure in ('reserve','guard','data'):cases.append(run(16384,fail=failure))
    cases.append(run(16384,query_status=0xc0000003))
    cases.append(run(12345));cases.append(run(16384,alignment=0x100000))
    print('PASS: actual i386 DLL executes host-page guard query/allocation, defaults, reserve alignment, all failure cleanup and legacy fallback')
record={'status':'PASS','expectation':args.expect,'dllSHA256':hashlib.sha256(args.dll.read_bytes()).hexdigest(),
        'function':'RtlCreateUserStack','exportRVA':hex(entry-base),'cases':cases,
        'boundary':'actual packaged i386 instructions; NT allocation, query and debug functions controlled',
        'physicalChromeExecutionClaimed':False}
args.out.parent.mkdir(parents=True,exist_ok=True);args.out.write_text(json.dumps(record,indent=2)+'\n')
