#!/usr/bin/env python3
"""Compile production quiet-store/cache policy; macOS also tests the real cache.

No game, FEX execution, iOS frame rate or device speed is simulated here.
"""
from pathlib import Path
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
virtual = (root / "build/ntdll-unix/virtual_ios.c").read_text(encoding="utf-8")
signal = (root / "build/ntdll-unix/signal_arm64_ios.c").read_text(encoding="utf-8")
cache = (root / "dxmt/src/winemetal/unix/cache.c").read_text(encoding="utf-8")
runtime = (root / "app/Madeira/GameRuntime.swift").read_text(encoding="utf-8")
library = (root / "app/Madeira/Library.swift").read_text(encoding="utf-8")

assert 'flag("MADEIRA_MULTI_GAME", fallback: false)' in runtime
assert 'bool("env.MADEIRA_MULTI_GAME", default: false)' in library
assert 'set("env.MADEIRA_MULTI_GAME", on ? "1" : "0")' in library
assert 'if (madeira_diag_enabled)\n                        {\n                            enum { FC_SLOTS' in signal
assert 'if (madeira_diag_enabled)\n                        {\n                            extern volatile unsigned long long ios_store_fault_t0' in signal
assert 'ios_store_fault_t0 = madeira_diag_enabled ? mach_absolute_time() : 0' in signal
assert 'ios_memory_census_enabled() && size >= (4u << 20)' in virtual
commit_probe = virtual.split('static void ios_verify_commit_zero(', 1)[1].split('\n}\n', 1)[0]
assert commit_probe.index('if (!ios_memory_census_enabled()) return;') < commit_probe.index('mach_vm_read_overwrite(')
assert 'if (ios_memory_census_enabled())\n        {\n            static unsigned long dc_hi' in virtual
assert 'ios_mono_bridge_capture( state.__x[18], state.__x[28]' in signal
assert '__atomic_exchange_n((uint64_t *)rw_addr' in signal
assert 'if (slot >= 0)' in signal and 'if (sticky) goto wx_done' in signal
assert 'gameplay-observers=v1' in virtual and 'sandbox-v1 reader ready' in cache
assert 'ios_wow_base' not in cache
print("PASS: quiet probes are gated before work; atomic stores/Mono/execute safety retained; reuse is opt-in")

