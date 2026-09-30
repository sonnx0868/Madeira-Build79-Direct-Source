#!/usr/bin/env python3
"""Independent execution of exact candidate instructions with modeled Mach VM.

Real ARM64 instructions execute in Unicorn. Kernel protection/query/remap APIs
are explicit mocks; target reads use Unicorn permissions and zeroing checks the
modeled physical rights. This is not a Darwin kernel or device test.
"""
from pathlib import Path
import hashlib, importlib.util, json, struct
from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE, UC_HOOK_MEM_READ
from unicorn import arm64_const as ar

H=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('dc_filemap',H.parents[1]/'guest-exit-fix/build.py')
fm=importlib.util.module_from_spec(spec);spec.loader.exec_module(fm)
START,END=0x10013bfb0,0x10013c514
SYMS={'alias':0x10012ae68,'mmap':0x10012c3a0,'zero':0x1009e1644,
      'query':0x1009e1d64,'protect':0x1009e1d40,'deallocate':0x1009e1cd4,'dprintf':0x1009e17f4}
MASK,VP,AC,AT=0x100d5d270,0x100d69320,0x100d29820,0x100d29828
RW,RX,SIZE,LC,LT=0x101073fa8,0x101073fb0,0x101073fb8,0x100d5d2b8,0x100d5d2c0
GOT=0x100bc0e98
MEM,ALIAS,STACK,DONE,DATA=0x7000000000,0x7100000000,0x30000000,0x40000000,0x20000000
VPT,VPL=DATA+0x10000,DATA+0x20000
PAGE,TOTAL=0x4000,0x10000
DENIED,NOMEM=0xc0000022,0xc0000017
REGS=[getattr(ar,'UC_ARM64_REG_X'+str(i)) for i in range(31)]
sha=lambda b:hashlib.sha256(b).hexdigest()

