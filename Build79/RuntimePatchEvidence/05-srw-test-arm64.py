"""Focused whole-function ARM64 regression for the exact SRW orphan candidate.

Run with ../fex-runtime-teb-fix/test-venv/bin/python test_native.py.
No device access. Only msync and dprintf are mocked. Baseline executes the
unchanged native reaper, including its actual atomic lock-word mutation;
the native wake bucket is mapped empty, so no real thread is awakened.
"""
from pathlib import Path
import hashlib, importlib.util, json, struct
from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE, UC_PROT_READ, UC_PROT_EXEC
from unicorn.arm64_const import *

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('macho_mapping', HERE.parent/'guest-exit-fix/build.py')
mapping = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mapping)
sha = lambda b: hashlib.sha256(b).hexdigest()
X = [globals()['UC_ARM64_REG_X'+str(i)] for i in range(31)]
Q = [globals()['UC_ARM64_REG_Q'+str(i)] for i in range(32)]
START, END = 0x10011c290, 0x10011c7c8
REAPER, REAPER_END = 0x10011ba80, 0x10011bcc0
SITE, DEAD_CALL = 0x10011c728, 0x1000fd418
MSYNC, PRINT = 0x1009e1e90, 0x1009e17f4
WAITERS, SUSP = 0x100d010d0, 0x100d050d8
LOCK, STAMPS = 0x703f913050, 0x20000000
STACK, SP, DONE = 0x30000000, 0x30009000, 0x40000000
EXPECTED_BASE = '6e09addbcea3478bb2fb4c6989703351849f0aff160fe3d4c49eb8e3ed5697a8'
EXPECTED_CANDIDATE = '91f4bc3acb90ed4dafa7fbf7cea5087f8c32ae9044990a6f785b116567e43d90'

def native(data, va, size):
    off = mapping.file_offset(data, va, size)
    return data[off:off+size]

def bl_target(data, va):
    op = struct.unpack('<I', native(data, va, 4))[0]
    assert op >> 26 == 0b100101
    imm = op & 0x3ffffff
    if imm & 0x2000000: imm -= 0x4000000
    return va + imm*4

