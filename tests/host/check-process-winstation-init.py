#!/usr/bin/env python3
"""Run the production user32 initializer in Chrome924's child-before-parent order.

Shared native initialization must run once. Server-owned station/desktop handles
must exist in every pseudo-process, without replacing selected sandbox objects.
This isolates that contract; it does not assert full Chromium sandbox support.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
src = (root / 'build/win32u-unix/class_ios.c').read_text(encoding='utf-8')


def function(signature, source=src):
    start = source.index(signature)
    begin = source.index('{', start)
    depth = 0
    for i in range(begin, len(source)):
        depth += (source[i] == '{') - (source[i] == '}')
        if not depth:
            return source[start:i + 1]
    raise AssertionError(signature)


initialize = function('NTSTATUS WINAPI NtUserInitializeClientPfnArrays(')
stage = 'pthread_mutex_lock( &process_init_lock );'
assert initialize.index('pthread_mutex_unlock') < initialize.index('pthread_once( ') < initialize.index(stage)
assert 'entry->user_init_pid != pid' in initialize
assert initialize.index('register_desktop_class();') > initialize.index(stage)
shared = function('static void init_user(')
assert 'winstation_init();' not in shared and 'register_desktop_class();' not in shared
old_src = subprocess.check_output(['git', '-C', str(root), 'show',
    '7f15a2ff3749b8538d901bcfc2bee2fe1a4988da:build/win32u-unix/class_ios.c'], text=True)
old_shared = function('static void init_user(', old_src)
old_initialize = function('NTSTATUS WINAPI NtUserInitializeClientPfnArrays(', old_src)
fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define WINAPI
#define STATUS_SUCCESS 0
#define NTUSER_NB_PROCS 3
#define IOS_MAX_CLIENT_PROCS 8
#define PTHREAD_ONCE_INIT 0
#define PTHREAD_MUTEX_INITIALIZER 0
#define SystemBasicInformation 0
#define dprintf(...) ((void)0)
typedef unsigned UINT,DWORD;
typedef int NTSTATUS;
typedef void *HINSTANCE;
typedef uintptr_t ntuser_client_func_ptr[1];
typedef int pthread_mutex_t,pthread_once_t;
struct winproc {uintptr_t procA,procW;};
struct ios_client_procs {void *peb; HINSTANCE user32_module; struct winproc procs[NTUSER_NB_PROCS]; DWORD user_init_pid;};
static struct ios_client_procs ios_client_procs_reg[IOS_MAX_CLIENT_PROCS];
static int ios_client_procs_count;
static HINSTANCE user32_module;
static struct winproc winproc_array[NTUSER_NB_PROCS];
static pthread_mutex_t winproc_lock;
static unsigned shared_calls,station_calls,codepage_calls,desktop_classes;
static unsigned startup_calls,session_calls,gdi_calls,sysparams_calls;
static unsigned current;
static int fail_create;
static unsigned locks;
static struct { void *Peb; struct {void *UniqueProcess;} ClientId; } teb;
static struct {uintptr_t station,desktop;} processes[4];
static int system_info;
#define NtCurrentTeb() (&teb)
#define HandleToULong(v) ((unsigned)(uintptr_t)(v))
static void select_process(unsigned p) {
    current=p; teb.Peb=(void*)(uintptr_t)(0x717ffd0000ull+p*0x100000000ull);
    teb.ClientId.UniqueProcess=(void*)(uintptr_t)(0x20+p*4);
}
static void pthread_mutex_lock(pthread_mutex_t *m) {
    unsigned bit=m==&winproc_lock?1:2; assert(!(locks&bit)); locks|=bit;
}
static void pthread_mutex_unlock(pthread_mutex_t *m) {
    unsigned bit=m==&winproc_lock?1:2; assert(locks&bit); locks&=~bit;
}
static void pthread_once(pthread_once_t *once,void (*init)(void)) {
    assert(!locks);
    if(!*once) { *once=1; ++shared_calls; init(); }
}
void ios_wow_fixup_peb64_ptrs(void) {++codepage_calls;}
static int NtQuerySystemInformation(int c,void *p,size_t n,void *r) {
    (void)c;(void)p;(void)n;(void)r;return 0;
}
static void init_startup_info(void) {++startup_calls;}
static void shared_session_init(void) {++session_calls;}
static void gdi_init(void) {++gdi_calls;}
static void sysparams_init(void) {++sysparams_calls;}
static void register_desktop_class(void) {++desktop_classes;}
static uintptr_t NtUserGetProcessWindowStation(void) {return processes[current].station;}
static unsigned GetCurrentThreadId(void) {return current+0x24;}
static uintptr_t NtUserGetThreadDesktop(unsigned tid) {
    assert(tid==GetCurrentThreadId());return processes[current].desktop;
}
static void winstation_init(void) {
    assert(!(locks&1));++station_calls;
    if(fail_create)return; // real creation failure must not become a fake handle
    if(!processes[current].station)processes[current].station=0x30+4*current;
    if(!processes[current].desktop)processes[current].desktop=0x80+4*current;
}
''' + shared + '\n' + initialize + r'''
int main(int argc,char **argv) {
    assert(argc==2);
    unsigned first=!strcmp(argv[1],"child-first")?1:0,second=1-first;
    ntuser_client_func_ptr a[NTUSER_NB_PROCS]={{0x1100},{0x1200},{0x1300}};
    ntuser_client_func_ptr w[NTUSER_NB_PROCS]={{0x2100},{0x2200},{0x2300}};
    ntuser_client_func_ptr workers[1]={{0}};
    select_process(first);
    assert(NtUserInitializeClientPfnArrays(a,w,workers,(void*)0x110000)==0);
    assert(processes[first].station && processes[first].desktop);
    select_process(second);
    assert(NtUserInitializeClientPfnArrays(a,w,workers,(void*)0x220000)==0);
#ifdef BASELINE
    // Chrome's CreateAltWindowStation returns error34 for this exact state.
    assert(!processes[second].station && !processes[second].desktop);
    assert(shared_calls==1 && station_calls==1);
    puts("PASS: old production initializer reproduces missing parent station / sandbox34");
    return 0;
#else
    assert(processes[second].station && processes[second].desktop);
    assert(shared_calls==1 && station_calls==2);
    assert(codepage_calls==1 && desktop_classes==2 && startup_calls==1);
    assert(session_calls==1 && gdi_calls==1 && sysparams_calls==1);
    // PFN callbacks and module identity remain scoped to each PEB.
    assert(ios_client_procs_count==2);
    assert(ios_client_procs_reg[0].peb!=ios_client_procs_reg[1].peb);
    assert(ios_client_procs_reg[0].user32_module==(void*)0x110000);
    assert(ios_client_procs_reg[1].user32_module==(void*)0x220000);
    assert(ios_client_procs_reg[1].procs[2].procA==0x1300);
    assert(ios_client_procs_reg[1].procs[2].procW==0x2300);
    // Loading/reinitializing user32 must not replace a selected alternate
    // station or desktop, nor repeatedly rerun shared initialization.
    processes[second].station=0x444;processes[second].desktop=0x888;
    for(unsigned i=0;i<100;++i)
        assert(!NtUserInitializeClientPfnArrays(a,w,workers,(void*)0x220000));
    assert(processes[second].station==0x444 && processes[second].desktop==0x888);
    assert(station_calls==2 && desktop_classes==2 && shared_calls==1 && ios_client_procs_count==2);
    // A process inheriting a station but lacking its thread desktop is connected.
    select_process(2);processes[2].station=0xaaa;
    assert(!NtUserInitializeClientPfnArrays(a,w,workers,(void*)0x330000));
    assert(processes[2].station==0xaaa && processes[2].desktop && station_calls==3);
    // Failed object creation stays failed; do not fabricate sandbox success.
    select_process(3);fail_create=1;
    assert(!NtUserInitializeClientPfnArrays(a,w,workers,(void*)0x440000));
    assert(!processes[3].station && !processes[3].desktop);
    assert(station_calls==4 && shared_calls==1 && !locks);
    // Recycled PEB, new server PID: re-register process-owned classes.
    teb.ClientId.UniqueProcess=(void*)0x100;fail_create=0;
    assert(!NtUserInitializeClientPfnArrays(a,w,workers,(void*)0x550000));
    assert(processes[3].station && processes[3].desktop && ios_client_procs_count==4);
    assert(station_calls==5 && desktop_classes==5 && shared_calls==1 && !locks);
    puts("PASS: production per-process station/desktop connection, child-first/main-first, Desktop/Message classes per PID, PEB reuse, PFN isolation, alternate-handle preservation and failure transparency");
    return 0;
#endif
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not compiler:
    raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-process-winstation-') as scratch:
    work = Path(scratch)
    for old in (True, False):
        source = work / ('baseline.c' if old else 'fixed.c')
        exe = work / ('baseline' if old else 'fixed')
        if os.name == 'nt':
            exe = exe.with_suffix('.exe')
        baseline = fixture.replace(shared, old_shared).replace(initialize, old_initialize)
        source.write_text(baseline if old else fixture, encoding='utf-8')
        subprocess.run([compiler, '-std=c11', '-O2', '-Werror',
                        *(['-DBASELINE'] if old else []), str(source), '-o', str(exe)], check=True)
        for order in (('child-first',) if old else ('child-first', 'main-first')):
            subprocess.run([str(exe), order], check=True)
print('NOT MEASURED: physical Chrome sandbox, browsing, sign-in, Sync or extensions')
