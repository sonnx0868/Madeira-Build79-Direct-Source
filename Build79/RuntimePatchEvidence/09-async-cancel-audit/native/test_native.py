"""Run complete pinned ARM64 request handlers with instrumented lifecycle helpers."""
from pathlib import Path
import hashlib,importlib.util,json,random,struct,sys
from unicorn import Uc, UcError, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn import arm64_const as ar
HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('native_build',HERE/'build.py');b=importlib.util.module_from_spec(spec);spec.loader.exec_module(b)
R=[getattr(ar,'UC_ARM64_REG_X'+str(i)) for i in range(31)]
MEM,STACK,DONE=0x20000000,0x30000000,0x40000000
REQ,REPLY,PROC,THREAD,TARGET,GROUP,SYNC= [MEM+n for n in (0,0x100,0x1000,0x2000,0x3000,0x4000,0x5000)]
HEAD=PROC+0x118
class Run:
 def __init__(self,variant,case):
  self.case=case;self.events=[];self.objects={};self.pending=[];self.freed=[];self.restoring=False
  self.u=u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
  pages={a&~4095 for a in b.SYMS.values()}|{b.START&~4095,(b.END-1)&~4095}
  for p in sorted(pages):u.mem_map(p,4096)
  for p,s in [(MEM,0x100000),(STACK,0x10000),(DONE,4096)]:u.mem_map(p,s)
  data=(HERE/variant).read_bytes();off=b.file_offset(data,b.START,b.END-b.START);u.mem_write(b.START,data[off:off+b.END-b.START])
  self.w64(b.SYMS['current'],THREAD);self.w64(THREAD+0x88,PROC)
  self.w32(REQ+0xc,0x48);self.w64(REQ+0x10,case.get('iosb',0));self.w32(REQ+0x18,case.get('only',False));self.w32(REPLY+8,0xdeadbeef)
  self.w64(HEAD,HEAD);self.w64(HEAD+8,HEAD)
  self.make(TARGET,'target',2)
  self.ops=[]
  for i,d in enumerate(case.get('ops',[])):
   a=MEM+0x10000+i*0x400;self.ops.append(a);self.make(a,'async',1);self.objects[a]['desc']=d
   self.w64(a+0x48,THREAD+(0x10000 if d.get('other_thread') else 0));self.w64(a+0x78,a+0x200)
   self.w64(a+0x200,TARGET+(0x10000 if d.get('other_object') else 0));self.w64(a+0xa0,d.get('iosb',7))
   flags=(0x10 if d.get('terminated') else 0)|(0x100 if d.get('system') else 0)|(0x20 if d.get('canceled') else 0)
   self.w16(a+0xd0,flags);self.add_tail(HEAD,a+0x60)
  self.sentinels=[0x1234500000000000+i for i in range(31)];self.sentinels[0]=REQ;self.sentinels[1]=REPLY;self.sentinels[30]=DONE
  for reg,val in zip(R,self.sentinels):u.reg_write(reg,val)
  self.sp=STACK+0x8000;u.reg_write(ar.UC_ARM64_REG_SP,self.sp)
  for name,addr in b.SYMS.items():
   if name not in ('current','global_error','async_cancel_ops'):u.hook_add(UC_HOOK_CODE,self.hook,begin=addr,end=addr,user_data=name)
 def w16(self,a,v):self.u.mem_write(a,struct.pack('<H',v))
 def w32(self,a,v):self.u.mem_write(a,struct.pack('<I',v))
 def w64(self,a,v):self.u.mem_write(a,struct.pack('<Q',v))
 def r16(self,a):return struct.unpack('<H',self.u.mem_read(a,2))[0]
 def r32(self,a):return struct.unpack('<I',self.u.mem_read(a,4))[0]
 def r64(self,a):return struct.unpack('<Q',self.u.mem_read(a,8))[0]
 def make(self,a,kind,refs):self.objects[a]={'kind':kind,'live':True};self.w32(a,refs)
 def add_tail(self,h,e):
  prev=self.r64(h+8);self.w64(e,h);self.w64(e+8,prev);self.w64(prev,e);self.w64(h+8,e)
 def remove(self,e):
  n,p=self.r64(e),self.r64(e+8);assert n and p,('freed node',hex(e));self.w64(n+8,p);self.w64(p,n);self.w64(e,0);self.w64(e+8,0)
 def retain(self,a):
  assert self.objects[a]['live'];n=self.r32(a);assert n;self.w32(a,n+1)
 def release(self,a):
  assert a in self.objects and self.objects[a]['live'],('release dead',hex(a));n=self.r32(a);assert n;self.w32(a,n-1)
  if n!=1:return
  obj=self.objects[a];obj['live']=False;self.freed.append(a)
  if obj['kind']=='async':
   assert not self.r64(a+0x100),'async destroyed with group attached'
   self.remove(a+0x60)
   # A destructor may reselect and complete a later tracked operation.
   if self.case.get('restore_cross') and a==self.ops[0] and len(self.ops)>1:
    other=self.ops[1]
    if self.objects[other]['live'] and other in self.pending:self.complete(other)
  elif obj['kind']=='group':
   if self.r64(a+0x48):self.release(self.r64(a+0x48))
 def complete(self,a):
  assert self.objects[a]['live'];g=self.r64(a+0x100)
  if g:
   assert self.objects[g]['live'];self.w64(a+0x100,0);n=self.r32(g+0x50);assert n
   self.w32(g+0x50,n-1)
   if n==1:self.events.append(('signal',g));self.release(g)
  if a in self.pending:self.pending.remove(a)
  self.w16(a+0xd0,self.r16(a+0xd0)|0x10)
  self.release(a)
 def hook(self,u,address,size,name):
  x=[u.reg_read(r) for r in R[:4]];lr=u.reg_read(R[30]);v=0
  if name=='get_handle_obj':
   assert x==[PROC,0x48,0,0]
   v=0 if self.case.get('invalid_handle') else TARGET
   if not v:self.w32(b.SYMS['global_error'],0xc0000008)
  elif name=='get_fd_user':v=self.r64(x[0])
  elif name=='alloc_object':
   assert x[0]==b.SYMS['async_cancel_ops']
   if self.case.get('alloc_fail'):self.w32(b.SYMS['global_error'],0xc0000017)
   else:self.make(GROUP,'group',1);v=GROUP
  elif name=='create_internal_sync':
   assert x[:2]==[1,0]
   if self.case.get('sync_fail'):self.w32(b.SYMS['global_error'],0xc0000017)
   else:self.make(SYNC,'sync',1);v=SYNC
  elif name=='grab_object':self.events.append(('grab',x[0]));self.retain(x[0]);v=x[0]
  elif name=='release_object':self.events.append(('release',x[0]));self.release(x[0])
  elif name=='alloc_handle':
   assert x==[PROC,GROUP,0x100000,0];assert self.objects[GROUP]['live'] and self.r32(GROUP+0x50)>0
   if self.case.get('handle_fail'):self.w32(b.SYMS['global_error'],0xc0000017)
   else:self.retain(GROUP);v=0x424;self.events.append(('handle',GROUP))
  elif name=='fd_cancel_async':
   a=x[1];assert x[0]==a+0x200 and self.r16(a+0xd0)&0x20
   self.events.append(('cancel',a));d=self.objects[a]['desc']
   if self.case.get('cross_previous') and a==self.ops[1] and self.ops[0] in self.pending:self.complete(self.ops[0])
   if self.case.get('cross_future') and a==self.ops[0] and len(self.ops)>1:self.complete(self.ops[1])
   if d.get('mode','sync')=='sync':self.complete(a)
   else:self.pending.append(a)
  else:raise AssertionError('native assertion')
  # Helpers may clobber caller-save registers; Apple platform register x18 retained.
  for i in range(18):u.reg_write(R[i],0xabcd0000+i)
  u.reg_write(R[0],v);u.reg_write(ar.UC_ARM64_REG_NZCV,0xa0000000);u.reg_write(ar.UC_ARM64_REG_PC,lr)
 def run(self):
  self.u.emu_start(b.START,DONE,count=200000)
  assert self.u.reg_read(ar.UC_ARM64_REG_PC)==DONE
  assert self.u.reg_read(ar.UC_ARM64_REG_SP)==self.sp
  for i in range(18,31):assert self.u.reg_read(R[i])==self.sentinels[i],('ABI',i)
  c=self.case;only=c.get('only',False);blocked=c.get('invalid_handle') or (only and(c.get('alloc_fail')or c.get('sync_fail')))
  selected=[]
  if not blocked:
   for a in self.ops:
    d=self.objects[a]['desc']
    if d.get('terminated')or d.get('system')or d.get('other_object')or(only and d.get('other_thread'))or(c.get('iosb')and c['iosb']!=d.get('iosb',7)):continue
    if c.get('cross_future') and a==self.ops[1]:continue
    selected.append(a)
  actual=[a for op,a in self.events if op=='cancel']
  assert actual==[a for a in selected if not self.objects[a]['desc'].get('canceled')],(c,actual,selected)
  expected_error=0xc0000008 if c.get('invalid_handle') else 0xc0000017 if only and (c.get('alloc_fail')or c.get('sync_fail')or(c.get('handle_fail')and any(op=='alloc_handle' for op,a in []))) else 0
  # Allocation failure/error states are preserved, NOT_FOUND only applies to process-wide empty matches.
  if only and c.get('handle_fail') and GROUP in self.objects and self.r32(GROUP+0x50)>0:expected_error=0xc0000017
  if not c.get('invalid_handle') and not only and not selected:expected_error=0xc0000225
  assert self.r32(b.SYMS['global_error'])==expected_error,(c,hex(self.r32(b.SYMS['global_error'])),hex(expected_error))
  expected_reply=0xdeadbeef if c.get('invalid_handle')or(not only and not selected) else 0x424 if any(op=='handle' for op,a in self.events)else 0
  assert self.r32(REPLY+8)==expected_reply
  # Every surviving list element must be doubly linked exactly once.
  seen=[];p=self.r64(HEAD);prev=HEAD
  while p!=HEAD:
   assert len(seen)<len(self.ops)+1 and self.r64(p+8)==prev;seen.append(p-0x60);prev,p=p,self.r64(p)
  assert self.r64(HEAD+8)==prev
  assert set(seen)=={a for a in self.ops if self.objects[a]['live']}
  # Drive all deferred completions, close returned handles, release excluded operations.
  for a in list(self.pending):self.complete(a)
  for a in self.ops:
   if self.objects[a]['live']:self.complete(a)
  if any(op=='handle' for op,a in self.events):self.release(GROUP)
  for a,o in self.objects.items():
   if o['kind']!='target':assert not o['live'],('leak',c,hex(a),o)
  assert self.r64(HEAD)==HEAD and self.r64(HEAD+8)==HEAD
  assert self.r32(TARGET)==(2 if c.get('invalid_handle')else 1)
  return {'name':c['name'],'cancelled':len(actual),'reply':hex(expected_reply),'status':hex(expected_error),'passed':True}