def run(data, case, candidate):
    u = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    # Map only the two actual functions, with trap padding outside them.
    u.mem_map(0x10011b000, 0x2000)
    u.mem_write(0x10011b000, bytes.fromhex('000020d4')*0x800)
    u.mem_write(START, native(data, START, END-START))
    u.mem_write(REAPER, native(data, REAPER, REAPER_END-REAPER))
    u.mem_protect(0x10011b000, 0x2000, UC_PROT_READ|UC_PROT_EXEC)
    u.mem_map(0x1009e1000, 0x1000)
    u.mem_map(0x100ad6000, 0x1000)
    u.mem_write(0x100ad6000, native(data, 0x100ad6000, 0x1000))
    u.mem_map(0x100cff000, 0x7000)  # real BSS addresses, including empty wake buckets
    u.mem_map(LOCK & ~0x3fff, 0x4000)
    u.mem_map(STAMPS, 0x1000)
    u.mem_map(STACK, 0x10000)
    u.mem_map(DONE, 0x1000)
    u.mem_write(STACK, b'\xa5'*0x10000)
    u.mem_write(LOCK-8, b'PRELOCK!')
    u.mem_write(LOCK+4, b'POSTLOCK')
    word = case.get('word', 0x10009)
    u.mem_write(LOCK, struct.pack('<I', word))
    slots = case.get('slots', list(range(case.get('waiters', 4))))
    for n, slot in enumerate(slots):
        assert 0 <= slot < 512
        u.mem_write(WAITERS+slot*32, struct.pack('<QQQII', 0x80+n*4, LOCK+2, 42, 1, 0))
    stamps = case.get('stamps', [])
    u.mem_write(STAMPS, b''.join(struct.pack('<Q', s) for s in stamps))
    if case.get('suspicion'):
        # Last slot exercises the unrolled clear/search tail, not just slot zero.
        u.mem_write(SUSP+7*16, struct.pack('<QI4x', LOCK, 2))
    waiter_snapshot = bytes(u.mem_read(WAITERS, 512*32))
    stamp_snapshot = bytes(u.mem_read(STAMPS, 0x1000))
    regs = {r: 0xab00000000000000+i*0x10101 for i,r in enumerate(X)}
    for r, v in regs.items(): u.reg_write(r,v)
    qregs = {r: 0xdeadbeef000000001234000000000000+i for i,r in enumerate(Q)}
    for r,v in qregs.items(): u.reg_write(r,v)
    u.reg_write(X[0],STAMPS)
    u.reg_write(X[1],len(stamps))
    u.reg_write(X[30],DONE)
    u.reg_write(UC_ARM64_REG_SP,SP)
    calls, logs, reaps, count = [], [], [], 0
    min_sp = SP

    def returned(value):
        lr = u.reg_read(X[30])
        # Deliberately make helper clobbers hostile to any unadvertised ABI reliance.
        for i in range(18): u.reg_write(X[i],0xcca0000000000000+i)
        for i in list(range(8))+list(range(16,32)):
            u.reg_write(Q[i],0xfa110000000000000000000000000000+i)
        u.reg_write(UC_ARM64_REG_NZCV,0xf0000000)
        u.reg_write(X[0],value & ((1<<64)-1))
        u.reg_write(UC_ARM64_REG_PC,lr)

    def hook(_u, pc, size, _):
        nonlocal count, min_sp
        count += 1
        cursp = u.reg_read(UC_ARM64_REG_SP)
        min_sp = min(min_sp,cursp)
        assert cursp % 16 == 0
        if pc == DONE:
            u.emu_stop(); return
        if pc == MSYNC:
            args = tuple(u.reg_read(X[i]) for i in range(3))
            assert args == (LOCK & ~0x3fff,0x4000,1), args
            calls.append('msync')
            returned(-1 if case.get('msync_fail') else 0)
        elif pc == PRINT:
            assert u.reg_read(X[0]) == 2
            fmt = u.reg_read(X[1])
            args = list(struct.unpack('<QQQQ',u.mem_read(cursp,32)))
            if fmt == 0x100ad68e3:
                assert args[0:3] == [LOCK,word,len(slots)], args
                logs.append({'kind':'strike','strike':args[3]})
            elif fmt == 0x100ad688c:
                assert args[:3] == [LOCK,word,len(slots)],args
                logs.append({'kind':'incoherent'})
            elif fmt == 0x100ad6749:
                assert args == [LOCK,0xdead,word,word & 0xfffe],args
                assert struct.unpack('<I',u.mem_read(LOCK,4))[0] == word & 0xfffe
                logs.append({'kind':'reap','before':args[2],'after':args[3]})
            else: raise AssertionError(hex(fmt))
            returned(73)
        elif pc == REAPER:
            args = (u.reg_read(X[0]),u.reg_read(X[1]))
            assert args == (LOCK,0xdead),args
            reaps.append(args)
        else:
            assert START <= pc < END or REAPER <= pc < REAPER_END,hex(pc)

    u.hook_add(UC_HOOK_CODE,hook)
    u.emu_start(START,DONE+4,count=200000)
    assert u.reg_read(UC_ARM64_REG_PC) == DONE,hex(u.reg_read(UC_ARM64_REG_PC))
    assert u.reg_read(UC_ARM64_REG_SP) == SP
    assert u.reg_read(X[30]) == DONE
    for i in range(18,30): assert u.reg_read(X[i]) == regs[X[i]],'x'+str(i)
    for i in range(8,16):
        assert u.reg_read(Q[i]) & ((1<<64)-1) == qregs[Q[i]] & ((1<<64)-1),'d'+str(i)
    assert bytes(u.mem_read(STACK,0x8000)) == b'\xa5'*0x8000
    assert bytes(u.mem_read(SP,0x7000)) == b'\xa5'*0x7000
    assert bytes(u.mem_read(LOCK-8,8)) == b'PRELOCK!'
    assert bytes(u.mem_read(LOCK+4,8)) == b'POSTLOCK'
    assert bytes(u.mem_read(WAITERS,512*32)) == waiter_snapshot
    assert bytes(u.mem_read(STAMPS,0x1000)) == stamp_snapshot
    eligible = case.get('eligible',False)
    expect_reap = eligible and not candidate
    after = struct.unpack('<I',u.mem_read(LOCK,4))[0]
    assert len(reaps) == int(expect_reap),(case['name'],reaps)
    assert after == (word & 0xfffe if expect_reap else word),(case['name'],hex(after))
    strikes = [v['strike'] for v in logs if v['kind']=='strike']
    if eligible:
        expected = [1,2,3] if expect_reap else [(i%3)+1 for i in range(len(slots))]
        assert strikes == expected,(strikes,expected)
    else: assert not strikes,strikes
    if case.get('suspicion'):
        assert bytes(u.mem_read(SUSP+7*16,12)) == b'\0'*12
    assert min_sp == SP-(0x1f0 if expect_reap else 0xa0)
    return {'case':case['name'],'binary':'candidate' if candidate else 'baseline',
            'passed':True,'instructions':count,'word_before':f'{word:08x}',
            'word_after':f'{after:08x}','reaper_calls':len(reaps),'strikes':strikes,
            'msync_calls':len(calls),'stack_used':SP-min_sp,'abi_preserved':True}

