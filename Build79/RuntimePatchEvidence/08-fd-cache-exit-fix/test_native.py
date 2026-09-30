"""Execute the complete native cache-release function with instrumented OS calls."""
from pathlib import Path
import hashlib, json, random, struct, sys
from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn import arm64_const as ar
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent/'guest-exit-fix'))
from build import file_offset

START, END = 0x1000f8fbc, 0x1000f9128
COUNT, SLOTS = 0x100cd4a2c, 0x100cd4a30
CACHE, EXTRA, STACK, DONE = 0x21000000, 0x22000000, 0x30000000, 0x40000000
PEB = 0x106280000
HOOKS = {0x1009e231c:'lock', 0x1009e2334:'unlock', 0x1009e1944:'free',
         0x1009e17f4:'log', 0x1000f9128:'note', 0x1009e16b0:'close'}
registers = [getattr(ar, 'UC_ARM64_REG_X'+str(i)) for i in range(31)]

def encode(fd_encoded, typ, flags=0):
    return ((flags & 0xffffffe0 | typ) << 32) | (fd_encoded & 0xffffffff)

cases = [dict(name='no slots', count=0), dict(name='empty cache'),
         dict(name='unmatched PEB', matching=False), dict(name='slot 63', slot=63),
         dict(name='already released', active=False), dict(name='null cache', null=True),
         dict(name='fd zero and block edges', entries=[(0,0,encode(1,1)),(0,8191,encode(235,1)),
                                                       (127,0,encode(250,3)),(127,8191,encode(252,7))]),
         dict(name='invalid error entries', entries=[(0,1,encode(0xc0000009,0)),(0,2,encode(19,0)),
                                                     (0,3,encode(1,0)),(0,4,encode(0,1)),
                                                     (0,5,encode(0x80000000,1))]),
         dict(name='mixed sparse blocks', entries=[(0,8189,encode(235,1)),(1,8191,encode(250,2)),
                                                  (1,40,encode(252,3)),(0,41,encode(0xc0000009,0))])]
for typ in range(1,32):
    cases.append(dict(name=f'type {typ}', entries=[(0,2,encode(235,typ,0xffffffe0))]))
rng = random.Random(903)
for n in range(6):
    positions = rng.sample(range(8192),30)
    cases.append(dict(name=f'mixed random block {n}', entries=[(0,p,encode(rng.randrange(1,4096),rng.randrange(0,8),rng.getrandbits(32))) for p in positions]))

results=[]
for variant in ['Madeira-original','Madeira']:
    binary = (HERE/variant).read_bytes()
    off = file_offset(binary, START, END-START)
    for case in cases:
        u=Uc(UC_ARCH_ARM64,UC_MODE_ARM)
        for addr,size in [(START&~4095,8192),(0x1009e1000,8192),(0x100c1b000,4096),
                          (0x100cd4000,8192),(CACHE,0x20000),(EXTRA,0x20000),(STACK,0x10000),(DONE,4096)]:
            u.mem_map(addr,size)
        u.mem_write(START,binary[off:off+END-START])
        slot=case.get('slot',0)
        count=case.get('count',slot+1)
        u.mem_write(COUNT,struct.pack('<I',count))
        selected=SLOTS+slot*24
        matching=case.get('matching',True)
        active=case.get('active',True)
        null=case.get('null',False)
        u.mem_write(selected,struct.pack('<QQI',PEB if matching else PEB+0x1000,0 if null else CACHE,int(active)))
        entries=case.get('entries',[(0,3,encode(235,1))])
        if case['name']=='empty cache': entries=[]
        blocks={b for b,_,_ in entries}
        assert len(blocks)<=2
        addresses={b:(CACHE+0x400 if b==0 else EXTRA) for b in blocks}
        for block,addr in addresses.items(): u.mem_write(CACHE+8*block,struct.pack('<Q',addr))
        for block,idx,value in entries: u.mem_write(addresses[block]+8*idx,struct.pack('<Q',value))
        sentinel=[0x1234500000000000+i for i in range(31)]
        sentinel[0],sentinel[30]=PEB,DONE
        for reg,val in zip(registers,sentinel): u.reg_write(reg,val)
        sp=STACK+0x8000
        u.reg_write(ar.UC_ARM64_REG_SP,sp)
        events=[]
        def hook(uc,address,size,unused):
            name=HOOKS[address]
            x=[uc.reg_read(reg) for reg in registers[:3]]
            if name=='note':
                assert x[1]==0x100acbec9 and x[2]==PEB
                events.append(('note',x[0]))
            elif name=='close': events.append(('close',x[0]))
            elif name=='free': events.append(('free',x[0]))
            elif name=='log':
                assert x[0]==2
                events.append(('summary',struct.unpack('<QQ',uc.mem_read(uc.reg_read(ar.UC_ARM64_REG_SP),16))))
            else: events.append((name,x[0]))
            lr=uc.reg_read(ar.UC_ARM64_REG_X30)
            # Model the ABI: helper calls may overwrite every caller-saved register.
            for i in range(19): uc.reg_write(registers[i],0xabcd0000+i)
            uc.reg_write(ar.UC_ARM64_REG_X0,0)
            uc.reg_write(ar.UC_ARM64_REG_NZCV,0xa0000000)
            uc.reg_write(ar.UC_ARM64_REG_PC,lr)
        for addr in HOOKS: u.hook_add(UC_HOOK_CODE,hook,begin=addr,end=addr)
        u.emu_start(START,DONE,count=1000000)
        assert u.reg_read(ar.UC_ARM64_REG_PC)==DONE,case['name']
        assert u.reg_read(ar.UC_ARM64_REG_SP)==sp
        for i in range(19,31): assert u.reg_read(registers[i])==sentinel[i],(case['name'],i)
        found=count>0 and active and matching
        released=found and not null
        if found: assert bytes(u.mem_read(selected+8,12))==bytes(12)
        expect=[]
        if released:
            for b,i,value in sorted(entries):
                raw=value&0xffffffff
                typ=(value>>32)&31
                if 0<raw<0x80000000 and (variant=='Madeira-original' or typ!=0):
                    expect.append(raw if variant=='Madeira-original' else raw-1)
        assert [v for k,v in events if k=='close']==expect,(variant,case,events,expect)
        assert [v for k,v in events if k=='note']==expect
        assert [k for k,v in events if k in ['lock','unlock']]==['lock','unlock']
        frees=[addresses[b] for b in sorted(blocks) if b!=0]+[CACHE] if released else []
        assert [v for k,v in events if k=='free']==frees
        assert [v for k,v in events if k=='summary']==([(PEB,len(expect))] if released else [])
        results.append({'variant':variant,'case':case['name'],'closed':expect,'result':'PASS'})
report={'result':'PASS','executed_cases':len(results),'function':'complete ios_fd_cache_release native instructions',
        'original_wrong_fd_reproduced':True,'patched_fd_zero_and_block_edges_pass':True,
        'invalid_cache_entries_skipped':True,'callee_registers_and_stack_preserved':True,
        'limitations':'OS close/free/mutex/logger calls are intercepted; this does not model concurrent guest teardown or run on iPad.',
        'cases':results}
(HERE/'native-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({k:v for k,v in report.items() if k!='cases'},indent=2))
