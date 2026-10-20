#!/usr/bin/env python3
"""Run actual Wine memory-protect and trampoline code at dangerous boundaries."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
compiler = os.environ.get('CC') or shutil.which('clang')
if not compiler:
    raise SystemExit('clang is required')


def function(source, signature):
    start = source.index(signature)
    brace = source.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


source = (root / 'wine/dlls/ntdll/signal_arm64ec.c').read_text(encoding='utf-8')
protect = function(source, 'NTSTATUS SYSCALL_API NtProtectVirtualMemory(')
fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
typedef void *HANDLE, *PVOID;
typedef size_t SIZE_T;
typedef unsigned ULONG;
typedef uintptr_t ULONG_PTR;
typedef int BOOL, NTSTATUS;
#define SYSCALL_API
#define FALSE 0
#define TRUE 1
#define PAGE_EXECUTE 0x10
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define CrossProcessPreVirtualProtect 1
#define CrossProcessPostVirtualProtect 2
#define ERR(...) ((void)0)
static unsigned entered, enters, leaves, syscalls, pre, post, cross_pre, cross_post;
static NTSTATUS next_status;
static BOOL enter_syscall_callback(void) {
    if(entered) return 0;
    entered=1; ++enters; return 1;
}
/* This fixture represents initialized EC threads. The separate native-context
 * regression executes the production process gate for absent CPU areas too. */
#define enter_process_syscall_callback(is_current) enter_syscall_callback()
static void leave_syscall_callback(void) { assert(entered); entered=0; ++leaves; }
static BOOL RtlIsCurrentProcess(HANDLE h) { return h==(HANDLE)1; }
static BOOL ios_in_big_image(ULONG_PTR p) { (void)p; return 0; }
static BOOL ios_bulk_protect_suppressed(const char *via,void *p,SIZE_T n,ULONG prot) {
    (void)via; (void)p; return n>=(1ull<<30) && !(prot&0xf0);
}
static NTSTATUS syscall_NtProtectVirtualMemory(HANDLE h,void **p,SIZE_T *n,ULONG prot,ULONG *old) {
    (void)h; (void)p; (void)n; (void)prot; *old=4; ++syscalls; return next_status;
}
static void notify(void *p,SIZE_T n,ULONG prot,BOOL after,NTSTATUS st) {
    (void)p; (void)n; (void)prot; (void)st; assert(entered);
    if(after) ++post; else ++pre;
}
static void (*pNotifyMemoryProtect)(void *,SIZE_T,ULONG,BOOL,NTSTATUS)=notify;
static void send_cross_process_notification(HANDLE h,int kind,void *p,SIZE_T n,int count,ULONG prot,NTSTATUS st) {
    (void)h; (void)p; (void)n; (void)count; (void)prot; (void)st; assert(entered);
    if(kind==1) ++cross_pre; else ++cross_post;
}
'''
fixture += protect
fixture += r'''
int main(void) {
    void *p=(void *)0x10000; SIZE_T n=4096; ULONG old;
    /* Success/failure and current/foreign paths across the 64-request boundary. */
    for(unsigned i=0;i<70;i++) {
        next_status=(i%3)?0:-1;
        HANDLE h=(i%2)?(HANDLE)1:(HANDLE)2;
        assert(NtProtectVirtualMemory(h,&p,&n,0x40,&old)==next_status);
        assert(!entered && enters==i+1 && leaves==i+1 && old==4);
    }
    assert(pre==35 && post==35 && cross_pre==35 && cross_post==35);
    /* Subsequent non-executable transitions still notify the emulator. */
    next_status=0;
    assert(!NtProtectVirtualMemory((HANDLE)1,&p,&n,4,&old));
    assert(pre==36 && post==36 && !entered);
    n=1ull<<30;
    assert(!NtProtectVirtualMemory((HANDLE)1,&p,&n,4,&old));
    assert(pre==36 && post==36 && !entered); /* retain bulk protect suppression */
    pNotifyMemoryProtect=NULL; n=4096;
    assert(!NtProtectVirtualMemory((HANDLE)1,&p,&n,0x40,&old));
    assert(!entered && enters==73 && leaves==73);
    /* Recursion belongs to the outer callback; never clear its flag. */
    entered=1;
    assert(!NtProtectVirtualMemory((HANDLE)1,&p,&n,0x40,&old));
    assert(entered && enters==73 && leaves==73 && syscalls==74);
    return 0;
}
'''
virtual = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
# The corrected callback must not expose the old FEX interval-lock reentry
# during loader-boundary image notifications. Execute Wine's existing guard
# with an emulator hook that allocates while handling the image notification.
image = fixture[:fixture.index('int main(void) {')]
image += r'''
#include <stdlib.h>
#include <string.h>
static const char *environment;
static char *test_getenv(const char *key) { assert(!strcmp(key,"MADEIRA_IMAGE_MAP_GUARD")); return (char *)environment; }
static int test_setenv(const char *key,const char *value,int overwrite) {
    assert(!strcmp(key,"MADEIRA_IMAGE_MAP_GUARD"));
    if(overwrite || !environment) environment=value;
    return 0;
}
#define getenv(k) test_getenv(k)
#define setenv(k,v,o) test_setenv(k,v,o)
#include "MemoryCallbackPolicy.h"
typedef int LONG;
typedef unsigned short WCHAR;
typedef struct { unsigned short Length,MaximumLength; WCHAR *Buffer; } UNICODE_STRING;
static void RtlInitUnicodeString(UNICODE_STRING *s,const void *value) { (void)s; (void)value; }
static int RtlQueryEnvironmentVariable_U(void *p,UNICODE_STRING *name,UNICODE_STRING *value) {
    (void)p; (void)name;
    if(!environment) return -1;
    value->Length=*environment ? sizeof(WCHAR) : 0;
    value->Buffer[0]=(WCHAR)*environment; return 0;
}
static unsigned image_calls, observed_guard;
static void image_hook(void *base) {
    assert(base==(void *)0x10000); ++image_calls;
    observed_guard=entered;
    void *p=(void *)0x20000; SIZE_T n=4096; ULONG old;
    /* FEX's own heap commit must bypass notifications under the guard. */
    assert(!NtProtectVirtualMemory((HANDLE)1,&p,&n,4,&old));
}
static void (*pNotifyImageMap)(void *)=image_hook;
'''
image += function(source, 'void arm64ec_notify_image_map(')
image += r'''
int main(int argc,char **argv) {
    assert(argc==3);
    environment=!strcmp(argv[1],"unset") ? NULL : argv[1];
    const char *original=environment;
    madeira_prepare_memory_callbacks();
    assert(environment && !strcmp(environment,original ? original : "1"));
    unsigned guarded=environment[0]=='1';
    unsigned outer=!strcmp(argv[2],"nested");
    entered=outer;
    arm64ec_notify_image_map((void *)0x10000);
    assert(image_calls==1 && observed_guard==(guarded || outer));
    assert(entered==outer); /* preserve the outer callback's ownership */
    assert(pre==(!(guarded || outer)) && post==pre && syscalls==1);
    arm64ec_notify_image_map(NULL); assert(image_calls==1);
    pNotifyImageMap=NULL; arm64ec_notify_image_map((void *)0x10000); assert(image_calls==1 && entered==outer);
    return 0;
}
'''
constants = virtual[virtual.index('#define IOS_JIT_TRAMPOLINE_SIZE'):virtual.index('/* Allocate a per-thread trampoline slot.')]
tramp = r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static void *ios_jit_rw_base_global, *ios_jit_rx_base_global;
static unsigned invalidations, logs;
#define dprintf(...) (++logs)
static void sys_icache_invalidate(void *p,size_t n) {
    assert(n==8 && (char *)p>=(char *)ios_jit_rx_base_global &&
           (char *)p+n<=(char *)ios_jit_rx_base_global+0x4000);
    ++invalidations;
}
'''
tramp += constants
for name in ('int ios_jit_alloc_trampoline_slot(', 'void ios_jit_set_teb_slot(', 'void *ios_jit_get_trampoline('):
    tramp += '\n' + function(virtual, name)
tramp += r'''
int main(void) {
    uint64_t rw[2050]={0}, rx[2050]={0};
    ios_jit_rw_base_global=rw; ios_jit_rx_base_global=rx;
    rw[2048]=0x12345678; rw[2049]=0xabcdef;
    assert(IOS_JIT_MAX_SLOTS==1024);
    for(int i=0;i<1024;i++) {
        assert(ios_jit_alloc_trampoline_slot()==i);
        ios_jit_set_teb_slot(i,0x10000+(uintptr_t)i*0x1000);
        assert(ios_jit_get_trampoline(i)==(char *)rx+i*16+8);
        uint32_t *code=(uint32_t *)((char *)rw+i*16+8);
        assert(code[0]==0x58FFFFD2 && code[1]==0xD61F0220);
    }
    for(int i=0;i<1024;i++) assert(rw[i*2]==0x10000+(uintptr_t)i*0x1000);
    assert(rw[2048]==0x12345678 && rw[2049]==0xabcdef && invalidations==1024 && logs==1);
    ios_jit_set_teb_slot(-1,1); ios_jit_set_teb_slot(1024,1);
    assert(!ios_jit_get_trampoline(-1) && !ios_jit_get_trampoline(1024));
    /* Existing finite-page fallback retained; exhaustion emits one diagnostic. */
    assert(ios_jit_alloc_trampoline_slot()==0 && logs==2);
    assert(ios_jit_alloc_trampoline_slot()==0 && logs==2 && invalidations==1024);
    assert(rw[0]==0x10000 && rw[2048]==0x12345678);
    return 0;
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-compatibility-') as directory:
    for name, code in (('protect', fixture), ('trampoline', tramp), ('image-map', image)):
        src = Path(directory) / (name + '.c')
        binary = Path(directory) / (name + ('.exe' if os.name == 'nt' else ''))
        src.write_text(code, encoding='utf-8')
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-I', str(root / 'app/Madeira'), str(src), '-o', str(binary)], check=True)
        if name == 'image-map':
            for setting in ('unset', '0', '1', '', 'other'):
                for mode in ('normal', 'nested'):
                    subprocess.run([str(binary), setting, mode], check=True)
        else:
            subprocess.run([str(binary)], check=True)
bridge = (root / 'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
assert bridge.index('madeira_prepare_memory_callbacks();') > bridge.index('setenv(k.UTF8String, v.UTF8String, 1);')
print('PASS: production callback exits preserve notifications, recursion and errors; 1024 distinct TEB slots remain within the reserved page')
print('PASS: production loader guard blocks emulator heap reentry, preserves outer ownership and explicit opt-outs')
