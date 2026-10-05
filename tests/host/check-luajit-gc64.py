#!/usr/bin/env python3
"""Verify the LÖVE 11.5 GC64 runtime and the effective game-DLL redirect."""
from pathlib import Path
import hashlib
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
PATCH = ROOT / "patches/wine-luajit-gc64-file-redirect.patch"
RUNTIME = ROOT / "app/Madeira/arm64ec-windows/lua51-gc64.dll"
WANT = "94fb3e3d4b1f6acce0110e46aadf1ecab1fa17c4ed1ab34caef3c5c2121c208e"
failures = []


def check(condition, label):
    print(("ok   " if condition else "FAIL ") + label)
    if not condition:
        failures.append(label)


patch = PATCH.read_text(encoding="utf-8")
bootstrap = (ROOT / "scripts/bootstrap-native-deps.sh").read_text(encoding="utf-8")
preflight = (ROOT / "scripts/check-ios-build.sh").read_text(encoding="utf-8")
library = (ROOT / "app/Madeira/Library.swift").read_text(encoding="utf-8")
bridge = (ROOT / "app/Madeira/WineProcessBridge.m").read_text(encoding="utf-8")

check("NtCreateFile" in patch and "_MADEIRA_LUA51_GC64_PATH" in patch,
      "ordinary game DLL opens reach the GC64 redirect")
check("disposition == FILE_OPEN" in patch and "FILE_WRITE_DATA | FILE_APPEND_DATA | GENERIC_WRITE" in patch,
      "redirect is restricted to read-only FILE_OPEN")
check("lua51W" in patch and "chars == ARRAY_SIZE(lua51W)" in patch,
      "redirect matches a complete lua51.dll path component")
check("NtCreateFile redirect" in patch and "original untouched" in patch,
      "runtime log proves the effective redirect")
check(PATCH.name in bootstrap, "bootstrap applies the Wine redirect patch")
check("NtCreateFile redirect" in preflight,
      "native-bundle preflight rejects an archive built before the redirect")
check('MadeiraConfig.flag("MADEIRA_LUAJIT_GC64")' in library and
      'setenv("_MADEIRA_LUA51_GC64", "1", 1)' in library,
      "detected LÖVE 11.5 sessions request GC64")
check('setenv("_MADEIRA_LUA51_GC64_PATH"' in bridge,
      "app publishes the verified bundle path before Wine starts")

# In a normal source checkout the Wine submodule is clean and the patch must
# apply. Worktree-only environments may not have a standalone submodule .git;
# --directory still validates the exact pinned source through the parent repo.
apply = subprocess.run(["git", "apply", "--check", "--directory=wine", str(PATCH)],
                       cwd=ROOT, capture_output=True, text=True)
check(apply.returncode == 0, "patch applies cleanly to pinned Wine file.c")
if apply.returncode:
    print(apply.stderr.strip())

if RUNTIME.is_file():
    data = RUNTIME.read_bytes()
    check(hashlib.sha256(data).hexdigest() == WANT, "staged GC64 DLL SHA-256")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    check(data[:2] == b"MZ" and data[pe:pe + 4] == b"PE\0\0" and
          struct.unpack_from("<H", data, pe + 4)[0] == 0x8664,
          "staged GC64 DLL is x86-64 PE")
    optional = pe + 24
    check(struct.unpack_from("<I", data, optional + 56)[0] == 0x8E000,
          "staged GC64 DLL SizeOfImage is 0x8e000")
else:
    print("skip staged DLL (fetch-love-luajit-gc64.sh supplies it on Codemagic)")

print("PASS" if not failures else "FAILED")
sys.exit(1 if failures else 0)
