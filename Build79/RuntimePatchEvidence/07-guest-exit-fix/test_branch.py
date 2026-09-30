"""Execute original/patched native decision instructions; intercept exit targets."""
from pathlib import Path
import hashlib, json, struct
from unicorn import Uc, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
from unicorn import arm64_const as ar
from build import file_offset, SITE, OLD_TARGET, NEW_TARGET

HERE = Path(__file__).resolve().parent
START, END, DATA = SITE-12, SITE+12, 0x20000000
statuses = [0,1,19,255,256,0x103,0xc0000005,0xffffffff]
cases = 0
for variant in ['Madeira-original', 'Madeira']:
    binary = (HERE/variant).read_bytes()
    off = file_offset(binary, START, END-START)
    code = binary[off:off+END-START]
    u = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    u.mem_map(START & ~4095, 4096)
    u.mem_write(START, code)
    u.mem_map(OLD_TARGET & ~4095, 4096)
    u.mem_map(DATA, 4096)
    reached = []
    def hook(uc, address, size, unused):
        if address in (OLD_TARGET, NEW_TARGET):
            reached.append(address)
            uc.emu_stop()
    u.hook_add(UC_HOOK_CODE, hook)
    registers = [getattr(ar, 'UC_ARM64_REG_X'+str(i)) for i in range(31)]
    for exiting in (0,1,2,0xffffffff):
        for status in statuses:
            for flags in range(16):
                initial = [0x1122000000000000+i for i in range(31)]
                initial[20], initial[21] = status, DATA
                for reg, value in zip(registers, initial): u.reg_write(reg, value)
                u.reg_write(ar.UC_ARM64_REG_SP, 0)
                u.reg_write(ar.UC_ARM64_REG_NZCV, flags << 28)
                u.mem_write(DATA, struct.pack('<I', exiting))
                reached.clear()
                u.emu_start(START, 0, count=8)
                expected = NEW_TARGET if exiting or variant == 'Madeira' else OLD_TARGET
                assert reached == [expected], (variant, exiting, status, reached)
                assert u.reg_read(ar.UC_ARM64_REG_X0) == status
                assert u.reg_read(ar.UC_ARM64_REG_X30) == (END if exiting else SITE+4)
                assert u.reg_read(ar.UC_ARM64_REG_SP) == 0
                assert u.reg_read(ar.UC_ARM64_REG_NZCV) == flags << 28
                for i, reg in enumerate(registers):
                    if i not in (0,8,30): assert u.reg_read(reg) == initial[i]
                assert u.mem_read(DATA,4) == struct.pack('<I',exiting)
                cases += 1
report = {'result':'PASS','executed_cases':cases,
          'original_abrupt_path_reaches_abort_process':True,
          'patched_abrupt_path_reaches_existing_exit_process':True,
          'existing_normal_path_preserved':True,'status_and_other_registers_preserved':True,
          'scope':'Final Mach-O branch instructions in Unicorn. Native cleanup body and iPad behavior are not executed by this fixture.'}
(HERE/'branch-test-report.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
