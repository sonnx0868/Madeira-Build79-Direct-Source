from pathlib import Path
from zipfile import ZipFile
import random
import re
import subprocess
import sys
import struct

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE / "source"))
from patch_control_row import patch_control_row, edits, offset, keyboard_code

with ZipFile(HERE.parents[1] / "outputs/Madeira-Light-Mode.ipa") as archive:
    original = archive.read("Payload/Madeira.app/Madeira")
patched = patch_control_row(original)
assert patched != original
for invalid in (b"", original[:-1], patched, bytes(len(original))):
    try:
        patch_control_row(invalid)
    except ValueError:
        pass
    else:
        raise AssertionError("Uninspected input accepted")
for pc, before, after in edits():
    assert patched[offset(pc):offset(pc) + len(after)] == after

# Verify assembled opacity immediate against llvm-mc, rather than trusting
# hand-encoded instructions. Dynamic state selection stays in original code.
opacity = subprocess.check_output(["llvm-mc", "-triple=arm64-apple-ios17.0", "-show-encoding"], input="fmov d0, #0.75\n", text=True)
assert "[0x00,0x10,0x6d,0x1e]" in opacity
assert original[offset(0x1000197d0):offset(0x1000197d8)] == patched[offset(0x1000197d0):offset(0x1000197d8)]

# Keyboard constructor argument/stack equivalence: text selector only, same
# U+2328 icon, two unchanged stack arguments, no extra control/action calls.
code = keyboard_code()
word0, word1, word2, metadata = struct.unpack_from("<4I", code)
scalar = ((word0 >> 5) & 65535) | (((word1 >> 5) & 65535) << 16) | (((word2 >> 5) & 65535) << 32)
assert scalar.to_bytes(8, "little")[:6].decode("utf-8") == "\u2328\ufe0e"
assert (((metadata >> 5) & 65535) << 48) == 0xa600000000000000
assert code[20:28] == bytes.fromhex("08 20 80 52 ff 23 bf a9")
assert code[28:32] == original[offset(0x1000181b0):offset(0x1000181b4)]
def argument_machine(instructions, initial, returned):
    regs, stack, sp, arguments = initial.copy(), bytearray([165] * 32), 16, []
    for word in struct.unpack("<9I", instructions):
        kind = word & 0x7f800000
        if kind in (0x52800000, 0x72800000):  # MOVZ / MOVK, W or X
            index, shift = word & 31, ((word >> 21) & 3) * 16
            value = ((word >> 5) & 65535) << shift
            regs[index] = value if kind == 0x52800000 else (regs[index] & ~(65535 << shift)) | value
            if not word & 0x80000000:
                regs[index] &= 0xffffffff
        elif word & 0xfc000000 == 0x94000000:
            arguments.append((regs[0], regs[1]))
            # Same existing public constructor with arbitrary returned values.
            regs[:9] = returned.copy()
        elif word == 0xd10043ff:
            sp -= 16
        elif word == 0x790013e8:
            stack[sp + 8:sp + 10] = (regs[8] & 65535).to_bytes(2, "little")
        elif word == 0xf90003ff:
            stack[sp:sp + 8] = bytes(8)
        elif word == 0xa9bf23ff:
            sp -= 16
            stack[sp:sp + 16] = bytes(8) + regs[8].to_bytes(8, "little")
        elif word == 0x12000042:
            regs[2] &= 1
        elif word != 0xd503201f:
            raise AssertionError(hex(word))
    return regs, stack, sp, arguments

old_code = original[offset(0x100018190):offset(0x1000181b4)]
for _ in range(1000):
    initial = [random.getrandbits(64) for _ in range(9)]
    returned = [random.getrandbits(64) for _ in range(9)]
    before = argument_machine(old_code, initial, returned)
    after = argument_machine(code, initial, returned)
    assert before[0] == after[0] and before[2] == after[2] == 0
    assert before[1][:10] == after[1][:10]  # argument padding additionally zeroed
    assert len(before[3]) == len(after[3]) == 1
    assert before[3][0] == (0xa88ce2, 0xa300000000000000)
    assert after[3][0] == (0x8eb8efa88ce2, 0xa600000000000000)
print("PASS: exact hash/bytes, scoped colors, assembled opacity, keyboard Unicode/stack arguments, bad-input rejection")