# Compile the actual native alias-write telemetry and the actual iOS cache switch.
alias = virtual.split('#define IOS_JIT_MAX_ANON_ALIASES', 1)[1].split('/* Allocate a page-aligned range from the pool head:', 1)[0]
alias = '#define IOS_JIT_MAX_ANON_ALIASES' + alias
policy = cache.split('static bool\nuse_ios_cache_dir(void)', 1)[1].split('\n}\n', 1)[0]
policy = 'static bool\nuse_ios_cache_dir(void)' + policy + '\n}\n'
checks = r'''
static void set_cache_switch(const char *value) {
#ifdef _WIN32
    assert(_putenv_s("DXMT_IOS_CACHE_DIR", value ? value : "") == 0);
#else
    assert(value ? setenv("DXMT_IOS_CACHE_DIR", value, 1) == 0 : unsetenv("DXMT_IOS_CACHE_DIR") == 0);
#endif
}
int main(void) {
    set_cache_switch(NULL); assert(use_ios_cache_dir());
    set_cache_switch("0"); assert(!use_ios_cache_dir());
    set_cache_switch("1"); assert(use_ios_cache_dir());
    set_cache_switch(NULL);
    ios_jit_anon_alias_count = 2;
    ios_jit_anon_aliases[0].user_va = 0x10000;
    ios_jit_anon_aliases[0].user_va_end = 0x20000;
    ios_jit_anon_aliases[1].user_va = 0x40000;
    ios_jit_anon_aliases[1].user_va_end = 0x50000;
    for (unsigned i = 0; i < 1000000; ++i) ios_jit_anon_alias_note_write(0x10008);
    assert(ios_alias_write_gen[0] == 0 && ios_alias_written[0] == 0 && ios_alias_highest[0] == 0);
    madeira_diag_enabled = 1;
    ios_jit_anon_alias_note_write(0x10008);
    assert(ios_alias_write_gen[0] == 1 && ios_alias_written[0] == 1 && ios_alias_highest[0] == 9);
    ios_jit_anon_alias_note_write(0x48000);
    assert(ios_alias_write_gen[1] == 1 && ios_alias_written[1] == 4 && ios_alias_highest[1] == 0x8001);
    ios_jit_anon_alias_note_write(0x20000);
    assert(ios_alias_write_gen[0] == 1 && ios_alias_write_gen[1] == 1);
    madeira_diag_enabled = 0;
    ios_jit_anon_alias_note_write(0x40008);
    assert(ios_alias_write_gen[1] == 1);
    puts("PASS: actual iOS cache default/opt-out and quiet/enabled alias telemetry bounds");
}
'''
compiler = os.environ.get("MD_RUNTIME_CC") or os.environ.get("CC") or shutil.which("cc")
if compiler:
    with tempfile.TemporaryDirectory(prefix="madeira-gameplay-check-") as scratch:
        folder = Path(scratch)
        source = folder / "test.c"
        binary = folder / ("test.exe" if os.name == "nt" else "test")
        source.write_text('#include <assert.h>\n#include <stdint.h>\n#include <stdbool.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\nvolatile int madeira_diag_enabled;\n' + alias + policy + checks, encoding="utf-8")
        subprocess.run([compiler, "-std=c11", "-O2", "-Wall", "-Werror", "-Wno-unused-variable", str(source), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
else:
    if "--require-tools" in sys.argv: raise SystemExit("C compiler required")
    print("SKIP: C behavior requires MD_RUNTIME_CC/CC/cc")

if sys.platform == "darwin":
    # Compile the production Objective-C cache classes, not a SQLite imitation.
    classes = cache.split('@interface CacheReader :', 1)[1].split('\nint\n_CacheReader_alloc_init', 1)[0]
    classes = '@interface CacheReader :' + classes
    main = r'''
int main(int argc, char **argv) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    assert(argc == 2 && use_ios_cache_dir());
    NSString *path = [NSString stringWithUTF8String:argv[1]];
    assert(resolve_ios_cache_dir(@"", true) == nil);
    CacheWriter *writer = [[CacheWriter alloc] initWithPath:path version:15];
    assert(writer != nil);
    NSData *key = [@"shader-variant" dataUsingEncoding:NSUTF8StringEncoding];
    const char payload[] = "compiled-bitcode-test";
    dispatch_data_t value = dispatch_data_create(payload, sizeof(payload), NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    [writer set:key value:value];
    dispatch_release(value);
    [writer release];
    // A new reader/process lifetime must retrieve the disk entry after writer close.
    CacheReader *reader = [[CacheReader alloc] initWithPath:path version:15];
    assert(reader != nil);
    dispatch_data_t found = [reader get:key];
    assert(found != NULL);
    const void *bytes; size_t length;
    dispatch_data_t flat = dispatch_data_create_map(found, &bytes, &length);
    assert(length == sizeof(payload) && memcmp(bytes, payload, length) == 0);
    dispatch_release(flat); dispatch_release(found);
    assert([reader get:[@"missing" dataUsingEncoding:NSUTF8StringEncoding]] == NULL);
    [reader release];
    puts("PASS: production Objective-C cache creates directories, writes WAL and reloads the shader bytes");
    [pool drain];
    return 0;
}
'''
    with tempfile.TemporaryDirectory(prefix="madeira-shader-cache-") as scratch:
        folder = Path(scratch); source = folder / "test.m"; binary = folder / "test"
        source.write_text('#import <Foundation/Foundation.h>\n#include <dispatch/dispatch.h>\n#include <sqlite3.h>\n#include <assert.h>\n#include <stdbool.h>\n#include <stdint.h>\n#include <limits.h>\n#include <string.h>\n#include <stdlib.h>\n#include <sys/file.h>\n#include <fcntl.h>\n#include <unistd.h>\n#define TARGET_OS_IPHONE 1\n' + classes + main, encoding="utf-8")
        subprocess.run(["xcrun", "clang", "-fblocks", "-fno-objc-arc", "-Wno-format", str(source), "-framework", "Foundation", "-lsqlite3", "-o", str(binary)], check=True)
        subprocess.run([str(binary), str(folder / "cache" / "shaders.db")], check=True)
else:
    if "--require-objc" in sys.argv: raise SystemExit("macOS/Foundation required for production shader-cache persistence test")
    print("SKIP: Objective-C persistence needs macOS/Foundation; Codemagic requires it")