def main():
    baseline=(HERE/'Madeira-original').read_bytes()
    candidate=(HERE/'Madeira').read_bytes()
    assert sha(baseline)==EXPECTED_BASE
    assert sha(candidate)==EXPECTED_CANDIDATE
    at=mapping.file_offset(baseline,SITE,4)
    assert at==0x120728
    assert native(baseline,SITE,4).hex()=='d6fcff97'
    assert native(candidate,SITE,4).hex()=='1f2003d5'
    assert len(baseline)==len(candidate)
    diff=[i for i,(a,b) in enumerate(zip(baseline,candidate)) if a!=b]
    assert diff==list(range(at,at+4))
    assert bl_target(baseline,SITE)==REAPER
    assert native(baseline,DEAD_CALL,4)==native(candidate,DEAD_CALL,4)
    assert bl_target(candidate,DEAD_CALL)==REAPER
    # STR zero; B back to clearing strikes: neither consumes x0, x1, NZCV, or LR.
    assert native(candidate,SITE+4,8).hex()=='9f0200f9effeff17'
    cases=[
        {'name':'live-unstamped-three-in-one-census','waiters':3,'word':0x10007,'eligible':True},
        {'name':'physical-four-waiter-word','waiters':4,'eligible':True},
        {'name':'five-waiters-diagnostic-retained','waiters':5,'word':0x1000b,'eligible':True},
        {'name':'stamped-scalar-clears-suspicion','stamps':[LOCK],'suspicion':True},
        {'name':'stamped-vector-clears-suspicion','stamps':[0]*7+[LOCK],'suspicion':True},
        {'name':'stamped-vector-tail-clears-suspicion','stamps':[0]*8+[LOCK],'suspicion':True},
        {'name':'released-clears-suspicion','word':8,'suspicion':True},
        {'name':'incoherent-two-owners-clears-suspicion','word':0x20009,'suspicion':True},
        {'name':'held-but-no-exclusive-queue','word':0x10001,'suspicion':True},
        {'name':'fewer-than-three-waiters','waiters':2},
        {'name':'unmapped-msync-rejection','msync_fail':True},
        {'name':'last-waiter-slots-scalar-count-tail','slots':[509,510,511],'word':0x10007,'eligible':True},
    ]
    results=[run(data,c,is_candidate) for c in cases for data,is_candidate in [(baseline,False),(candidate,True)]]
    report={'result':'PASS','passed':True,'candidate_sha256':sha(candidate),
        'baseline_sha256':sha(baseline),'test_script_sha256':sha(Path(__file__).read_bytes()),
        'whole_function':{'start':hex(START),'end_exclusive':hex(END)},
        'native_reaper':{'start':hex(REAPER),'end_exclusive':hex(REAPER_END),'executed_in_negative_controls':True},
        'differing_file_offsets':[hex(i) for i in diff],'all_other_bytes_identical':True,
        'explicit_dead_owner_call':{'address':hex(DEAD_CALL),'instruction':native(candidate,DEAD_CALL,4).hex(),'target':hex(REAPER),'unchanged':True},
        'unchanged_unwind_and_function_frames':True,'call_result_unused':True,
        'distinct_cases':len(cases),'executions':len(results),'results':results,
        'limitations':['Unicorn is a sequential ARM64 model, not physical iPad execution or concurrent memory-order validation.',
          'msync returns configured status and dprintf captures arguments; both aggressively clobber caller-saved state.',
          'Native reaper executes against an empty wake bucket; no OS waiter scheduling or dead-thread discovery is tested.',
          'The separate explicit-dead-owner call is verified byte-for-byte, not executed or certified as a sound death detector.',
          'The fix removes the unsafe anonymous-owner mutation; existing strike diagnostic counting and other possible freezes remain.']}
    (HERE/'native-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:report[k] for k in ['result','candidate_sha256','distinct_cases','executions']},indent=2))

if __name__=='__main__': main()
