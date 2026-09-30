"""Execute the final DLL's six TEB lookup expansions in an ARM64 emulator."""
from pathlib import Path
import struct,subprocess,re,json,hashlib
from unicorn import Uc,UC_ARCH_ARM64,UC_MODE_ARM,UC_HOOK_MEM_READ
from unicorn import arm64_const as ar
H=Path(__file__).resolve().parent
P=H/'build/Bin/libarm64ecfex.dll'
def load(path):
 d=path.read_bytes();p=struct.unpack_from('<I',d,60)[0];op=p+24
 base=struct.unpack_from('<Q',d,op+24)[0];size=struct.unpack_from('<I',d,op+56)[0]
 u=Uc(UC_ARCH_ARM64,UC_MODE_ARM);u.mem_map(base,(size+4095)&~4095)
 for n in range(struct.unpack_from('<H',d,p+6)[0]):
  s=op+struct.unpack_from('<H',d,p+20)[0]+n*40
  vs,rva,rs,raw=struct.unpack_from('<IIII',d,s+8)
  if rs:u.mem_write(base+rva,d[raw:raw+rs])
 nm=subprocess.check_output(['llvm-nm','--defined-only',str(path)],text=True)
 symbols={n:int(v,16) for v,n in re.findall(r'^([0-9a-f]+) \S (\w+)$',nm,re.M)}
 return u,symbols
u,symbols=load(P);a=symbols['DispatchJump'];b=symbols['JumpSetStack']+8
raw=bytes(u.mem_read(a,b-a));words=struct.unpack('<'+'I'*(len(raw)//4),raw)
macros=[]
for i,w in enumerate(words):
 if w&0xffffffe0!=0xd53bd060:continue
 reg=w&31;start=i-3
 assert start>=0 and words[i+1]==(0x927df000|(reg<<5)|reg)
 scratch=words[start]&31
 assert words[start]&0x9f000000==0x90000000
 assert (words[start+1]&31)==scratch and ((words[start+1]>>5)&31)==scratch
 assert words[i+4]==(0xaa0003e0|(18<<16)|reg)
 macros.append((a+4*start,reg,scratch))
assert [(r,s) for _,r,s in macros]==[(16,17),(16,23),(17,16),(17,16),(17,16),(17,16)]
TSD=0x20000000;u.mem_map(TSD,0x4000);offset_address=symbols['IosTebTsdOffset']
reads=[]
u.hook_add(UC_HOOK_MEM_READ,lambda uc,access,address,size,value,extra:reads.append((address,size)))
regs=[getattr(ar,'UC_ARM64_REG_X'+str(i)) for i in range(31)]
initial=[0x1122334400000000+i*0x101 for i in range(31)];FALLBACK=initial[18];TEB=0x71fff20000
cases=0
for start,dest,scratch in macros:
 for offset in [0,0x898,0x900,0xb18,0x1ff8]:
  for value in [0,TEB]:
   for tag in range(8):
    for flags in range(16):
     for reg,val in zip(regs,initial):u.reg_write(reg,val)
     u.reg_write(ar.UC_ARM64_REG_SP,0) # No stack access may be needed by this macro.
     u.reg_write(ar.UC_ARM64_REG_TPIDRRO_EL0,TSD|tag);u.reg_write(ar.UC_ARM64_REG_NZCV,flags<<28)
     u.mem_write(offset_address,struct.pack('<I',offset));u.mem_write(TSD+offset,struct.pack('<Q',value));reads.clear()
     u.emu_start(start,start+32,count=16)
     expected=value if offset and value else FALLBACK
     assert u.reg_read(regs[dest])==expected,(hex(start),offset,value,tag)
     assert u.reg_read(ar.UC_ARM64_REG_PC)==start+32
     assert u.reg_read(ar.UC_ARM64_REG_SP)==0 and u.reg_read(ar.UC_ARM64_REG_NZCV)==flags<<28
     for i,reg in enumerate(regs):
      if i not in (dest,scratch):assert u.reg_read(reg)==initial[i],(i,hex(start))
     assert reads==([(offset_address,4),(TSD+offset,8)] if offset else [(offset_address,4)]),reads
     cases+=1
# Execute the old shipped fixed-offset expansion against this iPad's measured
# offset. This negative control must read the wrong slot and return zero.
s,sym=load(H.parent/'fex-windows-baseline/shipped-xtajit64.dll');s.mem_map(TSD,0x4000)
s.mem_write(TSD+0x900,struct.pack('<Q',TEB));s.reg_write(ar.UC_ARM64_REG_TPIDRRO_EL0,TSD)
start=sym['DispatchJump']+12;s.emu_start(start,start+12,count=4)
assert s.reg_read(ar.UC_ARM64_REG_X16)==0
report={'status':'PASS','artifact_sha256':hashlib.sha256(P.read_bytes()).hexdigest(),'executed_cases':cases,'final_image_macros':len(macros),'scratch_registers':[(d,t) for _,d,t in macros],'offsets_tested':[0,0x898,0x900,0xb18,0x1ff8],'all_NZCV_values_preserved':True,'all_other_registers_preserved':True,'no_stack_access':True,'zero_offset_never_reads_slot_zero':True,'zero_TSD_entry_uses_existing_x18_fallback':True,'old_fixed_offset_negative_control_reproduced':True,'scope':'Actual final DLL ARM64 instructions executed in Unicorn 2.1.4; not an iPad runtime test.'}
(H/'assembly-test-report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
