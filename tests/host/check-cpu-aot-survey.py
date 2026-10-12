#!/usr/bin/env python3
"""Malformed PE bounds and readonly/writable AOT candidate classification."""
from pathlib import Path
import importlib.util
import struct
import tempfile

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("aot_survey", root / "tools/cpu-aot-survey.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
data = bytearray(0x800)
data[:2] = b"MZ"; struct.pack_into("<I", data, 60, 0x80)
data[0x80:0x84] = b"PE\0\0"
struct.pack_into("<HH", data, 0x84, 0x8664, 2)
struct.pack_into("<H", data, 0x94, 240)
optional = 0x98
struct.pack_into("<H", data, optional, 0x20b)
struct.pack_into("<I", data, optional + 16, 0x1000)
struct.pack_into("<H", data, optional + 70, 0x40)
struct.pack_into("<I", data, optional + 108, 16)
struct.pack_into("<II", data, optional + 112 + 3 * 8, 0x2000, 12)
for i, (name, va, raw, flags) in enumerate([(b".text", 0x1000, 0x400, 0x60000020), (b".pdata", 0x2000, 0x600, 0x40000040)]):
    off = optional + 240 + i * 40
    data[off:off+len(name)] = name
    struct.pack_into("<IIII", data, off + 8, 0x200, va, 0x200, raw)
    struct.pack_into("<I", data, off + 36, flags)
struct.pack_into("<III", data, 0x600, 0x1000, 0x1020, 0x2080)
with tempfile.TemporaryDirectory(prefix="madeira-aot-survey-") as scratch:
    file = Path(scratch) / "example.exe"
    file.write_bytes(data)
    result = module.survey(file)
    assert result["readonly_function_count"] == 1 and result["readonly_function_bytes"] == 32
    assert result["aslr"] and result["candidate_ranges"][0]["rva"] == 0x1000
    struct.pack_into("<I", data, optional + 240 + 36, 0xe0000020)
    file.write_bytes(data)
    assert module.survey(file)["readonly_function_count"] == 0
    struct.pack_into("<I", data, 0x600, 0x1021)
    file.write_bytes(data)
    try: module.survey(file); raise AssertionError("accepted inverted function range")
    except ValueError: pass
    file.write_bytes(data[:80])
    try: module.survey(file); raise AssertionError("accepted truncated PE")
    except ValueError: pass
    file.write_bytes(b"not an executable")
    try: module.survey(file); raise AssertionError("accepted non-PE")
    except ValueError: pass
print("PASS: readonly/writable candidates, ASLR, invalid unwind ranges and truncated PE headers")
