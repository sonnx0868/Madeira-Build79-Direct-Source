#!/usr/bin/env python3
"""Disable DXMT's optional QueryInterface warning deduper in tracked PE DLLs.

The canonical source fix lives in patches/dxmt-madeira-query-log.patch. Madeira
currently tracks the already-built ARM64EC DXMT PE modules, so cold builds also
apply this deterministic two-instruction patch until PE rebuilding is folded
into bootstrap-native-deps.sh.
"""
from pathlib import Path
import argparse
import hashlib
import os
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
FILES = [
    ROOT / "app/Madeira/arm64ec-windows/d3d11.dll",
    ROOT / "app/Madeira/arm64ec-windows/dxgi.dll",
]
SYMBOL = "_ZN4dxmt22logQueryInterfaceErrorERK5_GUIDS2_"
ORIGINAL_PROLOGUE = bytes.fromhex("ffc300d1f35301a9")
RETURN_FALSE = bytes.fromhex("00008052c0035fd6")  # mov w0,#0; ret


def symbol_offset(data: bytes) -> int:
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("not a PE image")
    section_count = struct.unpack_from("<H", data, pe + 6)[0]
    symbol_table = struct.unpack_from("<I", data, pe + 12)[0]
    symbol_count = struct.unpack_from("<I", data, pe + 16)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    section_table = pe + 24 + optional_size
    sections = []
    for index in range(section_count):
        pos = section_table + index * 40
        sections.append((struct.unpack_from("<I", data, pos + 12)[0],
                         struct.unpack_from("<I", data, pos + 20)[0]))
    strings = symbol_table + symbol_count * 18
    string_size = struct.unpack_from("<I", data, strings)[0]

    found = []
    index = 0
    while index < symbol_count:
        pos = symbol_table + index * 18
        zeroes, name_offset = struct.unpack_from("<II", data, pos)
        value = struct.unpack_from("<I", data, pos + 8)[0]
        section = struct.unpack_from("<h", data, pos + 12)[0]
        aux = data[pos + 17]
        if zeroes == 0 and 4 <= name_offset < string_size:
            end = data.find(b"\0", strings + name_offset, strings + string_size)
            name = data[strings + name_offset:end].decode(errors="replace")
        else:
            name = data[pos:pos + 8].rstrip(b"\0").decode(errors="replace")
        if name == SYMBOL and 1 <= section <= len(sections):
            _virtual, raw = sections[section - 1]
            found.append(raw + value)
        index += 1 + aux
    if len(found) != 1:
        raise ValueError(f"expected one {SYMBOL} symbol, found {len(found)}")
    return found[0]


def process(path: Path, check_only: bool) -> bool:
    data = bytearray(path.read_bytes())
    offset = symbol_offset(data)
    current = bytes(data[offset:offset + len(RETURN_FALSE)])
    if current == RETURN_FALSE:
        print(f"ok   {path.name}: QueryInterface warning deduper disabled at file+0x{offset:x}")
        return True
    if current != ORIGINAL_PROLOGUE:
        print(f"FAIL {path.name}: unexpected function bytes at file+0x{offset:x}: {current.hex()}")
        return False
    if check_only:
        print(f"FAIL {path.name}: still contains the crashing warning deduper")
        return False
    before = hashlib.sha256(data).hexdigest()
    data[offset:offset + len(RETURN_FALSE)] = RETURN_FALSE
    temp = path.with_suffix(path.suffix + ".tmp")
    temp.write_bytes(data)
    os.chmod(temp, path.stat().st_mode)
    os.replace(temp, path)
    after = hashlib.sha256(data).hexdigest()
    print(f"patched {path.name}: file+0x{offset:x} {before} -> {after}")
    return True


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    results = [process(path, args.check) for path in FILES]
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())
