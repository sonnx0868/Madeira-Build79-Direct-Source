#!/usr/bin/env python3
"""Replay Chrome920's actual PA reserve sequence against the production arena policy.

Compiles ios_reserve_fex_arena unchanged with a bounded Mach-map model. This
checks ownership/placement and rollback, not physical iPad browser execution.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
start = source.index('void ios_reserve_fex_arena(void)')
arena = source[start:source.index('\nNTSTATUS WINAPI NtAllocateVirtualMemoryEx(', start)]
compiler = os.environ.get('CC') or shutil.which('clang')
if not compiler:
    raise SystemExit('clang required')

fixture = r'''
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define min(a,b) ((a)<(b)?(a):(b))
#define dprintf(...) ((void)0)
#define ERR(...) ((void)0)
#define VM_FLAGS_FIXED 0
#define VM_FLAGS_ANYWHERE 1
#define KERN_SUCCESS 0
#define KERN_FAILURE 5
#define TASK_VM_INFO 1
#define TASK_VM_INFO_COUNT 1
#define MEMORY_OBJECT_NULL 0
#define PROT_NONE 0
#define VM_PROT_ALL 7
#define VM_INHERIT_COPY 1
#define MEM_RESERVE 0x2000
#define MEM_RESERVE_PLACEHOLDER 0x40000
#define PAGE_NOACCESS 1
#define MemExtendedParameterAddressRequirements 1
#define NtCurrentProcess() 0
typedef uint64_t ULONG_PTR;
typedef uint64_t mach_vm_address_t;
typedef uint64_t mach_vm_size_t;
typedef uint64_t SIZE_T;
typedef unsigned mach_msg_type_number_t;
typedef int kern_return_t;
typedef int NTSTATUS;
typedef void *task_info_t;
typedef struct { uint64_t max_address; } task_vm_info_data_t;
typedef struct { void *LowestStartingAddress, *HighestEndingAddress; uint64_t Alignment; } MEM_ADDRESS_REQUIREMENTS;
typedef struct { int Type; void *Pointer; } MEM_EXTENDED_PARAMETER;
struct span { uint64_t lo, hi; int owned; } spans[32];
static unsigned nspans, attempted, released, registered;
static int fail_registration, disable_arena;
static uint64_t task_max = 0x8000000000ull;
static uint64_t ios_layerkit_lo, ios_layerkit_hi;
static ULONG_PTR ios_fex_arena_base_unix, ios_fex_arena_end_unix;
static const char *browser_flag;
static char published_base[64], published_size[64];
static int mach_task_self(void) { return 1; }
static int task_info(int task, int flavor, task_info_t out, unsigned *count) {
    (void)task; (void)flavor; (void)count;
    ((task_vm_info_data_t *)out)->max_address = task_max; return 0;
}
static int ios_in_layerkit(uint64_t lo, uint64_t size) {
    return ios_layerkit_hi && lo < ios_layerkit_hi && lo + size > ios_layerkit_lo;
}
static long long madeira_cfg_int(const char *key, long long fallback) {
    (void)key; return fallback;
}
static const char *mock_getenv(const char *key) {
    if (!strcmp(key, "_MADEIRA_BROWSER_VA")) return browser_flag;
    if (!strcmp(key, "MADEIRA_FEX_ARENA") && disable_arena) return "0";
    return NULL;
}
#define getenv mock_getenv
static int mock_setenv(const char *key, const char *value, int overwrite) {
    (void)overwrite;
    if (!strcmp(key, "WINE_IOS_FEX_ARENA_BASE")) snprintf(published_base, sizeof(published_base), "%s", value);
    if (!strcmp(key, "WINE_IOS_FEX_ARENA_SIZE")) snprintf(published_size, sizeof(published_size), "%s", value);
    return 0;
}
#define setenv mock_setenv
static int overlaps(uint64_t lo, uint64_t size) {
    if (!size || lo >= task_max || size > task_max - lo) return 1;
    for (unsigned i=0; i<nspans; ++i)
        if (lo < spans[i].hi && lo+size > spans[i].lo) return 1;
    return 0;
}
static void occupy(uint64_t lo, uint64_t size, int owned) {
    assert(nspans<32); spans[nspans++] = (struct span){lo,lo+size,owned};
}
static int mach_vm_map(int task, uint64_t *at, uint64_t size, uint64_t mask,
                       int flags, int object, int offset, int copy,
                       int prot, int maxprot, int inheritance) {
    (void)task;(void)mask;(void)object;(void)offset;(void)copy;
    (void)prot;(void)maxprot;(void)inheritance; ++attempted;
    // Mach FIXED without OVERWRITE must refuse even one foreign page.
    if (overlaps(*at,size)) return KERN_FAILURE;
    assert(flags==VM_FLAGS_FIXED || flags==VM_FLAGS_ANYWHERE);
    occupy(*at,size,1); return KERN_SUCCESS;
}
static int mach_vm_deallocate(int task, uint64_t at, uint64_t size) {
    (void)task; ++released;
    for (unsigned i=0;i<nspans;++i) if (spans[i].lo==at && spans[i].hi==at+size) {
        assert(spans[i].owned); spans[i]=spans[--nspans]; return 0;
    }
    assert(!"Released an unowned mapping"); return 1;
}
static int mmap_add_fex_reserved_area(void *at, uint64_t size) {
    (void)at;(void)size; ++registered;
    if (fail_registration) { fail_registration=0; return 0; } return 1;
}
static int NtAllocateVirtualMemoryEx(int p, void **b, uint64_t *s, unsigned t,
                                    unsigned prot, MEM_EXTENDED_PARAMETER *ep, unsigned count) {
    (void)p;(void)b;(void)s;(void)t;(void)prot;(void)ep;(void)count;
    assert(!"Probe-only code must not run"); return 1;
}
''' + arena + r'''
// The exact 32GB ask and 16GB candidate stride in build920's log. Chromium
// frees a successful reservation when it is not aligned to its 32GB size.
static uint64_t chrome_pool(void) {
    uint64_t size=0x800000000ull;
    for (uint64_t at=0x7c00000000ull; at>=0x7400000000ull; at-=0x400000000ull) {
        if (overlaps(at,size)) continue;
        occupy(at,size,1);
        if (!(at & (size-1))) return at;
        mach_vm_deallocate(1,at,size);
    }
    return 0;
}
int main(int argc, char **argv) {
    assert(argc==2);
    if (!strcmp(argv[1],"chrome")) browser_flag="1";
    if (!strcmp(argv[1],"literal-only")) browser_flag="true";
    if (!strcmp(argv[1],"disabled")) { browser_flag="1";disable_arena=1; }
    if (!strcmp(argv[1],"occupied")) { browser_flag="1";occupy(0x7400000000ull,0x4000,0); }
    if (!strcmp(argv[1],"layerkit")) { browser_flag="1";ios_layerkit_lo=0x7400000000ull;ios_layerkit_hi=0x7400004000ull; }
    if (!strcmp(argv[1],"rollback")) { browser_flag="1";fail_registration=1; }
    if (!strcmp(argv[1],"small-task")) { browser_flag="1";task_max=0x1000000000ull; }
    ios_reserve_fex_arena();
    if (disable_arena) {
        assert(!attempted && !registered && !ios_fex_arena_base_unix); return 0;
    }
    assert(ios_fex_arena_base_unix && ios_fex_arena_end_unix>ios_fex_arena_base_unix);
    assert(strtoull(published_base,NULL,16)==ios_fex_arena_base_unix);
    assert(strtoull(published_size,NULL,16)==ios_fex_arena_end_unix-ios_fex_arena_base_unix);
    unsigned releases_before_pool=released;
    if (!strcmp(argv[1],"chrome")) {
        assert(ios_fex_arena_base_unix==0x7400000000ull);
        assert(ios_fex_arena_end_unix==0x7800000000ull);
        assert(chrome_pool()==0x7800000000ull); // actual glued pools now fit
    } else if (!strcmp(argv[1],"small-task")) {
        assert(ios_fex_arena_end_unix<=task_max); // existing low fallback still works
    } else {
        assert(ios_fex_arena_base_unix==0x7c00000000ull);
        assert(ios_fex_arena_end_unix==0x8000000000ull);
        assert(!chrome_pool()); // reproduce the original pre-window failure
    }
    if (!strcmp(argv[1],"rollback")) assert(releases_before_pool==1 && registered==2);
    if (!strcmp(argv[1],"occupied")) {
        assert(spans[0].lo==0x7400000000ull && !spans[0].owned && spans[0].hi==0x7400004000ull);
    }
    return 0;
}
'''

library = (root / 'app/Madeira/Library.swift').read_text(encoding='utf-8')
launch = library[library.index('    func configureLaunch() {'):]
assert launch.index('setenv("_MADEIRA_BROWSER_VA", "1", 1)') < launch.index('if usesSteamSession {')
assert 'if isWindowsBrowser && bits == 64' in launch
assert 'else { unsetenv("_MADEIRA_BROWSER_VA") }' in launch

with tempfile.TemporaryDirectory(prefix='madeira-browser-va-') as scratch:
    work = Path(scratch)
    (work / 'check.c').write_text(fixture, encoding='utf-8')
    exe = work / ('check.exe' if os.name=='nt' else 'check')
    subprocess.run([compiler, '-std=c11', '-O2', str(work / 'check.c'), '-o', str(exe)], check=True)
    for case in ['game', 'chrome', 'literal-only', 'disabled', 'occupied', 'layerkit', 'rollback', 'small-task']:
        subprocess.run([str(exe), case], check=True)
        print('PASS:', case)
print('PASS: original Chrome920 aligned allocation failure reproduced; browser layout removes it without overlap or extra RAM commitment')
print('NOT MEASURED: physical iPad Chrome execution or later renderer/sandbox compatibility')
