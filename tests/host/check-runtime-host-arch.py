#!/usr/bin/env python3
"""Require the actual packaged supervisor to be native ARM64, not x64/EC."""
from pathlib import Path
import struct
import sys

root = Path(__file__).resolve().parents[2]
host = Path(sys.argv[1]) if len(sys.argv) > 1 else root / "app/Madeira/aarch64-windows/madeira-session-host.exe"
data = host.read_bytes()
if len(data) < 64 or data[:2] != b"MZ": raise SystemExit("Invalid session host: missing DOS header")
pe = struct.unpack_from("<I", data, 60)[0]
if pe + 94 > len(data) or data[pe:pe + 4] != b"PE\0\0": raise SystemExit("Invalid session host: missing PE header")
machine = struct.unpack_from("<H", data, pe + 4)[0]
magic = struct.unpack_from("<H", data, pe + 24)[0]
subsystem = struct.unpack_from("<H", data, pe + 24 + 68)[0]
if (machine, magic, subsystem) != (0xaa64, 0x20b, 2):
    raise SystemExit(f"Outdated session host: machine={machine:#x} magic={magic:#x} subsystem={subsystem}; native ARM64 GUI PE required")
print("PASS: packaged reusable supervisor is native ARM64 GUI PE (no x64-root clone route)")
