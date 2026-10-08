#!/usr/bin/env python3
"""Production startup policies: pressure-only swap and Unity synchronization.

Does not simulate or measure Silksong FPS, Mono compilation or iOS paging.
"""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")
virtual = (root / "build/ntdll-unix/virtual_ios.c").read_text(encoding="utf-8")
bridge = (root / "app/Madeira/WineProcessBridge.m").read_text(encoding="utf-8")
assert 'ios_swap_pressure_update((uint64_t)os_proc_available_memory());' in bridge
assert 'ios_swap_pressure_update((uint64_t)os_proc_available_memory());' in virtual
for function in ("ios_swap_commit", "ios_swap_reserve"):
    body = virtual.split("static void " + function + "(", 1)[1].split("\n}", 1)[0]
    assert "ios_swap_pressure_" in body and "os_proc_available_memory" not in body
commit = virtual.split("static void ios_swap_commit(", 1)[1].split("\n}", 1)[0]
assert commit.index("ios_swap_eligible(base") < commit.index("ios_swap_pressure_back_commit")
assert 'get("env.MADEIRA_SWAP_PRESSURE") == nil' in library
assert 'coverage == "classic"' in library and 'getenv("MADEIRA_SWAP_PRESSURE") == nil' in library
assert ".disabled(!entry.fastSyncControlsAvailable)" in library

cc = os.environ.get("MD_RUNTIME_CC") or shutil.which("cc")
if not cc:
    if "--require-tools" in sys.argv: raise SystemExit("C compiler required")
    print("SKIP: pressure policy requires a C compiler")
else:
    c = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "ios_swap_pressure.h"
int main(void) {
    const uint64_t gb = 1ull << 30;
    assert(ios_swap_pressure_needs_backing(0, 16 * gb, 16 * 1024 * 1024));
    assert(ios_swap_pressure_needs_backing(1, 0, 16 * 1024 * 1024));
    assert(!ios_swap_pressure_needs_backing(1, 8 * gb, 16 * 1024 * 1024));
    assert(!ios_swap_pressure_needs_backing(1, 4 * gb, 64 * 1024 * 1024));
    assert(ios_swap_pressure_needs_backing(1, 2 * gb, 4096));
    assert(ios_swap_pressure_needs_backing(1, 3 * gb, gb));
    assert(ios_swap_pressure_needs_backing(1, gb, 2 * gb));
    assert(ios_swap_pressure_needs_backing(1, UINT64_MAX, SIZE_MAX));
    uint64_t burst = 8 * gb;
    for (unsigned i = 0; i < 5; ++i) assert(!ios_swap_pressure_back_commit(1, &burst, gb));
    assert(burst == 3 * gb);
    assert(ios_swap_pressure_back_commit(1, &burst, gb));
    assert(burst == 3 * gb); // file backing does not spend anonymous headroom
    burst = 0;
    assert(ios_swap_pressure_back_commit(1, &burst, 4096));
    burst = 8 * gb;
    assert(ios_swap_pressure_back_commit(0, &burst, gb) && burst == 8 * gb);
    puts("PASS: actual swap policy keeps hot allocations anonymous with ample RAM; pressure/unknown headroom retain backing");
}
'''
    with tempfile.TemporaryDirectory(prefix="madeira-silksong-native-") as scratch:
        folder = Path(scratch); source = folder / "test.c"
        binary = folder / ("test.exe" if os.name == "nt" else "test")
        source.write_text(c, encoding="utf-8")
        subprocess.run([cc, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                        "-I" + str(root / "build/ntdll-unix"), str(source), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)

swiftc = os.environ.get("SWIFTC") or shutil.which("swiftc")
if not swiftc:
    if "--require-swift" in sys.argv: raise SystemExit("swiftc required for the actual Unity sync policy")
    print("SKIP: actual Unity sync policy requires swiftc; Codemagic requires it")
else:
    policy = "enum UnityStartupSync {" + library.split("enum UnityStartupSync {", 1)[1].split("\nenum UnityLaunch", 1)[0]
    tests = r'''
func mode(_ engine: String, _ unity: Bool = true, _ game: Bool? = nil,
          _ global: String? = nil, _ allowed: Bool = true) -> String? {
    UnityStartupSync.mode(engine: engine, unity: unity, gameEnabled: game,
                         globalMode: global, automaticAllowed: allowed)
}
assert(mode("wine") == "auto")
assert(mode("wine", false) == nil)
assert(mode("wine", true, false) == nil)
assert(mode("wine", true, nil, "0") == nil)
assert(mode("wine", true, nil, "off") == nil)
assert(mode("wine", true, nil, nil, false) == nil)
assert(mode("madsync") == nil && mode("madsync", true, true, "1") == nil)
assert(mode("fastsync", false) == "auto")
assert(mode("fastsync", false, false) == "0")
assert(mode("fastsync", false, true, "cells") == "cells")
print("PASS: actual Unity startup sync; per-game off, global off, profile off and Madsync isolation")
'''
    with tempfile.TemporaryDirectory(prefix="madeira-silksong-swift-") as scratch:
        folder = Path(scratch); source = folder / "main.swift"; binary = folder / "test"
        source.write_text(policy + tests, encoding="utf-8")
        subprocess.run([swiftc, str(source), "-o", str(binary)], check=True, timeout=60)
        subprocess.run([str(binary)], check=True, timeout=10)
print("PASS: production launch publishes headroom; swap decisions do not query memory under virtual_mutex")
