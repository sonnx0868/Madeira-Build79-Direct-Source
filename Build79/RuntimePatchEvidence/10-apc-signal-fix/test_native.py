"""Independent full-function ARM64 tests; Mach helpers are instrumented, not real.

Run with work/beamng/fex-runtime-teb-fix/test-venv/bin/python.
The test never writes the input binaries or modifies the device.
"""
from pathlib import Path
import hashlib
import importlib.util
import itertools
import json
import struct
import subprocess

from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn import arm64_const as ar

HERE = Path(__file__).resolve().parent
BASE = HERE.parent / 'async-cancel-audit/native/Madeira'
CANDIDATE = HERE / 'Madeira'
BASE_SHA = 'f6ef9798133cfd487ee3decc83ec7cb0f762e2952b78b25967820a4efa1ec984'
CANDIDATE_SHA = '1ae878fbc6ed1f2ffcc9febda14ad55bbffdb5a6efbe8c35cd10db6822c67a7b'
START, END = 0x10006c314, 0x10006c400
PATCH_START, PATCH_END = 0x10006c32c, 0x10006c340
GOT_TASK, GOT_STDERR, DEBUG = 0x100bc0e98, 0x100bc0900, 0x101073cf8
FMT = 0x100aba148
R = [getattr(ar, 'UC_ARM64_REG_X' + str(i)) for i in range(31)]
HELPERS = {
    0x1009e1ce0: 'extract', 0x1009e14ac: 'errno',
    0x1009e14e8: 'kill', 0x1009e1cd4: 'dealloc',
    0x1009e1914: 'fprintf',
}
sha = lambda data: hashlib.sha256(data).hexdigest()
spec = importlib.util.spec_from_file_location('review_offsets', HERE.parent / 'guest-exit-fix/build.py')
offsets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(offsets)


def run(code, variant, pid, trace_port, outcome, signal, debug):
    u = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    pages = {START & ~4095, GOT_TASK & ~4095, DEBUG & ~4095,
             *[a & ~4095 for a in HELPERS]}
    for page in pages:
        u.mem_map(page, 4096)
    for page, size in [(0x20000000, 0x10000), (0x30000000, 0x10000), (0x40000000, 4096)]:
        u.mem_map(page, size)
    u.mem_write(START, code)
    w32 = lambda a, v: u.mem_write(a, struct.pack('<I', v & 0xffffffff))
    w64 = lambda a, v: u.mem_write(a, struct.pack('<Q', v))
    r32 = lambda a: struct.unpack('<I', u.mem_read(a, 4))[0]
    r64 = lambda a: struct.unpack('<Q', u.mem_read(a, 8))[0]
    thread, process, task_slot, errno_slot = 0x20000000, 0x20001000, 0x20002000, 0x20003000
    stderr_slot, stderr_value = 0x20004000, 0x20005000
    w64(thread + 0x88, process)
    w32(process + 0x198, trace_port)
    w32(thread + 0x1f8, pid)
    w32(thread + 0x1fc, 0x987)
    w32(thread + 0x90, 0x456)
    w64(GOT_TASK, task_slot)
    w32(task_slot, 0x543)
    w64(GOT_STDERR, stderr_slot)
    w64(stderr_slot, stderr_value)
    w32(DEBUG, debug)
    w32(errno_slot, 0)
    sentinels = [0x1234500000000000 + i for i in range(31)]
    sentinels[0], sentinels[1], sentinels[30] = thread, signal, 0x40000000
    for reg, value in zip(R, sentinels):
        u.reg_write(reg, value)
    sp = 0x30008000
    u.reg_write(ar.UC_ARM64_REG_SP, sp)
    events = []

    def hook(u, address, size, unused):
        if address not in HELPERS:
            return
        name = HELPERS[address]
        args = [u.reg_read(reg) for reg in R[:5]]
        lr = u.reg_read(R[30])
        helper_sp = u.reg_read(ar.UC_ARM64_REG_SP)
        assert helper_sp % 16 == 0
        events.append(name)
        value = 0
        if name == 'extract':
            assert args[:3] == [0x543, 0x987, 19]
            assert args[3:] == [sp - 0x50 + 0x18, sp - 0x50 + 0x1c]
            w32(args[3], 0x111)
            w32(args[4], 17)
            value = 5 if outcome == 'extract' else 0
        elif name == 'kill':
            assert args[:2] == [0x111, signal]
            if outcome.startswith('kill'):
                value = 0xffffffffffffffff
                w32(errno_slot, 3 if outcome == 'kill_esrch' else 22)
        elif name == 'dealloc':
            assert args[:2] == [0x543, 0x111]
        elif name == 'errno':
            value = errno_slot
        elif name == 'fprintf':
            assert args[:2] == [stderr_value, FMT]
            # Apple ARM64 variadic arguments are stack-passed, as in this body.
            assert r64(helper_sp) == 0x456
            assert r64(helper_sp + 8) == signal
        else:
            raise AssertionError(name)
        for i in range(18):
            u.reg_write(R[i], 0xabcd0000 + i)
        u.reg_write(R[0], value)
        u.reg_write(ar.UC_ARM64_REG_NZCV, 0xa0000000)
        u.reg_write(ar.UC_ARM64_REG_PC, lr)

    u.hook_add(UC_HOOK_CODE, hook)
    u.emu_start(START, 0x40000000, count=10000)
    reached = pid != -1 and (variant == 'candidate' or trace_port != 0)
    success = reached and outcome == 'none'
    assert u.reg_read(ar.UC_ARM64_REG_PC) == 0x40000000
    assert u.reg_read(R[0]) == int(success)
    assert events.count('extract') == int(reached)
    assert events.count('kill') == int(reached and outcome != 'extract')
    assert events.count('dealloc') == int(reached and outcome != 'extract')
    assert events.count('fprintf') == int(success and bool(debug))
    invalidated = reached and outcome in ('extract', 'kill_esrch')
    assert r32(thread + 0x1f8) == (0xffffffff if pid == -1 or invalidated else pid)
    assert r32(thread + 0x1fc) == (0xffffffff if invalidated else 0x987)
    assert r32(process + 0x198) == trace_port
    assert u.reg_read(ar.UC_ARM64_REG_SP) == sp
    for i in range(18, 31):
        assert u.reg_read(R[i]) == sentinels[i], ('ABI', i)
    return {
        'variant': variant, 'pid': pid, 'trace_port': trace_port,
        'outcome': outcome, 'signal': signal, 'debug': debug,
        'result': int(success), 'helpers': events, 'pass': True,
    }


