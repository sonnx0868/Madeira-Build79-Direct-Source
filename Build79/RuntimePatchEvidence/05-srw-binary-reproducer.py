"""Local-only: disable only speculative anonymous-SRW mutation, preserving diagnostics."""
from pathlib import Path
import hashlib, importlib.util, json, difflib, subprocess

H = Path(__file__).resolve().parent
BASE = H.parent / 'decommit-edge-fix/native/Madeira'
SOURCE = H.parents[1] / 'vulkan/build-audit/wine-source/dlls/ntdll/unix/sync.c'
BASE_SHA = '6e09addbcea3478bb2fb4c6989703351849f0aff160fe3d4c49eb8e3ed5697a8'
SOURCE_SHA = '1362b85e0cd1b3d09bff1406a354cbd57755c299b8db0ff1575587baaa0c1976'
SITE = 0x10011c728
sha = lambda data: hashlib.sha256(data).hexdigest()

def main():
    spec = importlib.util.spec_from_file_location('offsets', H.parent/'guest-exit-fix/build.py')
    offsets = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(offsets)
    baseline = BASE.read_bytes()
    assert sha(baseline) == BASE_SHA
    at = offsets.file_offset(baseline, SITE, 4)
    assert at == 0x120728
    old, new = bytes.fromhex('d6fcff97'), bytes.fromhex('1f2003d5')
    assert baseline[at:at+4] == old
    assert baseline[at-8:at+12] == bytes.fromhex('e00317aaa1d59b52d6fcff979f0200f9effeff17')
    explicit_at = offsets.file_offset(baseline, 0x1000fd418, 4)
    assert baseline[explicit_at:explicit_at+4] == bytes.fromhex('9a790094')
    candidate = baseline[:at] + new + baseline[at+4:]
    delta = [i for i, (a,b) in enumerate(zip(baseline,candidate)) if a != b]
    assert len(candidate) == len(baseline) and delta == list(range(at, at+4))
    (H/'Madeira-original').write_bytes(baseline)
    (H/'Madeira').write_bytes(candidate)
    source_bytes = SOURCE.read_bytes()
    assert sha(source_bytes) == SOURCE_SHA
    original = source_bytes.decode()
    needle = '            ios_srw_reap_exclusive( lock, 0xDEADull );'
    assert original.count(needle) == 1
    replacement = ('            /* A missing FEX stamp does not establish the owner of an anonymous\n'
                   '             * SRW is dead. Keep this heuristic diagnostic-only; leave the\n'
                   '             * real owner responsible for release and waiter notification. */')
    fixed = original.replace(needle, replacement)
    (H/'sync.original.c').write_bytes(source_bytes)
    (H/'sync.fixed.c').write_text(fixed)
    (H/'source.patch').write_text(''.join(difflib.unified_diff(
        original.splitlines(True), fixed.splitlines(True),
        fromfile='a/dlls/ntdll/unix/sync.c', tofile='b/dlls/ntdll/unix/sync.c')))
    before = subprocess.check_output(['llvm-objdump','--macho','--unwind-info',str(H/'Madeira-original')],text=True).splitlines()[1:]
    after = subprocess.check_output(['llvm-objdump','--macho','--unwind-info',str(H/'Madeira')],text=True).splitlines()[1:]
    assert before == after
    report = {
        'baseline':str(BASE), 'baseline_sha256':sha(baseline),
        'candidate_sha256':sha(candidate), 'length':len(candidate),
        'site':hex(SITE), 'file_offset':hex(at), 'old':old.hex(), 'new':new.hex(),
        'changed_offsets':[hex(x) for x in delta], 'exact_four_byte_change':True,
        'unwind_unchanged':True, 'explicit_dead_owner_call_unchanged':True,
        'source':str(SOURCE), 'source_sha256':sha(source_bytes),
        'fixed_source_sha256':sha(fixed.encode()),
        'scope':'Suppress only anonymous missing-stamp SRW mutation/wake. Keep diagnostics, metadata reset, explicit-dead-owner path, frame, and all other code.',
        'packaged':False, 'installed':False, 'physical_fix_proven':False,
    }
    (H/'build-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

if __name__ == '__main__':
    main()
