"""Pin the installed guest-exit baseline and correct its FD-cache decoding."""
from pathlib import Path
import hashlib, json, struct, subprocess, sys, zipfile, difflib

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE.parent/'guest-exit-fix'))
from build import file_offset

BASE = ROOT/'outputs/ipad-installers/Madeira-iPad-GuestExit-Test.ipa'
BASE_SHA = '650326e80abbf8277ce1fbd05c336e74955fbd7e3229dfe127dd26278b107bab'
NATIVE_SHA = 'd809d738999bd1774582440dcd851337a1c3b898a7da799a99d5612dc6b3d85b'
MEMBER = 'Payload/Madeira.app/Madeira'
START, END = 0x1000f907c, 0x1000f90cc
sha = lambda b: hashlib.sha256(b).hexdigest()

def main():
    assert sha(BASE.read_bytes()) == BASE_SHA
    with zipfile.ZipFile(BASE) as z:
        assert len(z.namelist()) == len(set(z.namelist()))
        original = z.read(MEMBER)
    assert sha(original) == NATIVE_SHA
    subprocess.run(['clang', '--target=aarch64-linux-gnu', '-c', str(HERE/'release-loop.S'), '-o', str(HERE/'loop.o')], check=True)
    symbols = {'note_close':0x1000f9128, 'close_fd':0x1009e16b0,
               'free_block':0x1009e1944, 'outer_next':0x1000f9068}
    subprocess.run(['ld.lld', '-m', 'aarch64elf', '-Ttext='+hex(START),
                    '--entry=release_loop', *['--defsym='+k+'='+hex(v) for k,v in symbols.items()],
                    str(HERE/'loop.o'), '-o', str(HERE/'loop.elf')], check=True)
    subprocess.run(['llvm-objcopy', '-O', 'binary', '--only-section=.text', str(HERE/'loop.elf'), str(HERE/'loop.bin')], check=True)
    replacement = (HERE/'loop.bin').read_bytes()
    assert len(replacement) == END-START == 80
    at = file_offset(original, START, len(replacement))
    changed = original[:at]+replacement+original[at+len(replacement):]
    assert len(changed) == len(original)
    assert changed[:at] == original[:at] and changed[at+80:] == original[at+80:]
    for addr, expected in [(0x1000efd98, '9fd60094'), (0x1000f9078, '96ffffb4'), (END, '005900d0')]:
        off = file_offset(changed, addr, 4)
        assert changed[off:off+4] == bytes.fromhex(expected), hex(addr)
    (HERE/'Madeira-original').write_bytes(original)
    (HERE/'Madeira').write_bytes(changed)
    # A source correction accompanies the fixed native instruction loop.
    src = ROOT/'work/beamng/legacy/fd-limit-audit/evidence/build__ntdll-unix__server_ios.c'
    assert sha(src.read_bytes()) == '4fb505447febc237f60a3c28cb4a77d412308aadac6cfc7a4d98cd83f879b5ef'
    before = src.read_text()
    old = '''            if (block[j].s.fd > 0)
            {
                /* ml586: the prime suspect close — a stale cache entry whose fd
                 * number was recycled into another thread's comm pipe */
                ios_fdt_note_close( block[j].s.fd, "fd-cache-release", peb );
                close( block[j].s.fd );
                closed++;
            }'''
    new = '''            if (block[j].s.type != FD_TYPE_INVALID && block[j].s.fd > 0)
            {
                /* Match add/get/remove_fd_from_cache: zero is unset, valid
                 * descriptors are encoded as fd+1, INVALID holds an error. */
                int fd = block[j].s.fd - 1;
                ios_fdt_note_close( fd, "fd-cache-release", peb );
                close( fd );
                closed++;
            }'''
    assert before.count(old) == 1
    after = before.replace(old, new)
    (HERE/'server_ios.fixed.c').write_text(after)
    (HERE/'source.patch').write_text(''.join(difflib.unified_diff(before.splitlines(True), after.splitlines(True),
                                                   fromfile='a/build/ntdll-unix/server_ios.c', tofile='b/build/ntdll-unix/server_ios.c')))
    test_python = ROOT/'work/beamng/fex-runtime-teb-fix/test-venv/bin/python'
    subprocess.run([str(test_python), str(HERE/'test_native.py')], check=True)
    subprocess.run([sys.executable, str(HERE/'test_real_fds.py')], check=True)
    target = ROOT/'outputs/ipad-installers/Madeira-iPad-GuestExit-v2-Test.ipa'
    with zipfile.ZipFile(BASE) as src, zipfile.ZipFile(target, 'w') as dst:
        for info in src.infolist():
            dst.writestr(info, changed if info.filename == MEMBER else src.read(info.filename))
    with zipfile.ZipFile(BASE) as src, zipfile.ZipFile(target) as dst:
        assert src.namelist() == dst.namelist()
        changed_members = [n for n in src.namelist() if src.read(n) != dst.read(n)]
        assert changed_members == [MEMBER]
    report = {'package':str(target), 'sha256':sha(target.read_bytes()), 'baseline_sha256':BASE_SHA,
              'native_original_sha256':NATIVE_SHA, 'native_sha256':sha(changed),
              'range':[hex(START),hex(END)], 'replacement_bytes':80,
              'actual_differing_bytes':sum(a!=b for a,b in zip(original,changed)),
              'changed_members':changed_members, 'prior_guest_exit_fix_preserved':True,
              'all_other_payload_members_identical':True, 'installed':False,
              'device_validated':False, 'cef_fixed':False}
    (HERE/'package-report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report, indent=2))

if __name__ == '__main__': main()