def main():
    baseline = BASE.read_bytes()
    assert sha(baseline) == BASE_SHA
    off = offsets.file_offset(baseline, START, END - START)
    original = baseline[off:off + END - START]
    expected = bytearray(original)
    # Reuse the exact task-self load later in the same instruction page,
    # followed by the original PID load/test. No assembler or file mutation.
    expected[0x18:0x2c] = original[0x98:0xa4] + original[0x20:0x28]
    assert expected[0x18:0x2c].hex() == 'a85a0090084d47f9000140b968fa41b91f050031'
    candidate = CANDIDATE.read_bytes()
    assert sha(candidate) == CANDIDATE_SHA
    assert len(candidate) == len(baseline)
    at = offsets.file_offset(baseline, PATCH_START, PATCH_END - PATCH_START)
    assert at == 0x7032c
    assert candidate[:at] == baseline[:at]
    assert candidate[at + 20:] == baseline[at + 20:]
    assert candidate[off:off + len(expected)] == expected
    assert sum(a != b for a, b in zip(baseline[at:at + 20], candidate[at:at + 20])) == 19
    unwind = subprocess.check_output(['llvm-objdump', '--macho', '--unwind-info', str(CANDIDATE)], text=True)
    assert 'function offset=0x0006c314, encoding[2]=0x04000003' in unwind
    assert 'function offset=0x0006c400' in unwind
    cases = []
    for variant, code in [('original', original), ('candidate', bytes(expected))]:
        for parameters in itertools.product([-1, 812], [0, 0x543],
                                            ['none', 'extract', 'kill_esrch', 'kill_einval'],
                                            [30, 3], [0, 1]):
            cases.append(run(code, variant, *parameters))
    assert len(cases) == 128
    report = {
        'test_sha256': sha(Path(__file__).read_bytes()),
        'baseline_sha256': BASE_SHA, 'candidate_sha256': CANDIDATE_SHA,
        'total': len(cases), 'per_variant': 64, 'debug_zero_cases': 64,
        'debug_one_cases': 64, 'all_passed': all(c['pass'] for c in cases),
        'passed': all(c['pass'] for c in cases),
        'outside_20_byte_site_identical': True,
        'compact_unwind': '0x04000003',
        'limitations': ['Mach and libc helpers are instrumented.',
                        'Does not execute APC queue or client callback code.',
                        'Does not prove iOS signal delivery or CEF recovery.'],
        'cases': cases,
    }
    (HERE / 'native-test-report.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'cases'}, indent=2))


if __name__ == '__main__':
    main()
