#!/usr/bin/env python3
"""Exercise real 4KB metadata, guard-fault policy, scalar decoder and alias policy.

The first run controls Mach mapping failures. On Apple Silicon a second run uses
real shared Mach aliases of a PROT_NONE page. Neither run proves Chrome startup.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')


def function(signature):
    start = source.index(signature)
    opening = source.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ios_guard_access.h"
#ifdef REAL_MACH
#define host_page_size apple_host_page_size
#include <mach/mach.h>
#include <mach/mach_vm.h>
#undef host_page_size
#include <sys/mman.h>
#else
typedef uint64_t mach_vm_address_t;
typedef int vm_prot_t;
#define mach_task_self() 1
#define KERN_SUCCESS 0
#define VM_FLAGS_ANYWHERE 1
#define VM_INHERIT_NONE 0
#define VM_PROT_READ 1
#define VM_PROT_WRITE 2
#endif
typedef uint8_t BYTE;
typedef uint32_t NTSTATUS;
typedef uint64_t ULONG_PTR;
typedef size_t SIZE_T;
typedef int BOOL;
typedef struct {void *Peb;} TEB;
#define TRUE 1
#define FALSE 0
#define _WIN64 1
#define VPROT_READ 1
#define VPROT_WRITE 2
#define VPROT_EXEC 4
#define VPROT_WRITECOPY 8
#define VPROT_GUARD 16
#define VPROT_COMMITTED 32
#define VPROT_WRITEWATCH 64
#define STATUS_SUCCESS 0
#define STATUS_ACCESS_VIOLATION 0xc0000005u
#define STATUS_GUARD_PAGE_VIOLATION 0x80000001u
#define EXCEPTION_READ_FAULT 0
#define EXCEPTION_WRITE_FAULT 1
#ifndef PROT_READ
#define PROT_READ 1
#define PROT_WRITE 2
#endif
#define ROUND_ADDR(a,m) ((void *)((uintptr_t)(a)&~(uintptr_t)(m)))
#define ROUND_SIZE(a,s,m) (((s)+((uintptr_t)(a)&(m))+(m))&~(m))
static const SIZE_T page_size=4096,page_shift=12,page_mask=4095,limit_4g=0x100000000ull;
static SIZE_T host_page_size=16384,host_page_mask=16383;
static const SIZE_T pages_vprot_shift=20,pages_vprot_mask=0xfffff,pages_vprot_size=32768;
static BYTE *pages_vprot[32768];
static BYTE bytes[16384],before[16384],alias_bytes[16384];
static uintptr_t guest_base=0x7100000000ull,host;
static int virtual_mutex,locked,callbacks,remaps,unmaps,fail_stage;
static ULONG_PTR ios_wow_base_for_peb(void *peb) {return (uintptr_t)peb;}
static void mutex_lock(int *m) {(void)m;assert(!locked);locked=1;}
static void mutex_unlock(int *m) {(void)m;assert(locked);locked=0;}
static void *find_view(const void *address,SIZE_T size) {
    assert(locked);return fail_stage!=4 && (uintptr_t)address>=host && (uintptr_t)address+size<=host+16384?(void *)1:NULL;
}
/*PROTECTIONS*/
static void set_page_vprot_bits(void *p,SIZE_T n,BYTE set,BYTE clear) {
    for(uintptr_t a=(uintptr_t)p;a<(uintptr_t)p+n;a+=page_size)
        pages_vprot[(a>>12)>>20][(a>>12)&0xfffff]=(get_page_vprot((void *)a)&~clear)|set;
}
static int mprotect_range(void *p,SIZE_T n,int a,int b) {(void)p;(void)n;(void)a;(void)b;return 0;}
static int get_unix_prot(BYTE b) {return (b&VPROT_GUARD)?0:((b&VPROT_COMMITTED)?b&3:0);}
static int is_write_watch_range(void *p,int n) {(void)p;(void)n;return 0;}
struct thread_stack_info {int unused;};
static int is_inside_thread_stack_teb(void *p,struct thread_stack_info *s,TEB *t) {(void)p;(void)s;(void)t;return 0;}
static NTSTATUS grow_thread_stack_teb(char *p,struct thread_stack_info *s,TEB *t) {(void)p;(void)s;(void)t;abort();}
typedef struct {NTSTATUS ExceptionCode;uintptr_t ExceptionInformation[3];} EXCEPTION_RECORD;
#define dprintf(...) ((void)0)
/*BASELINE*/
#ifndef REAL_MACH
static int mach_vm_remap(int a,mach_vm_address_t *out,SIZE_T n,int mask,int flags,int task,
                         mach_vm_address_t src,int copy,vm_prot_t *cur,vm_prot_t *max,int inherit) {
    (void)a;(void)mask;(void)flags;(void)task;(void)copy;(void)inherit;
    assert(locked && n==16384 && src==host);++remaps;
    if(fail_stage==1)return 5;
    memcpy(alias_bytes,bytes,sizeof bytes);*out=(uintptr_t)alias_bytes;*cur=0;
    *max=fail_stage==2?VM_PROT_READ:3;return 0;
}
static int mach_vm_protect(int task,mach_vm_address_t p,SIZE_T n,int set_max,vm_prot_t prot) {
    (void)task;(void)p;(void)n;assert(!set_max && !(prot&4));return fail_stage==3?5:0;
}
static int mach_vm_deallocate(int task,mach_vm_address_t p,SIZE_T n) {
    (void)task;(void)p;assert(locked && n==16384);++unmaps;
    if(callbacks)memcpy(bytes,alias_bytes,sizeof bytes);return 0;
}
#endif
/*ALIAS_POLICY*/
static struct ios_guard_access op;
static uint64_t regs[31];
static void callback(void *alias,void *opaque) {
    assert(locked && opaque==regs);++callbacks;ios_guard_execute(&op,alias,regs);
}
static void fresh(void) {
    memset(bytes,0xa5,sizeof bytes);memcpy(before,bytes,sizeof bytes);
    for(unsigned i=0;i<4;++i)set_page_vprot_bits((void *)(host+i*4096),4096,35,255);
    set_page_vprot_bits((void *)(host+4096),4096,VPROT_GUARD,0);
    memset(regs,0,sizeof regs);callbacks=remaps=unmaps=fail_stage=locked=0;
}
static void run_access(uint64_t address,unsigned width,int write,int expected) {
    TEB target={(void *)guest_base};
    op=(struct ios_guard_access){address,width,1,31,0,0,write?IOS_GUARD_STORE:IOS_GUARD_LOAD};
    assert(ios_virtual_guard_neighbor_access(&target,(void *)(uintptr_t)address,width,write,callback,regs)==expected);
    assert(!locked && callbacks==expected);
}
static void decoder_tests(void) {
    uint64_t r[31]={0};struct ios_guard_access a;uint64_t value;
    r[24]=host+0x3ff4;
    assert(ios_guard_decode(0x889fff1fu,r,0,&a)); /* exact930 STLR wzr,[x24] */
    assert(a.address==host+0x3ff4 && a.width==4 && a.kind==IOS_GUARD_STORE_RELEASE && a.rt==31);
    value=UINT64_MAX;ios_guard_execute(&a,&value,r);assert(value==0xffffffff00000000ull);
    r[24]=host+0x3ff0;
    assert(ios_guard_decode(0xc8dfff04u,r,0,&a)); /* LDAR x4,[x24] */
    value=0x8765432101234567ull;ios_guard_execute(&a,&value,r);assert(r[4]==value);
    r[24]++;assert(!ios_guard_decode(0xc8dfff04u,r,0,&a));r[24]=host+0x3ff4;
    assert(!ios_guard_decode(0x8800ff04u,r,0,&a)); /* STLXR never fakes an exclusive monitor */
    assert(!ios_guard_decode(0xb8e00304u,r,0,&a)); /* LDADD must remain atomic, not memcpy */
    assert(!ios_guard_decode(0xd503201fu,r,0,&a)); /* NOP */
    assert(ios_guard_decode(0xb81fc304u,r,0,&a)); /* STUR w4,[x24,#-4] */
    assert(a.address==r[24]-4 && a.width==4 && a.kind==IOS_GUARD_STORE);
    assert(ios_guard_decode(0xb9800304u,r,0,&a)); /* LDRSW x4,[x24] */
    value=0x80000000u;ios_guard_execute(&a,&value,r);assert(r[4]==0xffffffff80000000ull);
    assert(ios_guard_decode(0x2900131fu,r,0,&a)); /* STP wzr,w4,[x24] */
    value=UINT64_MAX;r[4]=7;ios_guard_execute(&a,&value,r);assert(value==0x700000000ull);
    assert(ios_guard_decode(0xb90003ffu,r,host+0x3ff4,&a));assert(a.address==host+0x3ff4); /* SP != XZR */
    r[24]=UINT64_MAX-3;assert(!ios_guard_decode(0xb9000704u,r,0,&a)); /* address wrap */
}
int main(void) {
#ifdef REAL_MACH
    void *original=mmap(NULL,16384,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
    assert(original!=MAP_FAILED);host=(uintptr_t)original;guest_base=host&~0xffffffffull;
    assert(guest_base && !(host&16383));
#else
    host=guest_base+0x1680000;
#endif
    SIZE_T bucket=(host>>12)>>20;
    assert(bucket<pages_vprot_size);pages_vprot[bucket]=calloc(1,1<<20);assert(pages_vprot[bucket]);
    fresh();decoder_tests();
    assert((get_host_page_vprot((void *)(host+0x3ff4))&VPROT_GUARD) &&
           !(get_page_vprot((void *)(host+0x3ff4))&VPROT_GUARD));
    TEB target={(void *)guest_base};EXCEPTION_RECORD rec={0,{EXCEPTION_WRITE_FAULT,host+0x3ff4}};
    assert(ios_virtual_handle_fault_for_thread(&rec,&target)==STATUS_GUARD_PAGE_VIOLATION);
    assert(!(get_page_vprot((void *)(host+4096))&VPROT_GUARD));
    puts("REPRODUCED:930 production fault handler raises80000001 for legal4KB data and consumes a different4KB guard in the same16KB host page");
    fresh();
#ifdef REAL_MACH
    memcpy(original,bytes,sizeof bytes);assert(!mprotect(original,16384,PROT_NONE));
#endif
    run_access(host+0x3ff4,4,1,1);
    assert(get_page_vprot((void *)(host+4096))&VPROT_GUARD);
#ifdef REAL_MACH
    assert(!mprotect(original,16384,PROT_READ|PROT_WRITE));
    assert(!memcmp(original,before,0x3ff4));assert(!memcmp((char *)original+0x3ff4,"\0\0\0\0",4));
    assert(!memcmp((char *)original+0x3ff8,before+0x3ff8,8));
    assert(!munmap(original,16384));
    puts("PASS: real Mach shared alias writes a permitted subpage of PROT_NONE backing; original guard metadata and all neighbour bytes preserved");
#else
    assert(remaps==1 && unmaps==1 && !memcmp(bytes,before,0x3ff4));
    assert(!memcmp(bytes+0x3ff4,"\0\0\0\0",4));
    for(unsigned i=0;i<4;++i) {
        fresh();run_access(host+i*4096+8,4,1,i!=1);
    }
    fresh();run_access(host+4094,4,1,0); /* span enters guard */
    fresh();run_access(host+8190,4,0,0); /* starts inside guard */
    fresh();set_page_vprot_bits((void *)(host+12288),4096,0,VPROT_COMMITTED);run_access(host+0x3ff4,4,1,0);
    fresh();set_page_vprot_bits((void *)(host+12288),4096,0,VPROT_WRITE);run_access(host+0x3ff4,4,1,0);
    fresh();set_page_vprot_bits((void *)(host+12288),4096,0,VPROT_WRITE);run_access(host+0x3ff4,4,0,1);
    fresh();set_page_vprot_bits((void *)(host+12288),4096,VPROT_WRITEWATCH,0);run_access(host+0x3ff4,4,1,0);
    fresh();run_access(host+0x100000000ull+0x3ff4,4,1,0); /* another process */
    fresh();set_page_vprot_bits((void *)(host+4096),4096,0,VPROT_GUARD);run_access(host+0x3ff4,4,1,0);
    for(int fail=1;fail<=4;++fail) {
        fresh();fail_stage=fail;run_access(host+0x3ff4,4,1,0);
        assert(unmaps==(fail==2 || fail==3) && !memcmp(bytes,before,sizeof bytes));
    }
    puts("PASS:4KB subpage matrix, exact930 WZR store, acquire/release, sign extension, pair spans, SP, wrap, original guards, foreign windows, denied pages and all Mach failure exits");
#endif
    free(pages_vprot[bucket]);return 0;
}
'''

text = fixture.replace('/*PROTECTIONS*/', '\n'.join(function(s) for s in
    ['static BYTE get_page_vprot(', 'static BYTE get_host_page_vprot(']))
text = text.replace('/*BASELINE*/', function('NTSTATUS ios_virtual_handle_fault_for_thread('))
text = text.replace('/*ALIAS_POLICY*/', function('BOOL ios_virtual_guard_neighbor_access('))
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-guard-neighbor-') as temp:
    work = Path(temp)
    c = work / 'guard-neighbor.c'
    c.write_text(text, encoding='utf-8')
    for real in ([False, True] if os.uname().sysname == 'Darwin' else [False]) if hasattr(os, 'uname') else [False]:
        exe = work / ('real-mach' if real else 'model')
        if os.name == 'nt': exe = exe.with_suffix('.exe')
        flags = ['-DREAL_MACH'] if real else []
        if os.name != 'nt' and not real: flags += ['-fsanitize=address,undefined']
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-Wno-unused-function', '-Wno-unused-variable', '-I', str(root/'build/ntdll-unix'),
                        *flags, str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True, timeout=30)
print('NOT PROVEN: exact930 TEB bounds/subpage bytes or physical Chrome startup; the guard-neighbour cause remains conditional until a device fault is classified')
