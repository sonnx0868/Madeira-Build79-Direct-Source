"""Redirect only the inspected R6 guest's setenv import; no runtime interpose.

Apple format: https://github.com/apple-oss-distributions/dyld/blob/main/include/mach-o/fixup-chains.h
Offsets include the universal executable's 0x4000 arm64 slice offset.
The replacement export is deliberately the same length as _setenv.
"""
import hashlib
import struct

BASE_SHA256 = "365071d6fd782642eff115b5ea64a7ad52f7d447cd4ce093f297a4d660b02fb9"
IMPORT_WORD = 12995004
IMPORT_NAME = 13015615
NLIST = 14668912
NLIST_NAME = 15349999
ALIAS = b"_mdrenv"


def patch_resolution_binding(original):
    if hashlib.sha256(original).hexdigest() != BASE_SHA256:
        raise ValueError("Resolution binding requires the exact inspected R6 executable")
    # DYLD_CHAINED_IMPORT: ordinal:8, weak:1, name_offset:23. Preserve
    # everything except ordinal 10 (libSystem) -> 37 (existing UI dylib).
    assert struct.unpack_from("<I", original, IMPORT_WORD)[0] == 0x739e0a
    assert struct.unpack_from("<H", original, NLIST + 6)[0] == 0x0a00
    for offset in (IMPORT_NAME, NLIST_NAME):
        assert original[offset:offset + 8] == b"_setenv\0"
    # Verify load-command order rather than silently trusting an ordinal.
    offset = 0x4000 + 32
    libraries = []
    for _ in range(struct.unpack_from("<I", original, 0x4000 + 16)[0]):
        command, size = struct.unpack_from("<II", original, offset)
        if command in (0xc, 0x80000018, 0x8000001f, 0x80000023):
            name = offset + struct.unpack_from("<I", original, offset + 8)[0]
            libraries.append(original[name:original.index(0, name)])
        offset += size
    assert libraries[9] == b"/usr/lib/libSystem.B.dylib"
    assert libraries[36] == b"@executable_path/Frameworks/MadeiraIPadUI.dylib"
    result = bytearray(original)
    result[IMPORT_WORD] = 37
    result[NLIST + 7] = 37
    for offset in (IMPORT_NAME, NLIST_NAME):
        result[offset:offset + len(ALIAS)] = ALIAS
    allowed = {IMPORT_WORD, NLIST + 7}
    allowed.update(range(IMPORT_NAME, IMPORT_NAME + len(ALIAS)))
    allowed.update(range(NLIST_NAME, NLIST_NAME + len(ALIAS)))
    assert len(result) == len(original)
    assert all(a == b or i in allowed for i, (a, b) in enumerate(zip(original, result)))
    return bytes(result)


if __name__ == "__main__":
    from pathlib import Path
    original = (Path(__file__).resolve().parents[2] / "ipa-inspect/Madeira").read_bytes()
    patched = patch_resolution_binding(original)
    assert patched[IMPORT_WORD] == 37
    assert patched[NLIST + 7] == 37
    for offset in (IMPORT_NAME, NLIST_NAME):
        assert patched[offset:offset + 8] == ALIAS + b"\0"
    for bad in (b"", original[:-1], patched, bytes(len(original))):
        try:
            patch_resolution_binding(bad)
        except ValueError:
            pass
        else:
            raise AssertionError("Uninspected executable accepted")
    print("Resolution import binding: exact scoped changes and bad-input guards PASS")