def main():
 cases=[]
 for only in (False,True):
  for mode in ('sync','deferred'):
   for n in range(25):cases.append({'name':f'{only}-{mode}-{n}','only':only,'ops':[{'mode':mode} for _ in range(n)]})
 for only in (False,True):
  for key in ('cross_previous','restore_cross','cross_future'):
   cases.append({'name':f'{only}-{key}','only':only,key:True,'ops':[{'mode':'deferred' if key=='cross_previous' else 'sync'},{'mode':'deferred'},{'mode':'sync'}]})
  cases.append({'name':f'{only}-filters','only':only,'iosb':7,'ops':[{'other_object':True},{'other_thread':True},{'iosb':9},{'terminated':True},{'system':True},{'canceled':True},{'mode':'deferred'},{}]})
 for failure in ('invalid_handle','alloc_fail','sync_fail','handle_fail'):
  for mode in ('sync','deferred'):
   cases.append({'name':failure+'-'+mode,'only':True,failure:True,'ops':[{'mode':mode} for _ in range(3)]})
 rng=random.Random(934)
 for i in range(32):
  cases.append({'name':'mixed-'+str(i),'only':bool(i%2),'ops':[{'mode':rng.choice(['sync','deferred']),'other_object':rng.randrange(5)==0,'other_thread':rng.randrange(4)==0,'iosb':rng.randrange(4)}for _ in range(16)],'iosb':i%4})
 results=[]
 for c in cases:
  try:results.append(Run('Madeira',c).run())
  except Exception:print('FAILED',c,flush=True);raise
 controls=[]
 for c in ({'name':'original-sync-uaf','ops':[{}]}, {'name':'original-group-order','only':True,'ops':[{}]}):
  try:Run('Madeira-original',c).run()
  except (UcError,AssertionError) as e:controls.append({'name':c['name'],'expected_failure':str(e)})
  else:raise AssertionError('Original unexpectedly passed')
 report={'passed':True,'native_cases':len(results),'negative_controls':controls,'cases':results,
 'native_sha256':hashlib.sha256((HERE/'Madeira').read_bytes()).hexdigest(),
 'scope':'Complete original and patched ARM64 request instructions in Unicorn; instrumented object/FD helpers model synchronous/deferred and cross-operation completion. ABI, statuses, groups, list integrity and allocation balance checked. Not physical iPad execution; source ASan fixture independently exercises real extracted completion/APC bodies.'}
 (HERE/'test-report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items()if k!='cases'},indent=2))
if __name__=='__main__':main()
