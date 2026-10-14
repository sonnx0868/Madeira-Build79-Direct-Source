#!/usr/bin/env python3
"""Exercise the production PE launch reader against native and managed images."""
from pathlib import Path
import os
import shutil
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
CC = os.environ.get("CC") or shutil.which("clang") or shutil.which("cc")
if not CC:
    raise SystemExit("A C compiler is required")


def image(machine=0x14c, flags=None):
    data = bytearray(1024)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 60, 128)
    data[128:132] = b"PE\0\0"
    struct.pack_into("<HH", data, 132, machine, 1)
    struct.pack_into("<H", data, 148, 224)
    struct.pack_into("<H", data, 152, 0x10b)
    struct.pack_into("<I", data, 152 + 92, 16)
    struct.pack_into("<IIII", data, 376 + 8, 512, 0x2000, 512, 512)
    if flags is not None:
        struct.pack_into("<II", data, 152 + 96 + 14 * 8, 0x2040, 72)
        struct.pack_into("<I", data, 576, 72)
        struct.pack_into("<I", data, 592, flags)
    return data


with tempfile.TemporaryDirectory(prefix="madeira-pe-launch-") as tmp:
    tmp = Path(tmp)
    source = tmp / "reader.c"
    source.write_text('#include "PELaunch.h"\nint main(int argc, char **argv) { '
                      'if (argc != 2) return 2; printf("%d", madeira_pe_requires_wow64(argv[1])); return 0; }\n')
    exe = tmp / ("reader.exe" if os.name == "nt" else "reader")
    subprocess.run([CC, "-std=c11", "-Wall", "-Wextra", "-Werror", "-I",
                    str(ROOT / "app/Madeira"), str(source), "-o", str(exe)], check=True)
    malformed = image(flags=1)
    struct.pack_into("<I", malformed, 376 + 20, 0xfffffff0)
    truncated = image(flags=1)[:585]
    excessive = image(flags=1)
    struct.pack_into("<H", excessive, 134, 65535)
    cases = [
        ("native-x86", image(), 1), ("native-x64", image(machine=0x8664), 0),
        ("native-arm64", image(machine=0xaa64), 0),
        ("anycpu", image(flags=1), 0), ("anycpu-prefer32", image(flags=0x20001), 0),
        ("celeste-required32", image(flags=3), 1), ("mixed-clr", image(flags=0), 1),
        ("bad-clr-rva", malformed, 1), ("truncated-clr", truncated, 1),
        ("excessive-sections", excessive, 1), ("not-pe", b"MZ", 0),
    ]
    for name, data, expected in cases:
        path = tmp / (name + ".exe")
        path.write_bytes(data)
        actual = subprocess.check_output([str(exe), str(path)], text=True)
        assert actual == str(expected), (name, actual, expected)
    assert subprocess.check_output([str(exe), str(tmp / "missing.exe")], text=True) == "0"
print("PASS: native x86/x64/ARM64, AnyCPU, required32, malformed CLR and missing files")
