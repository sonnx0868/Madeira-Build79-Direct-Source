#!/usr/bin/env python3
"""Forensic thread-suspending profilers must be opt-in during gameplay."""

from pathlib import Path

root = Path(__file__).resolve().parents[2]
server = (root / "build/ntdll-unix/server_ios.c").read_text(encoding="utf-8")
virtual = (root / "build/ntdll-unix/virtual_ios.c").read_text(encoding="utf-8")
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
monitor = virtual[virtual.index("static int ios_memory_census_enabled"):virtual.index("/* task #34: WAS THE POOL COPY")]
require('madeira_cfg_bool( "env.MADEIRA_MEMORY_CENSUS", 0 )' in monitor,
        "VM-map census is not explicitly default-off")
require("ios_memory_census_enabled() && (cycle == 2 || (cycle % 5) == 0)" in monitor,
        "the full physical-memory map is still walked during normal gameplay")
require("ios_memory_census_enabled() && (cycle == 1 || (cycle % 15) == 0)" in monitor,
        "periodic address-space inventories are not opt-in")
require("if (ios_memory_census_enabled())\n                    {\n                        static kern_return_t last[4]" in monitor,
        "CoreAnimation allocation experiments still run during normal play")
require("QOS_CLASS_UTILITY" in monitor and "ios_last_footprint_mb = fp_mb" in monitor and
        "sink += rx[o]" in monitor and "LOST EXEC" in monitor,
        "residency warming / execute checks / footprint control were removed")
require("MADEIRA_MEMORY_CENSUS" in docs, "the memory census escape hatch is undocumented")

print("runtime profiler opt-in contract: ok")
