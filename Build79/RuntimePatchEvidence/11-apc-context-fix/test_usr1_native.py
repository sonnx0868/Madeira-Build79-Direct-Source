#!/usr/bin/env python3
"""Full exact usr1_handler plus exact native context conversions in Unicorn.

No native executable is written. The proposed single B instruction is applied
only to the emulator's copied code. Kernel/TLS/server APIs are explicit mocks.
"""
from pathlib import Path
import hashlib, importlib.util, json, random, struct
from unicorn import Uc, UcError, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE, UC_HOOK_MEM_INVALID, UC_PROT_READ, UC_PROT_EXEC
from unicorn import arm64_const as ar

H=Path(__file__).resolve().parent
BASE=H.parent/'apc-signal-fix/Madeira'
BASE_SHA='1ae878fbc6ed1f2ffcc9febda14ad55bbffdb5a6efbe8c35cd10db6822c67a7b'
spec=importlib.util.spec_from_file_location('usr1_filemap',H.parent/'guest-exit-fix/build.py')
fm=importlib.util.module_from_spec(spec);spec.loader.exec_module(fm)
START,END,PATCH,EPILOGUE=0x100100458,0x10010060c,0x100100520,0x1001005f4
SAVE,SAVE_END,RESTORE,RESTORE_END=0x10010885c,0x100108a30,0x100108c44,0x100108e0c
SYMS={'teb':0x100124994,'wait':0x100125890,'get':0x1000feb18,'set':0x1000fe91c,
      'memcpy':0x1009e1e00,'dprintf':0x1009e17f4,'tls':0x20009000}
TOTAL,LAST,IN_POOL,RX,SIZE,NOTE=0x101073ea8,0x101073eac,0x101073eb0,0x101073fb0,0x101073fb8,0x100cebee8
DESC_TRAMP,DESC_TEB=0x100c5dd68,0x100c5dd38
UC,MC,TEB,TRAMP_SLOT,TEB_SLOT,TRAMP=0x20000000,0x20001000,0x20002000,0x20004000,0x20004008,0x20006000
STACK,DONE=0x30000000,0x40000000
JIT_START,JIT_ADR,JIT_STOP=0x14ff46480,0x14ff46488,0x14ff46494
FRAME,CR_BASE,CR_SP=0x7cdc001140,0x7cdd001000,0x7cdd2f3c50
R=[getattr(ar,'UC_ARM64_REG_X'+str(i))for i in range(31)]
sha=lambda b:hashlib.sha256(b).hexdigest()

