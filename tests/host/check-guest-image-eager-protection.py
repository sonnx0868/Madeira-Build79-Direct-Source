#!/usr/bin/env python3
"""Reproduce Chrome922's CPADinfo write fault on a shared 16 KB host page.

Compile the production eager-copy decision and guest-image predicate. Exercise
real host page protection and an actual store in child processes: the old eager
pass must fault, the new pass must retain the store. Native images and the
explicit legacy opt-out still take the original copy path. No browser success
or on-device performance is inferred from this regression.
"""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
src = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
parser = argparse.ArgumentParser()
parser.add_argument('--chrome-dll', type=Path)
args = parser.parse_args()


def function(signature):
    start = src.index(signature)
    brace = src.index('{', start)
    depth = 0
    for i in range(brace, len(src)):
        if src[i] == '{':
            depth += 1
        elif src[i] == '}':
            depth -= 1
            if not depth:
                return src[start:i + 1]
    raise AssertionError(signature)


# Both eager callers must go through the decision after final section rights.
assert src.count('ios_eager_image_exec(sec_addr, sec_size, prot);') == 2
assert 'mprotect_exec(sec_addr, sec_size, prot);' not in src
helpers = '\n'.join(function(sig) for sig in (
    'static int ios_guest_image_write_enabled(',
    'static int ios_guest_image_is_host_data(',
    'static inline int ios_eager_image_exec(',
))

# The exact mixed page from the signed Google Chrome Stable32 DLL in the log.
sections = [(0xfec0000, 0x18, 0x60000020), (0xfec1000, 0x486, 0xc0000040),
            (0xfec2000, 0x28, 0xc0000040), (0xfec3000, 0x71a, 0x60000020)]
if args.chrome_dll:
    # Parse PE section headers without requiring a third-party Python module.
    import struct
    data = args.chrome_dll.read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[pe:pe + 4] == b'PE\0\0'
    assert struct.unpack_from('<H', data, pe + 4)[0] == 0x14c
    count = struct.unpack_from('<H', data, pe + 6)[0]
    off = pe + 24 + struct.unpack_from('<H', data, pe + 20)[0]
    found = []
    for i in range(count):
        header = off + i * 40
        virtual_size, rva = struct.unpack_from('<II', data, header + 8)
        if 0xfec0000 <= rva < 0xfec4000:
            found.append((rva, virtual_size, struct.unpack_from('<I', data, header + 36)[0]))
    assert found == sections, found
    print('PASS: regression layout matches the supplied Chrome32 DLL', flush=True)

fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#endif
#define WINE_IOS 1
#define SEC_IMAGE 0x1000000
#ifdef _WIN32
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#endif
struct file_view { void *base; size_t size; unsigned protect; };
static struct file_view view;
static int guest, native_calls, last_prot;
static void *last_base;
static size_t last_size;
static int ios_wow_in_window(const void *p) { (void)p; return guest; }
static struct file_view *find_view(const void *p, size_t n) {
    uintptr_t a=(uintptr_t)p, b=(uintptr_t)view.base;
    return a>=b && n<=view.size && a-b<=view.size-n ? &view : NULL;
}
static void protect_page(void *p, int writable) {
#ifdef _WIN32
    DWORD old;
    assert(VirtualProtect(p,0x4000,writable?PAGE_READWRITE:PAGE_READONLY,&old));
#else
    assert(!mprotect(p,0x4000,writable?(PROT_READ|PROT_WRITE):PROT_READ));
#endif
}
static int mprotect_exec(void *base, size_t size, int prot) {
    ++native_calls; last_base=base; last_size=size; last_prot=prot;
    // Model iOS ml1030's guest-image EXEC removal and host-page rounding.
    // This is precisely the old eager call, after final vprot was installed.
    if(guest && (view.protect&SEC_IMAGE))
        protect_page(view.base, (prot&PROT_WRITE)!=0);
    return 0;
}
''' + helpers + r'''
int main(int argc, char **argv) {
    assert(argc==2);
    const int old = !strcmp(argv[1],"old");
    const int legacy = !strcmp(argv[1],"legacy");
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    void *page=VirtualAlloc(NULL,0x4000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    assert(page);
#else
    struct rlimit no_core={0,0}; setrlimit(RLIMIT_CORE,&no_core);
    void *page=mmap(NULL,0x4000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
    assert(page!=MAP_FAILED);
#endif
    view=(struct file_view){page,0x4000,SEC_IMAGE};
    guest=1;
    // Final protection pass: executable 4 KB + writable TLS + writable
    // CrashpadInfo + executable malloc hook. Host page's union is R+W;
    // guest EXEC is bookkeeping, not host-executed machine code.
    protect_page(page,1);
    for(unsigned offset=0;offset<=0x3000;offset+=0x3000) {
        if(old) mprotect_exec((char*)page+offset,0x1000,PROT_READ|PROT_EXEC);
        else ios_eager_image_exec((char*)page+offset,0x1000,PROT_READ|PROT_EXEC);
    }
    if(legacy) {
        assert(native_calls==2); // explicit env opt-out retains old path
        return 0;
    }
    if(!old) assert(native_calls==0);
    // The precise faulting CPADinfo field from b91c99ca at chrome.dll+fec2024.
    volatile unsigned char *field=(unsigned char*)page+0x2024;
    *field=2;
    assert(*field==2);
    if(old) return 99; // the regression must actually fault before this

    // Native ARM64/ARM64EC images, non-image guests and legacy disabled
    // policy must retain exactly the requested native-copy parameters.
    guest=0;
    ios_eager_image_exec((char*)page+0x1000,0x1000,PROT_READ|PROT_EXEC);
    assert(native_calls==1 && last_base==(char*)page+0x1000);
    assert(last_size==0x1000 && last_prot==(PROT_READ|PROT_EXEC));
    guest=1; view.protect=0;
    ios_eager_image_exec(page,0x4000,PROT_READ|PROT_WRITE|PROT_EXEC);
    assert(native_calls==2 && last_prot==7);
    view.protect=SEC_IMAGE;
    // No containing view: leave classification and the existing path intact.
    ios_eager_image_exec((char*)page+0x4000,0x1000,PROT_READ|PROT_EXEC);
    assert(native_calls==3 && last_size==0x1000);
#ifdef _WIN32
    assert(VirtualFree(page,0,MEM_RELEASE));
#else
    assert(!munmap(page,0x4000));
#endif
    puts("PASS: real CPADinfo store; guest eager pass preserves mixed-page WRITE; native/non-image paths unchanged");
    return 0;
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler:
    raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-guest-image-protection-') as scratch:
    work = Path(scratch)
    source = work / 'check.c'
    exe = work / ('check.exe' if os.name == 'nt' else 'check')
    source.write_text(fixture, encoding='utf-8')
    subprocess.run([compiler, '-std=c11', '-O2', '-Werror', str(source), '-o', str(exe)], check=True)
    env = dict(os.environ, MADEIRA_GUEST_IMAGE_WRITE='1')
    baseline = subprocess.run([str(exe), 'old'], env=env, capture_output=True)
    expected = (0xc0000005, -1073741819) if os.name == 'nt' else (-11, -10)
    assert baseline.returncode in expected, (baseline.returncode, baseline.stderr)
    print('PASS: old eager pass reproduces a real write access violation', flush=True)
    subprocess.run([str(exe), 'fixed'], env=env, check=True)
    subprocess.run([str(exe), 'legacy'], env=dict(env, MADEIRA_GUEST_IMAGE_WRITE='0'), check=True)
print('NOT MEASURED: physical Chrome startup, sign-in, Google Sync or extensions')
