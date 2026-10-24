#!/usr/bin/env python3
"""Reproduce Chrome929's RtlWakeAddressAll stack guard in production Wine C.

Page-table/OS operations are controlled boundaries. Both real production fault
entry points, stack selection/growth and exception-frame placement execute.
No Chrome browsing or iPad execution is inferred from this host regression.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
old_source = subprocess.check_output(['git', '-C', str(root), 'show',
    'b3b4b97:build/ntdll-unix/virtual_ios.c'], text=True, encoding='utf-8')


def function(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char BYTE;
typedef uint32_t ULONG,UINT,NTSTATUS;
typedef uintptr_t ULONG_PTR,SIZE_T;
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#define STATUS_SUCCESS 0u
#define STATUS_ACCESS_VIOLATION 0xc0000005u
#define STATUS_GUARD_PAGE_VIOLATION 0x80000001u
#define STATUS_STACK_OVERFLOW 0xc00000fdu
#define STATUS_IN_PAGE_ERROR 0xc0000006u
#define STATUS_EXECUTABLE_MEMORY_WRITE 1
#define EXCEPTION_READ_FAULT 0
#define EXCEPTION_WRITE_FAULT 1
#define EXCEPTION_EXECUTE_FAULT 8
#define VPROT_READ 1
#define VPROT_WRITE 2
#define VPROT_EXEC 4
#define VPROT_COMMITTED 8
#define VPROT_GUARD 16
#define VPROT_WRITEWATCH 32
#define PROT_READ 1
#define PROT_WRITE 2
#define max(a,b) ((a)>(b)?(a):(b))
#define ULongToPtr(x) ((void *)(uintptr_t)(ULONG)(x))
#define PtrToUlong(x) ((ULONG)(uintptr_t)(x))
#define ROUND_ADDR(a,m) ((void *)((uintptr_t)(a)&~(uintptr_t)(m)))
static void fixture_log(const char *format,...) {(void)format;}
#define WARN(...) fixture_log(__VA_ARGS__)
#define ERR(...) fixture_log(__VA_ARGS__)
#define dprintf(fd,...) fixture_log(__VA_ARGS__)
typedef struct {void *Peb,*DeallocationStack;struct {void *StackLimit,*StackBase;} Tib;
    SIZE_T GuaranteedStackBytes;void *wow;} TEB;
typedef struct {ULONG DeallocationStack;struct {ULONG StackLimit,StackBase;} Tib;
    ULONG GuaranteedStackBytes;} WOW_TEB;
typedef struct {NTSTATUS ExceptionCode;unsigned NumberParameters;void *ExceptionAddress;ULONG_PTR ExceptionInformation[3];} EXCEPTION_RECORD;
static TEB *current;
#define NtCurrentTeb() current
static WOW_TEB *get_wow_teb(TEB *teb) {return teb->wow;}
static ULONG_PTR ios_wow_base_for_peb(void *p) {return (uintptr_t)p;}
static const SIZE_T page_size=4096;
static SIZE_T host_page_size=16384,host_page_mask=16383;
static int is_win64=1,virtual_mutex,locks,force_exec_prot,enable_write_exceptions;
static struct {int allow_writes;} thread_data;
static void *ntdll_get_thread_data(void) {return &thread_data;}
// Cast matches production field access without depending on Wine headers.
#define ntdll_get_thread_data() (&thread_data)
static void mutex_lock(int *m) {(void)m;assert(!locks);++locks;}
static void mutex_unlock(int *m) {(void)m;assert(locks==1);--locks;}
struct page {uintptr_t address;BYTE bits;};
static struct page pages[64];
static unsigned page_count,protects;
static struct page *page_at(const void *p) {
    uintptr_t a=(uintptr_t)p&~host_page_mask;
    for(unsigned i=0;i<page_count;++i)if(pages[i].address==a)return &pages[i];
    assert(page_count<64);pages[page_count]=(struct page){a,0};return &pages[page_count++];
}
static BYTE get_host_page_vprot(const void *p) {return page_at(p)->bits;}
static void set_page_vprot_bits(void *p,SIZE_T n,BYTE set,BYTE clear) {
    for(SIZE_T i=0;i<n;i+=host_page_size) {struct page *v=page_at((char *)p+i);v->bits=(v->bits|set)&~clear;}
}
static int mprotect_range(void *p,SIZE_T n,int a,int b) {(void)p;(void)n;(void)a;(void)b;++protects;return 0;}
static int get_unix_prot(BYTE b) {return (b&VPROT_GUARD)?0:(b&(PROT_READ|PROT_WRITE));}
static int is_inside_signal_stack(void *p) {(void)p;return 0;}
static int is_vprot_exec_write(BYTE b) {(void)b;return 0;}
static int is_write_watch_range(void *p,int n) {(void)p;(void)n;return 0;}
static int ios_wow_in_window(void *p) {(void)p;return 0;}
static int ios_range_writable(void *p,SIZE_T n) {
    uintptr_t start=(uintptr_t)p&~host_page_mask,end=(uintptr_t)p+n;
    for(uintptr_t a=start;a<end;a+=host_page_size)
        if(!(get_unix_prot(get_host_page_vprot((void *)a))&PROT_WRITE))return 0;
    return 1;
}
static void abort_thread(int code) {(void)code;assert(!"unexpected exception-stack abort");}
struct thread_stack_info {char *start,*limit,*end;SIZE_T guaranteed;BOOL is_wow;};
/*PRODUCTION*/
static TEB target,server;
static WOW_TEB wow;
static ULONG_PTR base;
static void fresh(ULONG_PTR b) {
    memset(pages,0,sizeof pages);page_count=protects=locks=0;base=b;
    target=(TEB){(void *)b,(void *)0x7038000000ull,{(void *)0x7038080000ull,(void *)0x7038200000ull},0,&wow};
    server=(TEB){NULL,(void *)0x150000000ull,{(void *)0x150008000ull,(void *)0x150020000ull},0,NULL};
    wow=(WOW_TEB){0x1600000,{0x1684000,0x1800000},0};
    current=&server;
    set_page_vprot_bits((void *)(b+0x1680000),host_page_size,VPROT_READ|VPROT_WRITE|VPROT_COMMITTED|VPROT_GUARD,0);
    set_page_vprot_bits((void *)(b+0x167c000),host_page_size,VPROT_READ|VPROT_WRITE,0);
    set_page_vprot_bits((void *)(b+0x1684000),host_page_size,VPROT_READ|VPROT_WRITE|VPROT_COMMITTED,0);
}
static NTSTATUS fault(ULONG_PTR address,int remote) {
    EXCEPTION_RECORD rec={0};rec.ExceptionInformation[0]=EXCEPTION_WRITE_FAULT;rec.ExceptionInformation[1]=address;
    if(remote)return ios_virtual_handle_fault_for_thread(&rec,&target);
    current=&target;return virtual_handle_fault(&rec,(void *)0x7038100000ull);
}
int main(void) {
    fresh(0x7100000000ull);
#ifdef BASELINE
    // Exact log929 host fault; Mach helper knows only the native stack.
    assert(fault(base+0x1683dec,1)==STATUS_GUARD_PAGE_VIOLATION);
    assert(wow.Tib.StackLimit==0x1684000 && target.Tib.StackLimit==(void *)0x7038080000ull);
    fresh(0x7100000000ull);
    // The ordinary handler's WoW branch compares unrebased guest bounds.
    assert(fault(base+0x1683dec,0)==STATUS_GUARD_PAGE_VIOLATION);
    puts("REPRODUCED:926/929 signal and Mach handlers misclassify shifted32-bit stack growth as80000001");
#else
    ULONG_PTR bases[]={0,0x7100000000ull,0x7300000000ull,0x7900000000ull};
    for(unsigned bi=0;bi<4;++bi)for(int remote=0;remote<2;++remote) {
        fresh(bases[bi]);TEB before_server=server;void *native_limit=target.Tib.StackLimit;
        assert(fault(base+0x1683dec,remote)==STATUS_SUCCESS);
        assert(wow.Tib.StackLimit==0x1680000 && target.Tib.StackLimit==native_limit);
        assert(!memcmp(&server,&before_server,sizeof server) && !locks && protects==2);
        assert(!(get_host_page_vprot((void *)(base+0x1680000))&VPROT_GUARD));
        assert(get_host_page_vprot((void *)(base+0x167c000))&VPROT_GUARD);
        assert(fault(base+0x167fff0,remote)==STATUS_SUCCESS); // next guard page
        assert(wow.Tib.StackLimit==0x167c000);
        // A foreign process's guard is never accepted as this thread's stack.
        fresh(bases[bi]);ULONG_PTR foreign=base+0x2800000;
        set_page_vprot_bits((void *)foreign,host_page_size,VPROT_READ|VPROT_WRITE|VPROT_COMMITTED|VPROT_GUARD,0);
        assert(fault(foreign+4,remote)==STATUS_GUARD_PAGE_VIOLATION);
        assert(wow.Tib.StackLimit==0x1684000);
        fresh(bases[bi]);foreign=base+0x100000000ull+0x1680000;
        set_page_vprot_bits((void *)foreign,host_page_size,VPROT_READ|VPROT_WRITE|VPROT_COMMITTED|VPROT_GUARD,0);
        assert(fault(foreign+0x3dec,remote)==STATUS_GUARD_PAGE_VIOLATION);
        assert(wow.Tib.StackLimit==0x1684000); // identical ESP in another window
        // Preserve real stack overflow and the uncommitted lower red page.
        fresh(bases[bi]);ULONG_PTR overflow=base+0x1604000;
        set_page_vprot_bits((void *)overflow,host_page_size,VPROT_READ|VPROT_WRITE|VPROT_COMMITTED|VPROT_GUARD,0);
        assert(fault(overflow+8,remote)==STATUS_STACK_OVERFLOW);
        assert(wow.Tib.StackLimit==0x1604000 && !get_host_page_vprot((void *)(base+0x1600000)));
        // Exception-frame placement must use the same translated bounds.
        fresh(bases[bi]);EXCEPTION_RECORD rec={0};void *sp=(void *)(base+0x1684100);
        current=remote?&server:&target;
        void *frame=remote?ios_virtual_setup_exception_for_thread(sp,4096,&rec,&target):virtual_setup_exception(sp,4096,&rec);
        assert(frame==(void *)(base+0x1683100) && wow.Tib.StackLimit==0x1680000 && !locks);
    }
    // Native64-bit behavior is unchanged and never writes a sibling TEB32.
    fresh(0x7100000000ull);target.wow=NULL;
    set_page_vprot_bits((void *)0x703807c000ull,host_page_size,VPROT_READ|VPROT_WRITE|VPROT_COMMITTED|VPROT_GUARD,0);
    assert(fault(0x703807c100ull,1)==STATUS_SUCCESS);
    assert(target.Tib.StackLimit==(void *)0x703807c000ull && wow.Tib.StackLimit==0x1684000);
    // Do not turn arbitrary access violations into successful stack growth.
    fresh(0x7100000000ull);assert(fault(base+0x2500000,1)==STATUS_ACCESS_VIOLATION);
    puts("PASS: production signal/Mach fault service, classic/shifted WoW64, target-TEB isolation, repeated guard growth, guest StackLimit, exception frames, real overflow and native64 preservation");
#endif
    return 0;
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-wow-stack-') as temp:
    work = Path(temp)
    for old in (True, False):
        text = old_source if old else source
        signatures = (['static BOOL is_inside_thread_stack(', 'static NTSTATUS grow_thread_stack(',
                       'NTSTATUS virtual_handle_fault(', 'static BOOL is_inside_thread_stack_teb(',
                       'static NTSTATUS grow_thread_stack_teb('] if old else
                      ['static char *ios_wow_stack_address(', 'static BOOL is_inside_thread_stack_teb(',
                       'static BOOL is_inside_thread_stack(', 'static NTSTATUS grow_thread_stack_teb(',
                       'static NTSTATUS grow_thread_stack(', 'NTSTATUS virtual_handle_fault('])
        signatures += ['NTSTATUS ios_virtual_handle_fault_for_thread(', 'void *virtual_setup_exception(',
                       'void *ios_virtual_setup_exception_for_thread(']
        production = '\n\n'.join(function(text, s) for s in signatures)
        c = work / ('baseline.c' if old else 'fixed.c')
        exe = c.with_suffix('.exe' if os.name == 'nt' else '')
        c.write_text(fixture.replace('/*PRODUCTION*/', production), encoding='utf-8')
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-Wno-unused-function', *([] if os.name == 'nt' else ['-fsanitize=address,undefined']),
                        *(['-DBASELINE'] if old else []), str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True, timeout=30)
print('NOT MEASURED: complete Chrome browsing, Crashpad transport or Google Sync on iPad')