class Handler:
 def __init__(self,data,variant,c):
  self.c=c;self.variant=variant;self.events=[];self.set_context=None
  self.u=u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
  pages={v&~4095 for v in [START,END-1,SAVE,SAVE_END-1,RESTORE,RESTORE_END-1,*SYMS.values(),TOTAL,RX,SIZE,NOTE,DESC_TRAMP,DESC_TEB]}
  for p in sorted(pages):
   if not 0x20000000<=p<0x20010000:u.mem_map(p,4096)
  for p,size in [(0x20000000,0x10000),(STACK,0x10000),(DONE,4096)]:u.mem_map(p,size)
  for lo,hi in [(START,END),(SAVE,SAVE_END),(RESTORE,RESTORE_END)]:
   off=fm.file_offset(data,lo,hi-lo);u.mem_write(lo,data[off:off+hi-lo])
  if variant=='candidate':u.mem_write(PATCH,struct.pack('<I',0x14000000|((EPILOGUE-PATCH)//4)))
  rng=random.Random(c.get('seed',103))
  self.gprs=[rng.getrandbits(64)for _ in range(31)]
  self.gprs[17]=c.get('x17',CR_SP);self.gprs[18]=TEB;self.gprs[28]=FRAME
  self.pc=c.get('pc',JIT_ADR);self.sigsp=c.get('sp',0x70001000)
  self.cpsr=rng.getrandbits(32);self.fpcr=rng.getrandbits(32);self.fpsr=rng.getrandbits(32)
  self.vectors=bytes(rng.randrange(256)for _ in range(512))
  u.mem_write(MC,bytes([0xcc])*0x328);self.w64(UC+0x30,MC)
  for i,v in enumerate(self.gprs):self.w64(MC+0x10+i*8,v)
  self.w64(MC+0x108,self.sigsp);self.w64(MC+0x110,self.pc);self.w32(MC+0x118,self.cpsr)
  u.mem_write(MC+0x120,self.vectors);self.w32(MC+0x324,self.fpcr);self.w32(MC+0x320,self.fpsr)
  self.original_mc=bytes(u.mem_read(MC,0x328))
  self.w64(TEB+0x3a8,0x70002000);self.w64(TEB+0x378,0x70003000)
  self.w64(RX,c.get('rx',0x140000000));self.w64(SIZE,c.get('size',0x20000000))
  self.w32(TOTAL,7);self.w32(LAST,55);self.w32(IN_POOL,13);self.w32(NOTE,c.get('noteb_count',0))
  self.w64(DESC_TRAMP,SYMS['tls']);self.w64(DESC_TEB,SYMS['tls'])
  self.w64(TRAMP_SLOT,0 if c.get('null_tramp')else TRAMP)
  self.w64(TEB_SLOT,0 if c.get('null_signal_teb')else TEB)
  self.regs=[0x1122334455000000+i for i in range(31)];self.regs[0]=30;self.regs[1]=0;self.regs[2]=UC;self.regs[30]=DONE
  for r,v in zip(R,self.regs):u.reg_write(r,v)
  self.sp=STACK+0x9000;u.reg_write(ar.UC_ARM64_REG_SP,self.sp)
  for name,addr in SYMS.items():u.hook_add(UC_HOOK_CODE,self.hook,begin=addr,end=addr,user_data=name)
  self.inside=0x70002000<=self.sigsp<=0x70003000
  self.expected_mc=bytearray(self.original_mc)
 def w64(self,a,v):self.u.mem_write(a,struct.pack('<Q',v&((1<<64)-1)))
 def w32(self,a,v):self.u.mem_write(a,struct.pack('<I',v&0xffffffff))
 def r64(self,a):return struct.unpack('<Q',self.u.mem_read(a,8))[0]
 def r32(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
 def make_context(self,flags):
  d=bytearray(0x390);struct.pack_into('<II',d,0,flags,self.cpsr)
  for i,v in enumerate(self.gprs):struct.pack_into('<Q',d,8+8*i,v)
  struct.pack_into('<QQ',d,0x100,self.sigsp,self.pc);d[0x110:0x310]=self.vectors
  struct.pack_into('<II',d,0x310,self.fpcr,self.fpsr);return d
 def context_to_mc(self,ctx):
  d=bytearray(self.original_mc)
  d[0x10:0x108]=ctx[8:0x100];d[0x108:0x118]=ctx[0x100:0x110]
  d[0x118:0x11c]=ctx[4:8];d[0x120:0x320]=ctx[0x110:0x310]
  d[0x324:0x328]=ctx[0x310:0x314];d[0x320:0x324]=ctx[0x314:0x318];return d
 def hook(self,u,addr,size,name):
  self.events.append(name);assert u.reg_read(ar.UC_ARM64_REG_SP)%16==0
  x=[u.reg_read(r)for r in R[:3]];lr=u.reg_read(R[30]);ret=0
  if name=='teb':ret=0 if self.c.get('no_teb')else TEB
  elif name=='tls':
   assert x[0]in(DESC_TRAMP,DESC_TEB)
   ret=TRAMP_SLOT if x[0]==DESC_TRAMP else TEB_SLOT
   # Darwin TLV resolver ABI preserves all registers except its returned x0.
   u.reg_write(R[0],ret);u.reg_write(ar.UC_ARM64_REG_PC,lr);return
  elif name=='memcpy':
   assert x[2]==512
   u.mem_write(x[0],bytes(u.mem_read(x[1],x[2])));ret=x[0]
  elif name=='get':
   assert x[:2]==[(1<<64)-2,self.sp-0x3d0]
   assert self.r32(x[1])==0x40400007
   u.mem_write(x[1],bytes(self.make_context(0x40400007)))
  elif name=='wait':
   assert x[0]==self.sp-0x3d0
   wanted=0x40400007 if self.inside else 0x80400007
   assert self.r32(x[0])==wanted,(hex(self.r32(x[0])),hex(wanted))
   ctx=bytearray(u.mem_read(x[0],0x390))
   expected=self.make_context(wanted)
   # The unused debug-context tail is intentionally uninitialized by save.
   assert ctx[:0x318]==expected[:0x318]
   if self.c.get('mutate'):
    for i in range(31):struct.pack_into('<Q',ctx,8+8*i,0x8989000000000000+i)
    struct.pack_into('<I',ctx,4,0xa0000000)
    struct.pack_into('<Q',ctx,0x100,0x765432100000)
    struct.pack_into('<Q',ctx,0x108,self.c.get('new_pc',self.pc+4))
    ctx[0x110:0x310]=bytes(v^0xa5 for v in self.vectors)
    struct.pack_into('<II',ctx,0x310,0x1000040,0x8000001)
    u.mem_write(x[0],bytes(ctx))
   self.wait_context=bytes(ctx);self.expected_mc=self.context_to_mc(ctx)
  elif name=='set':
   assert x[:2]==[(1<<64)-2,self.sp-0x3d0]
   self.set_context=bytes(u.mem_read(x[1],0x390));assert self.set_context==self.wait_context
  elif name=='dprintf':assert x[0]==2 and u.reg_read(ar.UC_ARM64_REG_SP)==self.sp
  else:raise AssertionError(name)
  # Public C helpers may destroy caller-save GPRs, SIMD registers, and flags.
  for i in range(18):u.reg_write(R[i],0xcab0000000000000+i)
  for i in list(range(8))+list(range(16,32)):u.reg_write(getattr(ar,'UC_ARM64_REG_Q'+str(i)),0x123456789abcdef)
  u.reg_write(R[0],ret);u.reg_write(ar.UC_ARM64_REG_NZCV,0xb0000000);u.reg_write(ar.UC_ARM64_REG_PC,lr)
 def run(self):
  self.u.emu_start(START,DONE,count=3000)
  assert self.u.reg_read(ar.UC_ARM64_REG_PC)==DONE
  assert self.u.reg_read(ar.UC_ARM64_REG_SP)==self.sp
  for i in range(18,31):assert self.u.reg_read(R[i])==self.regs[i],('ABI register',i)
  assert self.r32(TOTAL)==8 and self.r32(LAST)==30
  rx,size=self.c.get('rx',0x140000000),self.c.get('size',0x20000000)
  inpool=bool(rx and rx<=self.pc<((rx+size)&((1<<64)-1)))
  assert self.r32(IN_POOL)==13+inpool
  if self.c.get('no_teb'):
   assert self.events==(['teb','dprintf']if self.c.get('noteb_count',0)<=3 else['teb'])
   assert bytes(self.u.mem_read(MC,0x328))==self.original_mc
  elif self.inside:
   assert self.events==['teb','get','wait','set']
   assert bytes(self.u.mem_read(MC,0x328))==self.original_mc
   assert self.set_context==self.wait_context
  else:
   assert self.events[:4]==['teb','memcpy','wait','memcpy']
   expected=bytearray(self.expected_mc)
   pc=struct.unpack_from('<Q',expected,0x110)[0]
   redirected=self.variant=='original'and not self.c.get('null_tramp')and not self.c.get('null_signal_teb')and rx and rx<=pc<((rx+size)&((1<<64)-1))
   if self.variant=='original':assert self.events[4:]==['tls','tls']
   else:assert len(self.events)==4
   if redirected:
    struct.pack_into('<Q',expected,0x98,pc);struct.pack_into('<Q',expected,0x110,TRAMP)
   assert bytes(self.u.mem_read(MC,0x328))==expected,('context bytes changed',self.variant,self.c)
  return {'name':self.c['name'],'variant':self.variant,'inside_syscall':self.inside,'events':self.events,
          'output_pc':hex(self.r64(MC+0x110)),'output_x17':hex(self.r64(MC+0x98)),'passed':True}

def resume(handler):
 """Feed actual handler output into the captured guard sequence.

 Darwin's x18-zero behavior and the original trampoline's restore+branch are
 explicitly modeled. No real sigreturn or Mach x18 recovery is tested here.
 """
 u=Uc(UC_ARCH_ARM64,UC_MODE_ARM);u.mem_map(0x14ff44000,0x4000)
 words=[0xf9405f91,0x91500231,0x1000008a,0xa9bf2a26,0xf9005b91]
 u.mem_write(JIT_START,struct.pack('<5I',*words));u.mem_protect(0x14ff44000,0x4000,UC_PROT_READ|UC_PROT_EXEC)
 u.mem_map(FRAME&~4095,4096);u.mem_map(CR_BASE,0x1000000)
 u.mem_write(FRAME+0xb8,struct.pack('<Q',CR_BASE));u.mem_write(FRAME+0xb0,struct.pack('<Q',CR_SP))
 for i in range(31):u.reg_write(R[i],handler.r64(MC+0x10+8*i))
 u.reg_write(ar.UC_ARM64_REG_SP,handler.r64(MC+0x108));u.reg_write(ar.UC_ARM64_REG_NZCV,handler.r32(MC+0x118)&0xf0000000)
 pc=handler.r64(MC+0x110)
 u.reg_write(R[18],0) # Modeled Darwin behavior, not kernel verification.
 used_trampoline=pc==TRAMP
 if used_trampoline:u.reg_write(R[18],TEB);pc=u.reg_read(R[17]) # Modeled original trampoline.
 faults=[]
 def invalid(uc,access,address,size,value,data):faults.append({'address':hex(address),'size':size});return False
 u.hook_add(UC_HOOK_MEM_INVALID,invalid)
 error=None
 try:u.emu_start(pc,JIT_STOP,count=10)
 except UcError as e:error=str(e)
 return {'resumed_pc':hex(pc),'modeled_original_trampoline':used_trampoline,'x18_after_modeled_return':hex(u.reg_read(R[18])),
         'faults':faults,'error':error,'state_callret_sp':hex(struct.unpack('<Q',u.mem_read(FRAME+0xb0,8))[0])}

def main():
 data=BASE.read_bytes();assert sha(data)==BASE_SHA
 assert data[fm.file_offset(data,PATCH,4):fm.file_offset(data,PATCH,4)+4]==bytes.fromhex('e05a00b0')
 cases=[]
 pcs=[0x13ffffffc,0x140000000,JIT_ADR,0x15ffffffc,0x160000000,0x180000000]
 for sp in [0x70001ffc,0x70002000,0x70002800,0x70003000,0x70003004]:
  for pc in pcs:
   for mutate in [False,True]:cases.append({'name':f'boundary-{sp:x}-{pc:x}-{mutate}','sp':sp,'pc':pc,'mutate':mutate})
 for name,extra in [('no-pool',{'rx':0}),('zero-size',{'size':0}),('null-tramp',{'null_tramp':True}),('null-signal-teb',{'null_signal_teb':True}),('both-null',{'null_tramp':True,'null_signal_teb':True}),('post-wait-leaves-pool',{'mutate':True,'new_pc':0x180000000}),('post-wait-enters-pool',{'pc':0x180000000,'mutate':True,'new_pc':JIT_ADR})]:
  cases.append({'name':name,**extra})
 for count in [0,3,4,100]:
  for pc in pcs:cases.append({'name':f'no-teb-{count}-{pc:x}','no_teb':True,'noteb_count':count,'pc':pc})
 rng=random.Random(1791)
 for i in range(64):cases.append({'name':f'random-registers-{i}','seed':rng.getrandbits(32),'mutate':bool(i%2),'sp':0x70001000 if i%3 else 0x70002800,'pc':pcs[i%len(pcs)]})
 results=[Handler(data,v,c).run()for v in ['original','candidate']for c in cases]
 controls=[]
 for v in ['original','candidate']:
  h=Handler(data,v,{'name':'actual-handler-output-resume','pc':JIT_ADR});h.run();r=resume(h);r['variant']=v;controls.append(r)
 assert controls[0]['faults'][0]['address']=='0x14ff46478'
 assert controls[0]['state_callret_sp']==hex(CR_SP)
 assert not controls[1]['faults'] and controls[1]['state_callret_sp']==hex(CR_SP-16)
 assert controls[1]['x18_after_modeled_return']=='0x0'
 offset=fm.file_offset(data,PATCH,4);candidate=data[:offset]+struct.pack('<I',0x14000035)+data[offset+4:]
 report={'result':'PASS','native_cases':len(results),'baseline_sha256':BASE_SHA,'synthetic_candidate_sha256':sha(candidate),
         'candidate_patch':{'address':hex(PATCH),'target':hex(EPILOGUE),'bytes':'35000014'},
         'scope':'Complete exact original/proposed usr1_handler plus exact native save_context/restore_context instruction bodies. NtCurrentTeb, wait_suspend, NtGet/NtSetContextThread, Darwin TLV resolver, memcpy and dprintf are explicit mocks. Context GPR/SP/PC/NZCV/vector/FP changes, pool bounds, TLS absence, inside-syscall behavior and ABI checked.',
         'resumption_controls':controls,'cases':results,
         'limitations':['No actual Darwin sigreturn, signal delivery, nested native signals or Mach x18 repair executed.','Candidate resumes the captured JIT sequence with x17 intact while modeled x18 remains zero; this test cannot establish that downstream x18-dependent code recovers.','APC work is modeled as modifications to the context in wait_suspend; real APC/server logic remains covered by separate tests.','No native executable, source implementation, app or device was modified.']}
 (H/'usr1-native-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({k:v for k,v in report.items()if k!='cases'},indent=2))

if __name__=='__main__':main()
