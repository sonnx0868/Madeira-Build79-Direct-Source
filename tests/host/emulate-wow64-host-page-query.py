#!/usr/bin/env python3
"""Run the actual ARM64 WoW64 NtQuerySystemInformation thunk, not a source model."""
from pathlib import Path
import argparse,hashlib,json,re,struct,sys
p=argparse.ArgumentParser();p.add_argument('--dll',type=Path,required=True);p.add_argument('--expect',choices=['old','fixed'],required=True);p.add_argument('--out',type=Path,required=True);p.add_argument('--unicorn-dir',type=Path)
a=p.parse_args()
if a.unicorn_dir:sys.path.insert(0,str(a.unicorn_dir))
import pefile
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_CODE
from unicorn.arm64_const import UC_ARM64_REG_X0,UC_ARM64_REG_X1,UC_ARM64_REG_X2,UC_ARM64_REG_X3,UC_ARM64_REG_X30,UC_ARM64_REG_SP,UC_ARM64_REG_PC
root=Path(__file__).resolve().parents[2]
s=(root/'wine/dlls/ntdll/ntsyscalls.h').read_text().split('#define ALL_SYSCALLS32',1)[1]
rows=[]
for id,name,count in re.findall(r'SYSCALL_ENTRY(?:_\w+)?\(\s*(0x[0-9a-f]+),\s*(\w+),\s*(\d+)\s*\)',s):
 if rows and int(id,16)==0:break
 rows.append((int(id,16),name,int(count)))
assert [i for i,n,c in rows]==list(range(len(rows)))
slot=next(i for i,n,c in rows if n=='NtQuerySystemInformation')
pe=pefile.PE(str(a.dll));assert pe.FILE_HEADER.Machine==0xaa64
base=pe.OPTIONAL_HEADER.ImageBase;im=pe.get_memory_mapped_image();descriptors=[]
for rva in range(0,len(im)-32,8):
 table,zero,count,args=struct.unpack_from('<QQQQ',im,rva)
 if zero or count!=len(rows) or not(base<=table<base+len(im)) or not(base<=args<base+len(im)):continue
 if im[args-base:args-base+count]!=bytes(c for i,n,c in rows):continue
 descriptors.append((rva,struct.unpack_from('<Q',im,table-base+slot*8)[0]))
assert len(descriptors)==1,descriptors
entry=descriptors[0][1];output=[]
for cls,length,nullout,nullret in [(1001,4,False,False),(1001,8,False,False),(1001,4,True,False),(1001,4,False,True),(1000,4,False,False),(0xffffffff,4,False,False)]:
 uc=Uc(UC_ARCH_ARM64,UC_MODE_ARM);uc.mem_map(base,(len(im)+4095)&~4095);uc.mem_write(base,im)
 for addr,size in [(0x100000,0x10000),(0x200000,0x10000),(0x300000,0x10000),(0x400000,0x10000)]:uc.mem_map(addr,size)
 hooks={};index=0
 for m in pe.DIRECTORY_ENTRY_IMPORT:
  for imp in m.imports:
   dest=0x300000+16*index;index+=1;uc.mem_write(imp.address,struct.pack('<Q',dest));hooks[dest]=imp.name.decode() if imp.name else str(imp.ordinal)
 arg=0x100000;buf=0x101000;retlen=0x102000;stop=0x400000
 uc.mem_write(buf,struct.pack('<III',0x12345678,0xaaaaaaaa,0x87654321));uc.mem_write(retlen,struct.pack('<I',0xbbbbbbbb))
 uc.mem_write(arg,struct.pack('<4I',cls,0 if nullout else buf+4,length,0 if nullret else retlen))
 uc.reg_write(UC_ARM64_REG_X0,arg);uc.reg_write(UC_ARM64_REG_X30,stop);uc.reg_write(UC_ARM64_REG_SP,0x20f000)
 calls=[]
 def hook(u,address,size,unused):
  if address not in hooks:return
  name=hooks[address]
  if name=='NtQuerySystemInformation':
   c,ptr,n,rl=[u.reg_read(x) for x in [UC_ARM64_REG_X0,UC_ARM64_REG_X1,UC_ARM64_REG_X2,UC_ARM64_REG_X3]]
   assert (c,ptr,n,rl)==(cls,0 if nullout else buf+4,length,0 if nullret else retlen)
   calls.append((c,ptr,n,rl));status=0xc0000004 if n!=4 else 0xc0000005 if not ptr else 0
   if rl:u.mem_write(rl,struct.pack('<I',4))
   if not status:u.mem_write(ptr,struct.pack('<I',16384))
  elif name.startswith('__wine_dbg_') or name in ('_vsnprintf','DbgPrint','vDbgPrintEx','DbgPrintEx'):status=0
  else:raise AssertionError(('unexpected API',name,hex(address)))
  u.reg_write(UC_ARM64_REG_X0,status);u.reg_write(UC_ARM64_REG_PC,u.reg_read(UC_ARM64_REG_X30))
 uc.hook_add(UC_HOOK_CODE,hook);uc.emu_start(entry,stop,count=20000)
 assert uc.reg_read(UC_ARM64_REG_PC)==stop
 status=uc.reg_read(UC_ARM64_REG_X0)&0xffffffff
 forward=cls==1000 or cls==1001 and a.expect=='fixed'
 expected=0xc0000004 if length!=4 else 0xc0000005 if nullout else 0
 assert status==(expected if forward else 0xc0000003),(cls,hex(status),calls)
 assert len(calls)==int(forward),(cls,calls)
 pre,value,post=struct.unpack('<III',bytes(uc.mem_read(buf,12)))
 assert pre==0x12345678 and post==0x87654321
 assert value==(16384 if forward and not expected else 0xaaaaaaaa)
 output.append(dict(infoClass=cls,length=length,nullOutput=nullout,nullReturnLength=nullret,status=hex(status),forwarded=bool(calls)))
r=dict(status='PASS',expectation=a.expect,dllSHA256=hashlib.sha256(a.dll.read_bytes()).hexdigest(),entryRVA=hex(entry-base),syscallSlot=slot,serviceCount=len(rows),cases=output,scope='actual packaged ARM64 WoW64 thunk; native NT query and debug boundary controlled',physicalChromeVerified=False)
a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(r,indent=2)+'\n')
print(('REPRODUCED:934 WoW64 DLL rejects private host-page query; PE stack silently falls back to4KB' if a.expect=='old' else 'PASS: actual ARM64 WoW64 DLL forwards host-page query, exact4-byte results/errors and NULL pointers'))
