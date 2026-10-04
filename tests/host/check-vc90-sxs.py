#!/usr/bin/env python3
"""VC++ 2008 x64 side-by-side assembly used by Steam-launched games."""
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "app/Madeira/WineProcessBridge.m").read_text(encoding="utf-8")
failures = []


def check(condition, label):
    print(("ok   " if condition else "FAIL ") + label)
    if not condition:
        failures.append(label)


match = re.search(r"static void madeira_seed_winsxs_amd64_vc90\(.*?\n\}", SOURCE, re.S)
body = match.group(0) if match else ""
check(bool(match), "amd64 VC90 WinSxS seeder exists")
check("processorArchitecture=\\\"amd64\\\"" in body and "Microsoft.VC90.CRT" in body,
      "manifest identity is Microsoft.VC90.CRT amd64")
check("9.0.30729.6161" in body and "1fc8b3b9a1e18e3b" in body,
      "manifest pins the SP1 security-update version and public key")
check('"msvcr90.dll", "msvcp90.dll"' in body and "x86_64-vcruntime" in body,
      "assembly links Microsoft's x64 msvcr90 and msvcp90 payloads")
check(SOURCE.count("madeira_seed_winsxs_amd64_vc90(") == 2,
      "seeder is defined once and called once")

# It must not be inside the optional i386 farm arm: an aarch64 Dock desktop
# launches x64 games too (the Don't Starve Together case).
call = SOURCE.rfind("madeira_seed_winsxs_amd64_vc90(fm, prefix, bundlePath);")
i386 = SOURCE.rfind("if (has_i386_set) {", 0, call)
next_block = SOURCE.find("/* ml719:", i386)
check(call > i386 and call < next_block and SOURCE.find("}", i386, call) >= 0,
      "amd64 seeding runs after, not inside, the optional i386 block")

for name in ("msvcr90.dll", "msvcp90.dll"):
    path = ROOT / "app/Madeira/x86_64-vcruntime" / name
    if not path.is_file():
        print(f"skip {name} (fetch-vcruntime.sh supplies it on Codemagic)")
        continue
    data = path.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    check(data[:2] == b"MZ" and data[pe:pe + 4] == b"PE\0\0" and
          struct.unpack_from("<H", data, pe + 4)[0] == 0x8664,
          f"{name} is an x86-64 PE runtime")

actctx = (ROOT / "wine/dlls/ntdll/actctx.c").read_text(encoding="utf-8", errors="replace")
check("build < min_build" in actctx and "revision < min_revision" in actctx,
      "Wine accepts a later VC90 build/revision for the requested 9.0 assembly")

print("PASS" if not failures else "FAILED")
sys.exit(1 if failures else 0)
