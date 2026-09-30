"""Exact build-72-only appearance patch; original control actions untouched.

SwiftUI Color.white and Color.primary getters have the identical public Swift
signature () -> Color. Retarget only audited row color calls, not imports.
Keyboard text keeps U+2328 and adds U+FE0E for monochrome text presentation.
"""
import hashlib
import re
import struct
import subprocess
from pathlib import Path

EXPECTED = "bcd28d90560e1d2d39a152e9ee4939e9fc8b4f3af1c84b14f426491220d6158e"
SLICE, BASE = 0x4000, 0x100000000
WHITE, PRIMARY = 0x1009dfe68, 0x1009dfea4
COLOR_SITES = (0x10001976c, 0x10001aa74, 0x10001ab14)

def offset(pc):
    return SLICE + pc - BASE

def branch(pc, destination):
    delta = destination - pc
    assert delta % 4 == 0 and -(1 << 27) <= delta < (1 << 27)
    return struct.pack("<I", 0x94000000 | ((delta // 4) & 0x3ffffff))

def keyboard_code():
    assembly = Path(__file__).with_name("row_keyboard_text.s")
    listing = subprocess.check_output(["llvm-mc", "-triple=arm64-apple-ios17.0", "-show-encoding", str(assembly)], text=True)
    rows = re.findall(r"encoding: \[([^]]+)\]", listing)
    data = bytearray(b"".join(bytes(int(v.strip(), 16) for v in row.split(",")) for row in rows))
    assert len(data) == 36
    data[16:20] = branch(0x1000181a0, 0x1009dfaa8)
    return bytes(data)

def edits():
    result = [(pc, branch(pc, WHITE), branch(pc, PRIMARY)) for pc in COLOR_SITES]
    # Existing diagnostics off-opacity 0.35 -> 0.75, active remains 1.0.
    # Replace literal construction with fmov d0,#0.75; leave fcsel untouched.
    result.append((0x1000197c4, bytes.fromhex("e8 e7 03 b2 c8 fa e7 f2 00 01 67 9e"),
                   bytes.fromhex("00 10 6d 1e 1f 20 03 d5 1f 20 03 d5")))
    old_keyboard = bytes.fromhex(
        "40 9c 91 52 00 15 a0 72 01 60 f4 d2 43 1e 27 94 "
        "ff 43 00 d1 08 20 80 52 e8 13 00 79 ff 03 00 f9 42 00 00 12")
    result.append((0x100018190, old_keyboard, keyboard_code()))
    return result

def patch_control_row(original):
    if hashlib.sha256(original).hexdigest() != EXPECTED:
        raise ValueError("Control row patch requires exact inspected build-72 executable")
    result = bytearray(original)
    allowed = set()
    for pc, before, after in edits():
        start = offset(pc)
        assert len(before) == len(after) and original[start:start + len(before)] == before, hex(pc)
        result[start:start + len(after)] = after
        allowed.update(range(start, start + len(before)))
    assert len(result) == len(original)
    assert all(a == b or i in allowed for i, (a, b) in enumerate(zip(original, result)))
    return bytes(result)
