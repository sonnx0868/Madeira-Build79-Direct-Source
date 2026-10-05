#!/usr/bin/env python3
"""Static and staged-binary checks for the SDL/LÖVE -> ANGLE -> DXMT route."""
from pathlib import Path
import hashlib
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
EXPECTED = {
    "libEGL.dll": "d9c8541eaf0293c67ece10e97d00a8b689d5e043a8356d43224aac1af3a21a5f",
    "libGLESv2.dll": "a1275f57b47575db9aa3a577e5eacba1d7f1d5578ef6a8072468d788c93c85ff",
}


def check(label, condition):
    global ok
    print(("ok   " if condition else "FAIL ") + label)
    ok &= condition


ok = True
fetch = (ROOT / "scripts/fetch-angle-d3d11.sh").read_text(encoding="utf-8")
library = (ROOT / "app/Madeira/Library.swift").read_text(encoding="utf-8")
bridge = (ROOT / "app/Madeira/WineProcessBridge.m").read_text(encoding="utf-8")
package = (ROOT / "scripts/package-native-deps.sh").read_text(encoding="utf-8")
preflight = (ROOT / "scripts/check-ios-build.sh").read_text(encoding="utf-8")

check("runtime detection uses SDL/LÖVE files, not a title allow-list",
      'names.contains("sdl2.dll")' in library and 'lastPathComponent.lowercased() == "love.dll"' in library)
check("SDL EGL and LÖVE GLES hints are applied",
      'setenv("SDL_OPENGL_ES_DRIVER", "1", 1)' in bridge and
      'setenv("LOVE_GRAPHICS_USE_OPENGLES", "1", 1)' in bridge)
check("ANGLE is forced to the D3D11 backend consumed by DXMT",
      'setenv("ANGLE_DEFAULT_PLATFORM", "d3d11", 1)' in bridge)
check("user can disable the automatic route", "MADEIRA_OPENGL_ANGLE" in library and
      'getenv("MADEIRA_OPENGL_ANGLE")' in bridge)
check("both DLLs are restored by the reusable native bundle",
      all(name in package and name in preflight for name in EXPECTED))
check("archive and staged hashes are pinned", all(value in fetch for value in EXPECTED.values()))
check("ANGLE licence and complete third-party notices are declared",
      (ROOT / "app/Madeira/licenses/ANGLE-BSD-3.txt").is_file() and
      "ANGLE-THIRD-PARTY-NOTICES.html" in fetch)

for name, expected in EXPECTED.items():
    path = ROOT / "app/Madeira/arm64ec-windows" / name
    if not path.is_file():
        print(f"skip staged {name} (fetch script supplies it on Codemagic)")
        continue
    data = path.read_bytes()
    check(f"{name} SHA-256", hashlib.sha256(data).hexdigest() == expected)
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    check(f"{name} is x86-64 PE", data[:2] == b"MZ" and data[pe:pe + 4] == b"PE\0\0" and
          struct.unpack_from("<H", data, pe + 4)[0] == 0x8664)
    if name == "libEGL.dll":
        check("libEGL exports the SDL EGL entry point", b"eglGetPlatformDisplayEXT" in data)
    else:
        check("libGLESv2 contains ANGLE's D3D11 backend", b"D3D11CreateDevice" in data)

print("PASS" if ok else "FAILED")
sys.exit(0 if ok else 1)
