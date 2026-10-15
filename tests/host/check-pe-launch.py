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

def function(source, start):
    a=source.index(start); b=source.index("{",a); end=b+1; depth=1
    while depth:
        depth+=(source[end]=="{")-(source[end]=="}");end+=1
    return source[a:end]

bridge=(ROOT/'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
machine=function(bridge,'static uint16_t madeira_pe_machine(')
target=function(bridge,'static uint16_t madeira_target_machine(').replace('NSString *bundle','const char *bundle').replace('bundle.fileSystemRepresentation','bundle')


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
    source.write_text('#include "PELaunch.h"\n#include <unistd.h>\n#include <limits.h>\n'
                      '#define MADEIRA_IMAGE_FILE_MACHINE_I386 0x014c\n'+machine+'\n'+target+
                      '\nint main(int argc, char **argv) { '
                      'if (argc != 5) return 2; printf("%d,%x", madeira_pe_requires_wow64(argv[1]),'
                      'madeira_target_machine(argv[2],argv[3],argv[4])); return 0; }\n')
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
    prefix=tmp/'prefix';drive=prefix/'drive_c';drive.mkdir(parents=True)
    bundle=tmp/'bundle'
    for farm in ('i386-windows','aarch64-windows','arm64ec-windows'):(bundle/farm).mkdir(parents=True)
    def read(path, exe):
        return subprocess.check_output([str(exe_reader),str(path),exe,str(prefix),str(bundle)],text=True)
    exe_reader=exe
    for name, data, expected in cases:
        path = drive / (name + ".exe")
        path.write_bytes(data)
        actual = read(path,'C:\\'+path.name)
        routed=0x14c if expected else {'native-x64':0x8664,'native-arm64':0xaa64}.get(name,0)
        assert actual == f'{expected},{routed:x}', (name, actual, expected, routed)
    assert read(drive/'missing.exe','C:\\missing.exe') == "0,0"
    (bundle/'i386-windows/only32.exe').write_bytes(image())
    (bundle/'i386-windows/both.exe').write_bytes(image())
    (bundle/'aarch64-windows/both.exe').write_bytes(image(machine=0xaa64))
    assert read(drive/'missing.exe','only32.exe')=='0,14c'
    assert read(drive/'missing.exe','both.exe')=='0,0'
print("PASS: native x86/x64/ARM64, AnyCPU, required32, malformed CLR and missing files")
print("PASS: production launch routing preserves AnyCPU with the i386 farm and native 64-bit bare names")
