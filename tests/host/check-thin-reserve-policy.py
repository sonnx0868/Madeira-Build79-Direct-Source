#!/usr/bin/env python3
"""Exercise the real thin-reservation configuration, including opt-out after init.

This is a policy test; the x64 device probe covers VirtualAlloc behavior.
"""
from pathlib import Path
import os, shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
source = (root / "build/ntdll-unix/virtual_ios.c").read_text(encoding="utf-8")
config = source[source.index("static int ios_thin_config(void)"):source.index("/* Like mmap_add_fex_reserved_area")]
compiler = os.environ.get("MD_RUNTIME_CC") or shutil.which("cc") or shutil.which("clang")
if not compiler:
    raise SystemExit("A C compiler is required for the thin reservation policy test")
harness = r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
typedef uint64_t ULONG_PTR;
#define IOS_THIN_GUARD 0x100000ull
static int ios_thin_mode = -1;
static ULONG_PTR ios_thin_stride, ios_thin_head, ios_thin_after;
static const char *enabled, *stride, *after;
static char *test_getenv(const char *name) {
    return (char *)(!strcmp(name,"MADEIRA_THIN_RESERVE") ? enabled :
                   !strcmp(name,"MADEIRA_THIN_RESERVE_MB") ? stride : after);
}
#define getenv test_getenv
#define dprintf(...) (0)
'''
checks = r'''
int main(void) {
    assert(!ios_thin_config() && ios_thin_mode==-1);
    for (int i=0; i<4; ++i) {
        enabled=(const char *[]) {"0","true","01","11"}[i];
        assert(!ios_thin_config());
    }
    enabled="1"; assert(ios_thin_config());
    assert(ios_thin_stride==(64ull<<20) && ios_thin_head==(63ull<<20) && ios_thin_after==(8ull<<30));
    enabled="0"; assert(!ios_thin_config());
    enabled=NULL; assert(!ios_thin_config());
    enabled="1"; assert(ios_thin_config());
    ios_thin_mode=-1; stride="4096"; after="1000000";
    assert(ios_thin_config() && ios_thin_stride==(512ull<<20) && ios_thin_after==(64ull<<30));
    ios_thin_mode=-1; stride="1"; after="0";
    assert(ios_thin_config() && ios_thin_stride==(16ull<<20) && ios_thin_head==(15ull<<20) && ios_thin_after==0);
    return 0;
}
'''
with tempfile.TemporaryDirectory() as folder:
    path = Path(folder)
    c = path / "policy.c"
    exe = path / ("policy.exe" if os.name == "nt" else "policy")
    c.write_text(harness + config + checks, encoding="utf-8")
    subprocess.run([compiler, "-std=c11", "-O2", str(c), "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print("PASS: thin reservations require explicit 1, disable after initialization, and bound stride/threshold")
