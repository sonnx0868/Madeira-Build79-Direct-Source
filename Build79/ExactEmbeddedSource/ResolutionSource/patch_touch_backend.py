"""Redirect only six Wine calls in original TouchControlButton, never its UI.

The existing scoped mdrenv import is a dispatcher for resolution and input.
Every original control/editor/gesture instruction is otherwise preserved.
Never patch Wine function bodies, host imports or arbitrary branch sites.
"""
import hashlib
import struct
from patch_resolution_binding import BASE_SHA256, patch_resolution_binding

SLICE = 0x4000
BASE = 0x100000000
STUB = 0x1009e24cc
# callsite -> exact original target. Preserve branch vs branch-and-link.
SITES = {
    0x10002ddf8: 0x100008e7c,  # applyStick old-key releases
    0x10002df60: 0x100008e7c,  # applyStick new-key presses
    0x10002e16c: 0x100008e7c,  # press(key) tail call
    0x10002e208: 0x10000b244,  # press(mouse) tail call
    0x10002e418: 0x100008e7c,  # original onEnded key cleanup
    0x10002e428: 0x10000b244,  # original onEnded mouse cleanup
}


def target(pc, word):
    immediate = word & 0x3ffffff
    if immediate & 0x2000000:
        immediate -= 0x4000000
    return pc + immediate * 4


def patch_touch_backend(original):
    if hashlib.sha256(original).hexdigest() != BASE_SHA256:
        raise ValueError("Touch backend requires exact inspected R6 executable")
    result = bytearray(patch_resolution_binding(original))
    for pc, expected in SITES.items():
        offset = SLICE + pc - BASE
        word = struct.unpack_from("<I", original, offset)[0]
        assert word & 0x7c000000 == 0x14000000
        assert target(pc, word) == expected
        delta = STUB - pc
        assert delta % 4 == 0 and -(1 << 27) <= delta < (1 << 27)
        replacement = (word & 0xfc000000) | ((delta // 4) & 0x3ffffff)
        assert target(pc, replacement) == STUB
        struct.pack_into("<I", result, offset, replacement)
    # ABI bridges: verify original getter prologues/storage use exact symbols.
    for pc, expected in ((0x100026cb0, "00 4e 00 90"), (0x100026cc4, "00 4e 00 90")):
        offset = SLICE + pc - BASE
        assert original[offset:offset + 4] == bytes.fromhex(expected)
    baseline = patch_resolution_binding(original)
    allowed = {SLICE + pc - BASE + i for pc in SITES for i in range(4)}
    assert len(result) == len(original)
    assert all(a == b or i in allowed for i, (a, b) in enumerate(zip(baseline, result)))
    return bytes(result)


if __name__ == "__main__":
    from pathlib import Path
    original = (Path(__file__).resolve().parents[2] / "ipa-inspect/Madeira").read_bytes()
    patched = patch_touch_backend(original)
    # Complete enumeration: no remaining Wine calls inside the control range.
    found = set()
    for pc in range(0x10002dc7c, 0x10002e448, 4):
        word = struct.unpack_from("<I", original, SLICE + pc - BASE)[0]
        if word & 0x7c000000 == 0x14000000 and target(pc, word) in (0x100008e7c, 0x10000b244):
            found.add(pc)
    assert found == set(SITES)
    for bad in (b"", original[:-1], patched):
        try:
            patch_touch_backend(bad)
        except ValueError:
            pass
        else:
            raise AssertionError("Uninspected executable accepted")
    print("PASS: six exact touch input sites redirected; original UI/editor preserved; ABI/hash guards")
