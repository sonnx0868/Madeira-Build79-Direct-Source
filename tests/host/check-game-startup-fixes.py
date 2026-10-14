#!/usr/bin/env python3
"""Execute production boot-clock and split-pool helpers at boundary cases."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
native = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
clock = (root / 'wine/dlls/kernelbase/sync.c').read_text(encoding='utf-8')


def function(source, signature):
    start = source.index(signature)
    pos = source.index('{', start) + 1
    depth = 1
    while depth:
        depth += (source[pos] == '{') - (source[pos] == '}')
        pos += 1
    return source[start:pos] + '\n'


helpers = ''.join(function(native, sig) for sig in (
    'static size_t ios_pool_hole_head_place(',
    'static size_t ios_pool_hole_tail_start(',
    'static size_t ios_pool_hole_between(',
    'static void ios_pool_tail_unreserve(',
))
helpers += function(clock, 'static ULONGLONG madeira_tick_count(')
helpers += function(clock, 'ULONG WINAPI DECLSPEC_HOTPATCH GetTickCount(')
helpers += function(clock, 'ULONGLONG WINAPI DECLSPEC_HOTPATCH GetTickCount64(')

assert native.count('if (IOS_POOL_IN_HOLE(o)) continue;') == 4
assert 'ios_pool_hole_head_place( jit_pool_offset, alloc_size,' in native
assert 'ios_pool_hole_tail_start( ios_jit_pool_size_global, cur, alloc_size,' in native
assert native.count('ios_pool_tail_unreserve( &ios_jit_tail_reserved, reserve_offset + alloc_size, tail_added,') == 2
swift = (root / 'app/Madeira/StikJITHelper.swift').read_text(encoding='utf-8')
assert 'krHole == KERN_SUCCESS' in swift
assert 'rw >= rxB + sizeB' in swift
assert 'let wanted = vm_address_t(want) & ~(mb16 - 1)' in swift
content = (root / 'app/Madeira/ContentView.swift').read_text(encoding='utf-8')
assert 'setenv("WINE_IOS_JIT_HOLE"' in content and 'unsetenv("WINE_IOS_JIT_HOLE")' in content

prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
typedef uint64_t ULONGLONG;
typedef uint32_t ULONG;
typedef union { int64_t QuadPart; } LARGE_INTEGER;
#define WINAPI
#define DECLSPEC_HOTPATCH
#define __arm64ec__ 1
#define ERR(...) ((void)0)
static uint64_t now;
static int ok=1;
static int RtlQueryPerformanceCounter(LARGE_INTEGER *c) { c->QuadPart=now; return ok; }
'''
checks = r'''
int main(void) {
    /* The exact boot clock, fractions, 49.7-day 32-bit wrap and sleep jumps. */
    const uint64_t ms[] = {0,1,17,86400000,4294967295ULL,4294967296ULL,4294967297ULL,9000000000ULL};
    for(unsigned i=0;i<sizeof(ms)/sizeof(ms[0]);i++) {
        now=ms[i]*10000+9999;
        assert(GetTickCount64()==ms[i]);
        assert(GetTickCount()==(uint32_t)ms[i]);
    }
    ok=0; assert(!GetTickCount64()); ok=1;
    /* Exhaustive small pool layouts: no head/tail allocation enters the gap. */
    for(size_t total=8;total<64;total++)
      for(size_t lo=1;lo<total-1;lo++)
        for(size_t hi=lo+1;hi<total;hi++)
          for(size_t n=1;n<=total;n++) {
            for(size_t cursor=0;cursor<=total;cursor++) {
              size_t head=ios_pool_hole_head_place(cursor,n,lo,hi);
              if(head<=total && n<=total-head)
                assert(head>=hi || head+n<=lo);
              size_t r=ios_pool_hole_tail_start(total,cursor,n,lo,hi);
              if(r<=total && n<=total-r) {
                size_t end=total-r, start=end-n;
                assert(end<=lo || start>=hi);
              }
            }
          }
    /* Real Party Animals layout: a 51MB phonon image can use the second run. */
    const size_t MB=1024*1024;
    size_t head=ios_pool_hole_head_place(512*MB,51*MB,560*MB,571*MB);
    assert(head==571*MB && head+51*MB<=896*MB-48*MB);
    assert(ios_pool_hole_between(896*MB,512*MB,48*MB,560*MB,571*MB)==11*MB);
    assert(!ios_pool_hole_between(896*MB,622*MB,48*MB,560*MB,571*MB));
    /* A failed tail reservation cannot rewind past another concurrent carve. */
    volatile size_t reserved=120;
    ios_pool_tail_unreserve(&reserved,100,20,1); assert(reserved==120);
    ios_pool_tail_unreserve(&reserved,120,20,1); assert(reserved==100);
    ios_pool_tail_unreserve(&reserved,100,20,0); assert(reserved==80);
    puts("PASS: native boot clock, timer wrap/sleep, split pool gaps and concurrent rollback");
    return 0;
}
'''
cc = os.environ.get('MD_RUNTIME_CC') or shutil.which('clang') or shutil.which('cc')
if not cc:
    raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-startup-fixes-') as folder:
    folder = Path(folder)
    source = folder / 'check.c'
    exe = folder / ('check.exe' if os.name == 'nt' else 'check')
    source.write_text(prefix + helpers + checks, encoding='utf-8')
    subprocess.run([cc, '-std=c11', '-O2', str(source), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
