#!/usr/bin/env python3
"""Execute the production guest spill allocator without mappings in the FEX arena."""
from pathlib import Path
import os, shutil, subprocess, tempfile
root=Path(__file__).resolve().parents[2]
source=(root/'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
start=source.index('static NTSTATUS ios_allocate_guest_spill(')
helper=source[start:source.index('\n#endif',start)]
compiler=os.environ.get('CC') or shutil.which('clang')
if not compiler: raise SystemExit('clang required')
c=r'''
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "guest_spill_ranges.h"
typedef uint64_t ULONG_PTR;
typedef size_t SIZE_T;
typedef unsigned DWORD;
typedef int NTSTATUS;
#define STATUS_NO_MEMORY -1
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
static const ULONG_PTR ios_steer_slot=0x7c00000000ull, ios_spill_cap=0x7800000000ull;
static const ULONG_PTR host_addr_space_limit=0x8000000000ull;
static ULONG_PTR ios_fex_arena_base_unix=0x7c00000000ull, ios_fex_arena_end_unix=0x8000000000ull;
static uint64_t cursor=0x7400000000ull;
static unsigned calls;
static int fail;
static NTSTATUS allocate_virtual_memory(void **ret,SIZE_T *size,DWORD type,DWORD protect,
                                       ULONG_PTR lo,ULONG_PTR hi,int a,int b) {
    (void)type;(void)protect;(void)a;(void)b;++calls;
    assert(hi<=ios_fex_arena_base_unix || lo>=ios_fex_arena_end_unix);
    if(fail) { *ret=(void*)1;*size=1;return STATUS_NO_MEMORY; }
    uint64_t at=cursor>lo ? cursor : lo;
    if(at>hi || *size>hi-at) return STATUS_NO_MEMORY;
    *ret=(void*)(uintptr_t)at;cursor=at+*size;return 0;
}
'''+helper+r'''
int main(void) {
    // Reproduce the large reserves that consumed Party's host arena, with
    // more reserves than the observed 17. They now occupy the spare guest window.
    for(unsigned i=0;i<24;i++) {
        void *p=NULL;size_t bytes=0x1ffff000ull;
        assert(!ios_allocate_guest_spill(&p,&bytes,0x2000,1));
        assert((uint64_t)(uintptr_t)p>=0x7400000000ull);
        assert((uint64_t)(uintptr_t)p+bytes<=ios_spill_cap);
    }
    assert(calls==24);
    fail=1;void *p=(void*)42;size_t bytes=123;
    assert(ios_allocate_guest_spill(&p,&bytes,0x2000,1)==STATUS_NO_MEMORY);
    assert(p==(void*)42 && bytes==123); // failure cannot corrupt retry inputs
    struct ios_spill_range ranges[2];
    for(uint64_t lo=1;lo<35;lo++) for(uint64_t hi=lo+1;hi<50;hi++)
      for(uint64_t base=1;base<50;base+=3) for(uint64_t end=base+1;end<60;end+=5) {
        unsigned n=ios_guest_spill_ranges(lo,hi,base,end,ranges);
        assert(n<=2);
        for(unsigned i=0;i<n;i++) {
          assert(ranges[i].lo>=lo && ranges[i].hi<=hi && ranges[i].lo<ranges[i].hi);
          assert(ranges[i].hi<=base || ranges[i].lo>=end);
        }
      }
    assert(ios_guest_spill_ranges(10,30,0,0,ranges)==1);
    assert(ios_guest_spill_ranges(10,30,10,30,ranges)==0);
    assert(ios_guest_spill_ranges(30,10,0,0,ranges)==0);
    puts("PASS: guest reserves exclude FEX, Party-sized reservations fit, split boundaries and failed retries preserve inputs");
}
'''
assert source.count('sst = ios_allocate_guest_spill(ret, size_ptr, type, protect);')==2
with tempfile.TemporaryDirectory(prefix='madeira-guest-spill-') as scratch:
    path=Path(scratch);(path/'check.c').write_text(c,encoding='utf-8');exe=path/'check.exe'
    subprocess.run([compiler,'-std=c11','-O2','-I'+str(root/'build/ntdll-unix'),str(path/'check.c'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