class Case:
 def __init__(self,data,c):
  self.c=c;self.events=[];self.queries={};self.protects={};self.rights=[];self.deallocated=[];self.reads=0
  self.u=u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
  native_pages={a&~4095 for a in [START,END-1,*SYMS.values(),MASK,VP,AC,AT,RW,RX,SIZE,LC,LT,GOT]}
  for p in native_pages:u.mem_map(p,4096)
  u.mem_map(DATA,0x130000);u.mem_map(STACK,0x10000);u.mem_map(DONE,4096)
  for lo,hi in [(START,END)]:
   off=fm.file_offset(data,lo,hi-lo);u.mem_write(lo,data[off:off+hi-lo])
  for a in (MEM,ALIAS):u.mem_map(a,TOTAL)
  self.original=bytes((i*7%251)+1 for i in range(TOTAL))
  u.mem_write(MEM,self.original);u.mem_write(ALIAS,self.original)
  self.base=c.get('base',MEM+c.get('offset',0x1000));self.size=c.get('size',0x7000)
  self.actual_size=c.get('view_size',0x7000) if not self.size else self.size
  self.prots={MEM+i:c.get('prot',0) for i in range(0,TOTAL,PAGE)}
  self.maxs={p:c.get('max',7) for p in self.prots}
  self.prots.update(c.get('prots',{}));self.maxs.update(c.get('maxs',{}));self.initial_prots=self.prots.copy()
  for p,v in self.prots.items():u.mem_protect(p,PAGE,v&7)
  self.w64(MASK,PAGE-1);self.w64(GOT,DATA);self.w32(DATA,0x1234);self.w64(DATA+0x128,c.get('view_size',0x7000))
  self.w64(VP,VPT);self.w64(VPT+((MEM>>32)*8),VPL)
  self.logical=bytes((i*13+0x20)&255 for i in range(16));u.mem_write(VPL,self.logical)
  entries=c.get('aliases',[])
  if c.get('alias'):entries=[(MEM,MEM+TOTAL,ALIAS,ALIAS)]+entries
  self.entries=entries;self.w32(AC,len(entries))
  for i,e in enumerate(entries):u.mem_write(AT+i*32,struct.pack('<4Q',*e))
  self.w64(RW,c.get('pool_rw',0));self.w64(RX,c.get('pool_rx',0));self.w64(SIZE,c.get('pool_size',0))
  ledger=c.get('ledger',[]);self.w32(LC,len(ledger))
  for i,e in enumerate(ledger):u.mem_write(LT+24*i,struct.pack('<3Q',*e))
  self.regs=[0xaadd000000000000+i for i in range(31)];self.regs[:3]=[DATA+0x100,self.base,self.size];self.regs[30]=DONE
  for r,v in zip(REGS,self.regs):u.reg_write(r,v)
  self.sp=STACK+0x9000;u.reg_write(ar.UC_ARM64_REG_SP,self.sp)
  for k,a in SYMS.items():u.hook_add(UC_HOOK_CODE,self.hook,begin=a,end=a,user_data=k)
  u.hook_add(UC_HOOK_MEM_READ,self.read,begin=MEM,end=MEM+TOTAL-1)
 def w64(self,a,v):self.u.mem_write(a,struct.pack('<Q',v&((1<<64)-1)))
 def w32(self,a,v):self.u.mem_write(a,struct.pack('<I',v&0xffffffff))
 def r64(self,a):return struct.unpack('<Q',self.u.mem_read(a,8))[0]
 def r32(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
 def read(self,u,access,a,n,value,user):
  assert self.prots[a&~(PAGE-1)]&1,('read without physical READ',hex(a));self.reads+=n
 def clobber(self,ret,lr):
  for i in range(18):self.u.reg_write(REGS[i],0xbbcc000000000000+i)
  self.u.reg_write(ar.UC_ARM64_REG_NZCV,0xb0000000)
  self.u.reg_write(REGS[0],ret&((1<<64)-1));self.u.reg_write(ar.UC_ARM64_REG_PC,lr)
 def hook(self,u,a,n,name):
  x=[u.reg_read(r) for r in REGS[:8]];lr=u.reg_read(REGS[30]);ret=0
  assert u.reg_read(ar.UC_ARM64_REG_SP)==self.sp-0x190,('frame',name)
  self.events.append((name,*x[:3]))
  if name=='alias':
   ret=next((rw+x[0]-b for b,e,rw,rx in self.entries if b<=x[0]<e),0)
  elif name=='query':
   assert x[0]==0x1234 and x[3]==9 and self.r32(x[5])==9 and self.r32(x[6])==0
   p=self.r64(x[1]);assert p in self.prots and p%PAGE==0
   seq=self.queries[p]=self.queries.get(p,0)+1;inj=self.c.get('query_inject',{}).get((p,seq))
   right=0x500+len(self.rights);self.rights.append(right);self.w32(x[6],right)
   addr=p;size=PAGE;current=self.prots[p];maximum=self.maxs[p];count=9
   if inj=='fail':ret=1
   elif inj=='short':size=PAGE-1
   elif inj=='gap':addr=p+PAGE
   elif inj=='coverage':addr=p-PAGE;size=PAGE
   elif inj=='count':count=8
   elif inj=='wrong_prot':current^=1
   elif inj=='wrong_max':maximum^=4
   elif inj=='larger':addr=p-PAGE;size=PAGE*3
   self.w64(x[1],addr);self.w64(x[2],size);self.w32(x[5],count)
   u.mem_write(x[4],struct.pack('<9I',current,maximum,0,0,0,0,0,0,0))
  elif name=='deallocate':
   assert x[0]==0x1234 and x[1] in self.rights and x[1] not in self.deallocated
   self.deallocated.append(x[1])
  elif name=='protect':
   p=x[1];assert x[0]==0x1234 and p in self.prots and x[2:4]==[PAGE,0]
   desired=x[4];assert desired in (self.initial_prots[p],self.initial_prots[p]|3)
   assert not desired&4 and not desired&~self.maxs[p],('unexpected execute/COPY/right',desired)
   seq=self.protects[p]=self.protects.get(p,0)+1;inj=self.c.get('protect_inject',{}).get((p,seq))
   if inj not in ('fail','silent'):
    self.prots[p]=desired;u.mem_protect(p,PAGE,desired&7)
   if inj in ('fail','applied_error'):ret=1
  elif name=='mmap':
   lo,length=x[:2];assert x[2:4]==[3,0] and lo%PAGE==0 and length%PAGE==0
   assert self.base<=lo<lo+length<=self.base+self.actual_size
   if self.c.get('mmap_fail'):ret=(1<<64)-1
   else:
    for p in range(lo,lo+length,PAGE):self.prots[p]=3;u.mem_protect(p,PAGE,3)
    u.mem_write(lo,bytes(length));ret=lo
  elif name=='dprintf':
   assert self.c.get('ledger') and x[:2]==[2,0x100adf907]
   assert [self.r64(self.sp-0x190+i*8) for i in range(5)]==[self.base,self.actual_size,ALIAS+self.base-MEM,self.c['ledger'][0][0],self.c['ledger'][0][2]]
  elif name=='zero':
   lo,length=x[:2];assert length>0
   if not ALIAS<=lo<ALIAS+TOTAL:
    assert self.base<=lo<lo+length<=self.base+self.actual_size
    for p in range(lo&~(PAGE-1),(lo+length+PAGE-1)&~(PAGE-1),PAGE):
     assert self.prots[p]&3==3 and not self.prots[p]&4,('unsafe clear',hex(lo),self.prots[p])
   u.mem_write(lo,bytes(length))
   if self.c.get('bad_zero'):u.mem_write(lo+length-1,b'\xff')
  else:raise AssertionError(name)
  self.clobber(ret,lr)
 def run(self):
  self.u.emu_start(START,DONE,count=1000000)
  assert self.u.reg_read(ar.UC_ARM64_REG_PC)==DONE,'did not return'
  assert self.u.reg_read(ar.UC_ARM64_REG_SP)==self.sp
  for i in range(18,31):assert self.u.reg_read(REGS[i])==self.regs[i],('callee save',i)
  status=self.u.reg_read(REGS[0])&0xffffffff;assert status==self.c.get('status',0),(self.c['name'],hex(status))
  preflight_failure=any(seq<=2 and inj!='larger' for (p,seq),inj in self.c.get('query_inject',{}).items()) or any(seq==1 for p,seq in self.c.get('protect_inject',{})) or self.c.get('preflight_failure',False)
  if preflight_failure:
   assert not any(e[0] in ('zero','mmap') for e in self.events),'cleared/remapped before all edge preparations succeeded'
   assert bytes(self.u.mem_read(MEM,TOTAL))==self.original,'failed preflight changed any data'
  assert self.rights==self.deallocated,'leaked Mach query right'
  logical=bytearray(self.logical)
  if not status:
   for i in range((self.base-MEM)//4096,(self.base-MEM+self.actual_size+4095)//4096):logical[i]&=~0x20
  assert bytes(self.u.mem_read(VPL,16))==logical,'logical commitment wrong'
  if MEM<=self.base<self.base+self.actual_size<=MEM+TOTAL:
   data=bytes(self.u.mem_read(MEM,TOTAL));lo=self.base-MEM;hi=lo+self.actual_size
   assert data[:lo]==self.original[:lo] and data[hi:]==self.original[hi:],'neighbor data changed'
   if not status and not self.c.get('alias'):assert data[lo:hi]==bytes(hi-lo),'not zero'
   for p in self.prots:
    interior=((self.base+PAGE-1)&~(PAGE-1))<=p and p+PAGE<=((self.base+self.actual_size)&~(PAGE-1))
    # Only successfully remapped full interior pages intentionally remain RW.
    remapped=any(e[0]=='mmap' for e in self.events) and not self.c.get('mmap_fail') and interior
    expected=3 if remapped else self.initial_prots[p]
    if p not in self.c.get('unrestored',[]):assert self.prots[p]==expected,('restore',hex(p),self.prots[p],expected)
   if self.c.get('alias'):
    alias=bytes(self.u.mem_read(ALIAS,TOTAL));expected=bytearray(self.original)
    if not self.c.get('ledger'):expected[lo:hi]=bytes(hi-lo)
    assert alias==expected,'alias behavior changed'
  return {'name':self.c['name'],'status':hex(status),'calls':len(self.events),'verified_read_bytes':self.reads,'passed':True}

def caller_test(data,status):
 u=Uc(UC_ARCH_ARM64,UC_MODE_ARM);u.mem_map(0x100134000,4096);u.mem_map(START&~4095,4096);u.mem_map(DATA,4096)
 lo,hi=0x100134a6c,0x100134aac;off=fm.file_offset(data,lo,hi-lo);u.mem_write(lo,data[off:off+hi-lo])
 for i,v in [(19,DATA),(20,DATA+8),(22,0x123456),(25,0x765432)]:u.reg_write(REGS[i],v)
 u.mem_write(DATA,b'\xcc'*16)
 def call(u,a,n,d):u.reg_write(REGS[0],status);u.reg_write(ar.UC_ARM64_REG_PC,u.reg_read(REGS[30]))
 u.hook_add(UC_HOOK_CODE,call,begin=START,end=START)
 stop=0x100134978 if status else hi
 u.emu_start(lo,stop,count=100)
 assert u.reg_read(ar.UC_ARM64_REG_PC)==stop and u.reg_read(REGS[24])==status
 expected=b'\xcc'*16 if status else struct.pack('<QQ',0x765432,0x123456)
 assert bytes(u.mem_read(DATA,16))==expected
 return {'status':hex(status),'outputs_preserved_on_failure':bool(status),'passed':True}

def main():
 data=(H/'Madeira').read_bytes();cases=[]
 for name,off,size in [('leading',0x1000,0x7000),('trailing',0,0x7000),('subpage',0x1000,0x2000),('bridge',0x1000,0x6000),('both',0x1000,0xa000),('whole',0,0x8000),('tiny',0x1000,0x1000)]:
  for prot in [0,1,2,3]:cases.append({'name':f'{name}-prot{prot}','offset':off,'size':size,'prot':prot})
 for page in [MEM,MEM+PAGE*2]:
  for seq in [1,2,3]:
   for inj in ['fail','short','gap','coverage','count','wrong_prot','wrong_max']:
    # Initial current/max values are authoritative; mutations there are not query failures.
    if seq==1 and inj in ('wrong_prot','wrong_max'):continue
    cases.append({'name':f'query-{page:x}-{seq}-{inj}','size':0xa000,'query_inject':{(page,seq):inj},'status':DENIED})
  for seq in [1,2]:
   for inj in ['fail','silent','applied_error']:
    cases.append({'name':f'protect-{page:x}-{seq}-{inj}','size':0xa000,'protect_inject':{(page,seq):inj},'status':DENIED,'unrestored':[page] if seq==2 and inj!='applied_error' else []})
 for prot,maxp in [(4,7),(5,7),(7,7),(0,1),(1,1),(2,2)]:cases.append({'name':f'reject-{prot}-{maxp}','prot':prot,'max':maxp,'status':DENIED})
 cases += [
  {'name':'mmap-failure-restores-two','size':0xa000,'mmap_fail':True,'status':NOMEM},
  {'name':'failed-clear-readback-restores-two','size':0xa000,'bad_zero':True,'status':DENIED},
  {'name':'mixed-physical-pages','size':0xa000,'prots':{MEM:1,MEM+PAGE*2:3}},
  {'name':'larger-query-region','query_inject':{(MEM,1):'larger',(MEM,2):'larger',(MEM,3):'larger'}},
  {'name':'zero-size-uses-view','size':0,'view_size':0x7000},
  {'name':'zero-view-rejected','size':0,'view_size':0,'status':DENIED},
  {'name':'overflow-rejected','base':0xfffffffffffff000,'size':0x2000,'status':DENIED},
  {'name':'plain-neighbor-alias-rejected','aliases':[(MEM,MEM+0x1000,ALIAS,ALIAS)],'status':DENIED},
  {'name':'plain-pool-RW-rejected','pool_rw':MEM,'pool_size':0x1000,'status':DENIED},
  {'name':'plain-pool-RX-rejected','pool_rx':MEM,'pool_size':0x1000,'status':DENIED},
  {'name':'second-edge-alias-rejected','size':0xa000,'aliases':[(MEM+0xb000,MEM+0xc000,ALIAS,ALIAS)],'status':DENIED,'preflight_failure':True},
  {'name':'second-edge-pool-rejected','size':0xa000,'pool_rw':MEM+0xb000,'pool_size':0x1000,'status':DENIED,'preflight_failure':True},
  {'name':'normal-alias-clear','alias':True},
  {'name':'live-alias-refusal','alias':True,'pool_rw':ALIAS,'pool_size':TOTAL,'ledger':[(0,0x8000,123)]},
 ]
 results=[]
 for c in cases:
  try:results.append(Case(data,c).run())
  except Exception as e:raise AssertionError(c['name']) from e
 repeated=Case(data,{'name':'repeat-dirty-logically-uncommitted','prot':3,'size':0xa000})
 repeated.run();repeated.u.mem_write(repeated.base,bytes([0xe7])*repeated.actual_size)
 for r,v in zip(REGS,repeated.regs):repeated.u.reg_write(r,v)
 results.append(repeated.run())
 baseline=(H.parents[1]/'apc-context-fix/Madeira').read_bytes()
 assert sha(baseline)=='f3269d77d5e9872c0f3506570de11b9a4b707b5753d356176e1ba2691417c1f4'
 control=Case(baseline,{'name':'original-leading-protected-clear'})
 try:control.run()
 except AssertionError as e:assert e.args[0]==('unsafe clear',hex(MEM+0x1000),0),e
 else:raise AssertionError('Original negative control unexpectedly passed')
 negative={'original_sha256':sha(baseline),'unsafe_clear_address':hex(MEM+0x1000),'physical_protection':0,'detected':True}
 callers=[caller_test(data,s) for s in [0,DENIED,NOMEM,0xc0000001]]
 report={'result':'PASS','native_sha256':sha(data),'cases':results,'caller_cases':callers,'original_negative_control':negative,
  'limitations':['Mach VM, alias lookup, bzero, and remap helpers are modeled, not executed on Darwin.','No real device/CEF/game launch or concurrent Mach exception server is executed.','Kernel refusal to restore rights is reported as error; restoration cannot be guaranteed after an OS failure.']}
 (H/'native-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
 print(json.dumps({'result':'PASS','native_sha256':sha(data),'function_cases':len(results),'caller_cases':len(callers)},indent=2))
if __name__=='__main__':main()
