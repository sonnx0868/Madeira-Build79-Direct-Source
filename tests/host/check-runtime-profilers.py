#!/usr/bin/env python3
"""Forensic thread-suspending profilers must be opt-in during gameplay."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
server = (root / "build/ntdll-unix/server_ios.c").read_text(encoding="utf-8")
docs = (root / "docs/LIBRARY.md").read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"runtime profiler contract failed: {message}")


start = server.index("/* iOS-Madeira 2026-07-03 sampling profiler")
startup = server[start:server.index("void server_init_thread", start)]
require('getenv("MADEIRA_RUNTIME_PROFILERS")' in startup,
        "there is no explicit profiling opt-in")
require("runtime_profilers && __sync_bool_compare_and_swap" in startup,
        "task-wide samplers still start in normal sessions")
for profiler in ("ios_thread_sampler_main", "ios_xprobe_main", "ios_wprof_main"):
    require(profiler in startup, f"test no longer covers {profiler}")
require("if (runtime_profilers)" in startup and "thread_suspend" in startup,
        "the high-frequency profiler is not under the same opt-in")
require("multi-second presentation gaps in Balatro" in docs,
        "the measured gameplay cost is not documented")

print("runtime profiler opt-in contract: ok")
