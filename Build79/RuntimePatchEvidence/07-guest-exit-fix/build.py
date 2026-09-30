"""Redirect only NtTerminateProcess's abrupt self-exit to existing iOS cleanup."""
from pathlib import Path
import hashlib, json, struct, subprocess, zipfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
BASELINE = ROOT / 'outputs/ipad-installers/Madeira-iPad-CPU-ModuleTLS-v3-Test.ipa'
BASE_SHA = 'ef68c5d683eee70e8bba080d948b00b051c8ba7b5119e930165db2125eb210f3'
NATIVE_SHA = '607600d9156fa454a0b70b04e79af97b1989fbcafc1c42a68cfab0827fbbf1bf'
MEMBER = 'Payload/Madeira.app/Madeira'
SITE, OLD_TARGET, NEW_TARGET = 0x1000efd98, 0x1001255e4, 0x100125814

def sha(data): return hashlib.sha256(data).hexdigest()

def file_offset(data, address, length):
    # The signed baseline keeps a one-architecture FAT wrapper.
    assert struct.unpack_from('>II', data) == (0xcafebabe, 1)
    cpu, subtype, slice_at, slice_size, alignment = struct.unpack_from('>IIIII', data, 8)
    assert cpu == 0x100000c and subtype == 0 and alignment == 14
    assert slice_at == 0x4000 and slice_at + slice_size <= len(data)
    assert struct.unpack_from('<II', data, slice_at) == (0xfeedfacf, 0x100000c)
    count = struct.unpack_from('<I', data, slice_at+16)[0]
    p = slice_at+32
    for _ in range(count):
        command, size = struct.unpack_from('<II', data, p)
        assert size >= 8 and p + size <= len(data)
        if command == 0x19:
            vm, vs, off, fs = struct.unpack_from('<QQQQ', data, p + 24)
            if vm <= address and address + length <= vm + fs:
                assert off + address - vm + length <= slice_size
                return slice_at + off + address - vm
        p += size
    raise ValueError('Unmapped code address')

def call(target):
    delta = target - SITE
    assert delta % 4 == 0 and -(1 << 27) <= delta < (1 << 27)
    return struct.pack('<I', 0x94000000 | ((delta // 4) & 0x3ffffff))

def main():
    assert sha(BASELINE.read_bytes()) == BASE_SHA
    with zipfile.ZipFile(BASELINE) as z:
        assert len(z.namelist()) == len(set(z.namelist()))
        original = z.read(MEMBER)
    assert sha(original) == NATIVE_SHA
    at = file_offset(original, SITE, 4)
    assert original[at:at+4] == call(OLD_TARGET) == bytes.fromhex('13d60094')
    # Pin both the existing guard and the adjacent normal-exit branch.
    assert original[at-12:at+12] == bytes.fromhex('a80240b968000035e00314aa13d60094e00314aa9dd60094')
    changed = original[:at] + call(NEW_TARGET) + original[at+4:]
    assert len(original) == len(changed)
    assert all(original[i] == changed[i] for i in range(len(original)) if not at <= i < at+4)
    (HERE / 'Madeira-original').write_bytes(original)
    (HERE / 'Madeira').write_bytes(changed)
    test_python = ROOT / 'work/beamng/fex-runtime-teb-fix/test-venv/bin/python'
    subprocess.run([str(test_python), str(HERE/'test_branch.py')], check=True)
    target = ROOT / 'outputs/ipad-installers/Madeira-iPad-GuestExit-Test.ipa'
    with zipfile.ZipFile(BASELINE) as src, zipfile.ZipFile(target, 'w') as dst:
        for info in src.infolist():
            dst.writestr(info, changed if info.filename == MEMBER else src.read(info.filename))
    with zipfile.ZipFile(BASELINE) as src, zipfile.ZipFile(target) as dst:
        assert src.namelist() == dst.namelist()
        changed_members = [n for n in src.namelist() if src.read(n) != dst.read(n)]
        assert changed_members == [MEMBER]
    report = {'package': str(target), 'sha256': sha(target.read_bytes()),
              'baseline_sha256': BASE_SHA, 'native_original_sha256': NATIVE_SHA,
              'native_sha256': sha(changed), 'site': hex(SITE),
              'old_target': hex(OLD_TARGET), 'new_target': hex(NEW_TARGET),
              'original_instruction': call(OLD_TARGET).hex(),
              'replacement_instruction': call(NEW_TARGET).hex(),
              'changed_members': changed_members, 'changed_instruction_bytes': 4,
              'actual_differing_bytes': sum(a != b for a,b in zip(original, changed)),
              'preserved_other_payload_members': True, 'installed': False,
              'device_validated': False, 'cef_fixed': False}
    (HERE/'package-report.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report, indent=2))

if __name__ == '__main__': main()
