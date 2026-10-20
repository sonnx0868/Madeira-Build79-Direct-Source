#!/usr/bin/env python3
"""Execute Wine's actual callback gates/wrappers for native and ARM64EC callers."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
cc = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert cc, 'C compiler required'
pin = subprocess.check_output(['git', '-C', str(root), 'rev-parse', 'HEAD:wine'], text=True).strip()
relative = 'dlls/ntdll/signal_arm64ec.c'
original = subprocess.check_output(['git', '-C', str(root / 'wine'), 'show', pin + ':' + relative]).decode()


def function(source, signature):
    start = source.index(signature)
    at = source.index('{', start)
    depth, end = 1, at + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end] + '\n'


prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
typedef void *HANDLE, *PVOID;
typedef uintptr_t ULONG_PTR;
typedef size_t SIZE_T;
typedef unsigned ULONG, UINT;
typedef int BOOL, NTSTATUS, LONG, SECTION_INHERIT;
typedef long long LARGE_INTEGER;
typedef struct { unsigned char InSyscallCallback; } CHPE_V2_CPU_AREA_INFO;
typedef struct { uintptr_t unused; } MEM_EXTENDED_PARAMETER;
typedef struct { ULONG State, Protect; void *BaseAddress; SIZE_T RegionSize; } MEMORY_BASIC_INFORMATION;
typedef unsigned short WCHAR;
typedef struct { unsigned short Length, MaximumLength; WCHAR *Buffer; } UNICODE_STRING;
#define TRUE 1
#define FALSE 0
#define WINAPI
#define SYSCALL_API
#define NT_SUCCESS(s) ((s)>=0)
#define STATUS_SUCCESS 0
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define PAGE_EXECUTE 0x10
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define MemoryBasicInformation 0
enum { CrossProcessPreVirtualAlloc=1, CrossProcessPostVirtualAlloc,
       CrossProcessPreVirtualFree, CrossProcessPostVirtualFree,
       CrossProcessPreVirtualProtect, CrossProcessPostVirtualProtect, CrossProcessFlushCache };
enum { ALLOC, ALLOC_EX, FREE, PROTECT, FLUSH, UNMAP, UNMAP_EX, MAP, MAP_EX, OPS };
static CHPE_V2_CPU_AREA_INFO ec, *cpu_area;
static unsigned raw_calls, local_pre, local_post, foreign_pre, foreign_post, bad_native_calls;
static unsigned image_calls, nested_calls, raw_type, callback_status, callback_kind;
static BOOL recurse;
static NTSTATUS raw_status;
static const char *guard_setting;
static CHPE_V2_CPU_AREA_INFO *get_arm64ec_cpu_area(void) { return cpu_area; }
static BOOL RtlIsCurrentProcess(HANDLE h) { return h==(HANDLE)1; }
static HANDLE NtCurrentProcess(void) { return (HANDLE)1; }
static void *RtlGetCurrentPeb(void) { return (void *)0x12340000; }
static BOOL ios_in_big_image(ULONG_PTR p) { (void)p; return FALSE; }
static void log_discard(const char *format,...) { (void)format; }
#define ERR(...) log_discard(__VA_ARGS__)
static BOOL ios_bulk_protect_suppressed(const char *via,void *p,SIZE_T n,ULONG prot) {
    (void)via; (void)p; return n>=(1ull<<30) && !(prot&0xf0);
}
static NTSTATUS raw(ULONG type) { ++raw_calls; raw_type=type; return raw_status; }
static NTSTATUS syscall_NtAllocateVirtualMemory(HANDLE h,void **p,ULONG_PTR z,SIZE_T *n,ULONG t,ULONG prot) {
    (void)h; (void)p; (void)n; assert(z==0 && prot==4); return raw(t);
}
static NTSTATUS syscall_NtAllocateVirtualMemoryEx(HANDLE h,void **p,SIZE_T *n,ULONG t,ULONG prot,MEM_EXTENDED_PARAMETER *par,ULONG count) {
    (void)h; (void)p; (void)n; assert(prot==4 && !par && !count); return raw(t);
}
static NTSTATUS syscall_NtFreeVirtualMemory(HANDLE h,void **p,SIZE_T *n,ULONG t) {
    (void)h; (void)p; (void)n; return raw(t);
}
static NTSTATUS syscall_NtProtectVirtualMemory(HANDLE h,void **p,SIZE_T *n,ULONG prot,ULONG *old) {
    (void)h; (void)p; (void)n; *old=4; return raw(prot);
}
static NTSTATUS NtQueryVirtualMemory(HANDLE h,void *p,int type,MEMORY_BASIC_INFORMATION *m,SIZE_T n,SIZE_T *got) {
    (void)h;(void)p;(void)type;(void)m;(void)n;*got=0;return 0;
}
static NTSTATUS syscall_NtFlushInstructionCache(HANDLE h,const void *p,SIZE_T n) { (void)h;(void)p;(void)n;return raw(0); }
static NTSTATUS syscall_NtUnmapViewOfSection(HANDLE h,void *p) { (void)h;(void)p;return raw(0); }
static NTSTATUS syscall_NtUnmapViewOfSectionEx(HANDLE h,void *p,ULONG f) { (void)h;(void)p;assert(f==0);return raw(0); }
static NTSTATUS syscall_NtMapViewOfSection(HANDLE section,HANDLE h,void **p,ULONG_PTR z,SIZE_T commit,
                                          const LARGE_INTEGER *offset,SIZE_T *n,SECTION_INHERIT inherit,ULONG t,ULONG prot) {
    (void)section;(void)h;(void)p;(void)n;assert(!z && !commit && !offset && !inherit && prot==4);return raw(t);
}
static NTSTATUS syscall_NtMapViewOfSectionEx(HANDLE section,HANDLE h,void **p,const LARGE_INTEGER *offset,
                                            SIZE_T *n,ULONG t,ULONG prot,MEM_EXTENDED_PARAMETER *par,ULONG count) {
    (void)section;(void)h;(void)p;(void)n;assert(!offset && prot==4 && !par && !count);return raw(t);
}
static void ios_chrome_clear_view(void *p) { (void)p; }
static BOOL ios_chrome_lookup(HANDLE h) { (void)h;return FALSE; }
static void ios_chrome_set_view(HANDLE h,void *p) { (void)h;(void)p;assert(0); }
static void ios_log_unmap(void *p) { (void)p; }
static void ios_map_gate_probe(const char *s,HANDLE sec,HANDLE h,NTSTATUS st,void *p,SIZE_T n,int cur,void *a,int before,int entered) {
    (void)s;(void)sec;(void)h;(void)st;(void)p;(void)n;(void)cur;(void)a;(void)before;(void)entered;
}
static NTSTATUS NtAllocateVirtualMemoryEx(HANDLE,void **,SIZE_T *,ULONG,ULONG,MEM_EXTENDED_PARAMETER *,ULONG);
static void nested_allocation(void) {
    unsigned pre=local_pre,post=local_post;
    unsigned char owned=cpu_area ? cpu_area->InSyscallCallback : 0;
    void *p=(void *)0x30000;SIZE_T n=4096;
    assert(NtAllocateVirtualMemoryEx((HANDLE)1,&p,&n,MEM_RESERVE,4,NULL,0)==raw_status);
    assert(local_pre==pre && local_post==post);
    assert(!cpu_area || cpu_area->InSyscallCallback==owned);
    ++nested_calls;
}
static void local_callback(BOOL after,NTSTATUS status) {
    if(after) ++local_post; else ++local_pre;
    callback_status=(unsigned)status;
    /* Record the forbidden entry instead of invoking undefined NULL+0x38 access. */
    if(!cpu_area) { ++bad_native_calls; return; }
    assert(cpu_area->InSyscallCallback);
    if(recurse) nested_allocation();
}
static void alloc_hook(void *p,SIZE_T n,ULONG t,ULONG prot,BOOL after,NTSTATUS status) {
    (void)p;(void)n;(void)t;(void)prot;local_callback(after,status);
}
static void memory_hook(void *p,SIZE_T n,ULONG t,BOOL after,NTSTATUS status) {
    (void)p;(void)n;(void)t;local_callback(after,status);
}
static void unmap_hook(void *p,BOOL after,NTSTATUS status) { (void)p;local_callback(after,status); }
static void flush_hook(const void *p,SIZE_T n) { (void)p;(void)n;local_callback(TRUE,0); }
static void (*pNotifyMemoryAlloc)(void *,SIZE_T,ULONG,ULONG,BOOL,NTSTATUS)=alloc_hook;
static void (*pNotifyMemoryFree)(void *,SIZE_T,ULONG,BOOL,NTSTATUS)=memory_hook;
static void (*pNotifyMemoryProtect)(void *,SIZE_T,ULONG,BOOL,NTSTATUS)=memory_hook;
static void (*pNotifyUnmapViewOfSection)(void *,BOOL,NTSTATUS)=unmap_hook;
static void (*pBTCpu64FlushInstructionCache)(const void *,SIZE_T)=flush_hook;
static void notify_map_view_of_section(HANDLE h,void *p,SIZE_T n,ULONG alloc,ULONG prot,NTSTATUS *status) {
    (void)h;(void)p;(void)n;(void)alloc;(void)prot;local_callback(TRUE,*status);
}
static void send_cross_process_notification(HANDLE h,int kind,const void *p,SIZE_T n,int count,...) {
    (void)p;(void)n;(void)count;assert(h==(HANDLE)2);
    callback_kind=kind;
    if(kind==CrossProcessPreVirtualAlloc || kind==CrossProcessPreVirtualFree || kind==CrossProcessPreVirtualProtect) ++foreign_pre;
    else ++foreign_post;
    /* Mapping the work-list can allocate in the caller. It must not reenter FEX. */
    if(recurse) nested_allocation();
}
static void RtlInitUnicodeString(UNICODE_STRING *s,const void *value) { (void)s;(void)value; }
static int RtlQueryEnvironmentVariable_U(void *p,UNICODE_STRING *name,UNICODE_STRING *value) {
    (void)p;(void)name;
    if(!guard_setting) return -1;
    value->Length=*guard_setting ? sizeof(WCHAR) : 0;
    value->Buffer[0]=(WCHAR)*guard_setting;return 0;
}
static void image_hook(void *base) {
    assert(base==(void *)0x10000);++image_calls;
    if(cpu_area) assert(cpu_area->InSyscallCallback);
    /* The intervals-only image hook remains valid before EC thread setup. */
    if(!EXPECT_OLD || cpu_area) nested_allocation();
}
static void (*pNotifyImageMap)(void *)=image_hook;
'''

suffix = r'''
static void reset(int mode) {
    ec.InSyscallCallback=mode==2;cpu_area=mode ? &ec : NULL;
    raw_calls=local_pre=local_post=foreign_pre=foreign_post=bad_native_calls=0;
    image_calls=nested_calls=raw_type=callback_status=callback_kind=0;recurse=FALSE;raw_status=0;
}
static NTSTATUS invoke(unsigned op,HANDLE h,ULONG prot) {
    void *p=(void *)0x10000;SIZE_T n=4096;ULONG old=0;
    switch(op) {
    case ALLOC:return NtAllocateVirtualMemory(h,&p,0,&n,MEM_RESERVE,4);
    case ALLOC_EX:return NtAllocateVirtualMemoryEx(h,&p,&n,MEM_RESERVE,4,NULL,0);
    case FREE:return NtFreeVirtualMemory(h,&p,&n,0x8000);
    case PROTECT: { NTSTATUS st=NtProtectVirtualMemory(h,&p,&n,prot,&old);assert(old==4);return st; }
    case FLUSH:return NtFlushInstructionCache(h,p,n);
    case UNMAP:return NtUnmapViewOfSection(h,p);
    case UNMAP_EX:return NtUnmapViewOfSectionEx(h,p,0);
    case MAP:return NtMapViewOfSection((HANDLE)3,h,&p,0,0,NULL,&n,0,0,4);
    case MAP_EX:return NtMapViewOfSectionEx((HANDLE)3,h,&p,NULL,&n,0,4,NULL,0);
    default:assert(0);return -1;
    }
}
int main(void) {
    unsigned cases=0,baseline_faults=0;
    guard_setting="1";
    /* Native, initialized EC and recursive EC; current and foreign targets;
       successful and failed syscalls, executable and ordinary protections. */
    for(unsigned mode=0;mode<3;mode++) for(unsigned foreign=0;foreign<2;foreign++)
    for(unsigned op=0;op<OPS;op++) for(unsigned failed=0;failed<2;failed++)
    for(unsigned exec=0;exec<2;exec++) {
        reset(mode);raw_status=failed ? -37 : 0;
        assert(invoke(op,(HANDLE)(uintptr_t)(foreign+1),exec ? 0x40 : 4)==raw_status);
        assert(raw_calls==1);
        assert(!cpu_area || cpu_area->InSyscallCallback==(mode==2));
        unsigned active=mode!=2 && (mode || EXPECT_OLD || foreign);
        unsigned has_result=(op==FLUSH || op==MAP || op==MAP_EX) ? !failed : 1;
        unsigned local=active && !foreign && has_result;
        unsigned queued=active && foreign && op<=FLUSH && has_result;
        assert(local_post==local && local_pre==(local && op!=FLUSH && op!=MAP && op!=MAP_EX));
        assert(foreign_post==queued && foreign_pre==(queued && op!=FLUSH));
        assert(bad_native_calls==(mode==0 ? local_pre+local_post : 0));
        baseline_faults+=bad_native_calls;
        if(local_post && op!=MAP && op!=MAP_EX && op!=FLUSH) assert(callback_status==(unsigned)raw_status);
        ++cases;
    }
    assert(EXPECT_OLD ? baseline_faults>0 : baseline_faults==0);
    /* Both pre/post callback reentry preserve the outer flag and skip callbacks. */
    for(unsigned op=0;op<OPS;op++) {
        reset(1);recurse=TRUE;assert(!invoke(op,(HANDLE)1,4));
        assert(raw_calls==1+nested_calls && nested_calls==local_pre+local_post);
        assert(nested_calls && !ec.InSyscallCallback && !bad_native_calls);
    }
    /* A native caller still queues a foreign change, with safe local allocations. */
    if(!EXPECT_OLD) for(unsigned op=0;op<=FLUSH;op++) {
        reset(0);recurse=TRUE;assert(!invoke(op,(HANDLE)2,4));
        assert(foreign_post==1 && foreign_pre==(op!=FLUSH));
        assert(!local_pre && !local_post && !bad_native_calls);
        assert(raw_calls==1+foreign_pre+foreign_post);
    }
    /* Missing callback pointers do not skip the syscall or leave ownership set. */
    pNotifyMemoryAlloc=NULL;pNotifyMemoryFree=NULL;pNotifyMemoryProtect=NULL;
    pNotifyUnmapViewOfSection=NULL;pBTCpu64FlushInstructionCache=NULL;
    for(unsigned mode=0;mode<3;mode++) for(unsigned op=0;op<=UNMAP_EX;op++) {
        reset(mode);assert(!invoke(op,(HANDLE)1,0x40));
        assert(raw_calls==1 && !local_pre && !local_post && !bad_native_calls);
        assert(!cpu_area || ec.InSyscallCallback==(mode==2));
    }
    pNotifyMemoryAlloc=alloc_hook;
    /* Keep loader registration before ThreadInit, and under an existing guard. */
    for(unsigned mode=0;mode<3;mode++) {
        reset(mode);arm64ec_notify_image_map((void *)0x10000);
        assert(image_calls==1 && !local_pre && !local_post);
        assert(!cpu_area || ec.InSyscallCallback==(mode==2));
        arm64ec_notify_image_map(NULL);assert(image_calls==1);
    }
    reset(0);leave_syscall_callback();assert(!cpu_area);
    printf("PASS: %s production Wine: %u current/foreign/native/EC/status cases; NULL-area callback entries=%u; recursion, image-map and missing hooks preserved\n",
           EXPECT_OLD ? "baseline repro" : "patched",cases,baseline_faults);
    return 0;
}
'''

with tempfile.TemporaryDirectory(prefix='madeira-native-callback-') as temporary:
    folder = Path(temporary)
    path = folder / relative
    path.parent.mkdir(parents=True)
    path.write_text(original, encoding='utf-8', newline='\n')
    # Same preceding callback-exit fix as the published build; do not touch Wine.
    subprocess.run(['git', '-C', str(folder), 'apply', str(root / 'patches/wine-exec-callback.patch')], check=True)
    baseline = path.read_text(encoding='utf-8')
    patch = root / 'patches/wine-native-callback-context.patch'
    subprocess.run(['git', '-C', str(folder), 'apply', '--check', str(patch)], check=True)
    subprocess.run(['git', '-C', str(folder), 'apply', str(patch)], check=True)
    subprocess.run(['git', '-C', str(folder), 'apply', '--reverse', '--check', str(patch)], check=True)
    production = path.read_text(encoding='utf-8')
    for name, source in (('baseline', baseline), ('fixed', production)):
        code = '#define EXPECT_OLD ' + ('1' if name == 'baseline' else '0') + '\n' + prefix
        signatures = ['static inline BOOL enter_syscall_callback(', 'static inline void leave_syscall_callback(']
        if name == 'fixed':
            signatures.insert(1, 'static inline BOOL enter_process_syscall_callback(')
        signatures += ['NTSTATUS SYSCALL_API ' + n + '(' for n in (
            'NtAllocateVirtualMemory', 'NtAllocateVirtualMemoryEx', 'NtFreeVirtualMemory',
            'NtProtectVirtualMemory', 'NtFlushInstructionCache', 'NtUnmapViewOfSection',
            'NtUnmapViewOfSectionEx', 'NtMapViewOfSection', 'NtMapViewOfSectionEx')]
        signatures += ['void arm64ec_notify_image_map(']
        code += '\n'.join(function(source, s) for s in signatures) + suffix
        c = folder / (name + '.c')
        binary = folder / (name + ('.exe' if os.name == 'nt' else ''))
        c.write_text(code, encoding='utf-8')
        subprocess.run([cc, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', str(c), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True, timeout=10)

# The package verifier consumes actual ARM64 instructions, not marker strings.
# Exercise its two accepted paths and reject mutated paths that reach a callback,
# dereference NULL, return without a syscall, loop, or contain an unknown opcode.
spec = importlib.util.spec_from_file_location('native_callback_bundle', root / 'tools/ntdll-native-callback-bundle.py')
bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundle)
words = {0x100: 0x94000240, 0x104: 0xf94bc648, 0x108: 0xb4000108,
         0x10c: 0x39400509, 0x110: 0x350000c9, 0x114: 0x52800029,
         0x118: 0x39000509, 0x11c: 0xd63f0160, 0x120: 0xd65f03c0,
         0x124: 0xd503201f, 0x128: 0x140001f6}
for native in (True, False):
    result = bundle.prove_path(words.__getitem__, 0x100, 0x12c, 0x900, 0xa00, native)
    assert result['completed_paths'] == 1 and result['cpu_area_load_rvas'] == [0x104]
for at, opcode, expected in ((0x108, 0xb40000a8, 'indirect'),
                             (0x108, 0xb4000028, 'NULL context dereference'),
                             (0x128, 0xd65f03c0, 'exactly one raw syscall'),
                             (0x128, 0x14000000, 'cycle'),
                             (0x128, 0xa9005113, 'non-stack pair'),
                             (0x104, 0xfd4bc648, 'unrecognized instruction'),
                             (0x128, 0x00000000, 'unrecognized instruction')):
    broken = dict(words)
    broken[at] = opcode
    try:
        bundle.prove_path(broken.__getitem__, 0x100, 0x12c, 0x900, 0xa00, True)
    except AssertionError as error:
        assert expected in str(error), (expected, str(error))
    else:
        raise AssertionError(('compiled guard verifier accepted a broken path', hex(at), hex(opcode)))
bad_handle = dict(words)
bad_handle[0x100], bad_handle[0x104] = 0xd2800000, 0x9400023f
try:
    bundle.prove_path(bad_handle.__getitem__, 0x100, 0x12c, 0x900, 0xa00, True)
except AssertionError as error:
    assert 'verified pseudo handle' in str(error), str(error)
else:
    raise AssertionError('compiled guard verifier assumed a foreign handle was current')
print('PASS: ARM64 compiled-path verifier accepts native/recursive bypass and rejects callback, NULL access, omitted syscall, loop and unknown instructions')
