#!/usr/bin/env python3
"""Test real Wine stack creation with host-sized guards and WoW64 ULONG query.

On Darwin, apply the same page protection union to real mmap pages and execute
the permitted write at the same offset above StackLimit as Chrome932. No ARM64
fault decoder participates: preventing the mixed stack page removes that cost.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import re

root = Path(__file__).resolve().parents[2]
thread = (root/'wine/dlls/ntdll/thread.c').read_text(encoding='utf-8')
old = subprocess.check_output(['git','-C',str(root/'wine'),'show','HEAD:dlls/ntdll/thread.c'],text=True,encoding='utf-8')
system = (root/'wine/dlls/ntdll/unix/system.c').read_text(encoding='utf-8')
wow = (root/'wine/dlls/wow64/system.c').read_text(encoding='utf-8')


def function(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


query_case = system.split('    case SystemWineHostPageSizeInformation:',1)[1].split('    case SystemNativeBasicInformation:',1)[0]
assert 'sysconf( _SC_PAGESIZE )' in query_case
wow_group = wow.split('NTSTATUS WINAPI wow64_NtQuerySystemInformation(',1)[1]
wow_group = wow_group[wow_group.index('    case SystemPerformanceInformation:'):]
wow_group = wow_group[:wow_group.index('        return NtQuerySystemInformation( class, ptr, len, retlen );')]
assert 'case SystemWineHostPageSizeInformation:' in wow_group
wow_enums = ','.join(n+'='+str(100+i) for i,n in enumerate(re.findall(r'case (\w+):',wow_group)) if n!='SystemWineHostPageSizeInformation')

fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef REAL_PROTECTION
#include <unistd.h>
#include <sys/mman.h>
#define host_page_size apple_host_page_size
#include <mach/mach.h>
#include <mach/mach_vm.h>
#undef host_page_size
#else
#define _SC_PAGESIZE 1
#endif
typedef uintptr_t SIZE_T;
typedef uint32_t ULONG,NTSTATUS;
typedef struct {SIZE_T ReserveSize,ZeroBits;void *StackBase;} PROCESS_STACK_ALLOCATION_INFORMATION;
typedef struct {void *OldStackBase,*OldStackLimit,*StackBase,*StackLimit,*DeallocationStack;} INITIAL_TEB;
typedef struct {struct {SIZE_T SizeOfStackReserve,SizeOfStackCommit;} OptionalHeader;} IMAGE_NT_HEADERS;
static IMAGE_NT_HEADERS image_headers={{0x80000,4096}};
static struct {struct {void *ImageBaseAddress;} *Peb;} teb;
#define WINAPI
#define TRACE(...) ((void)0)
#define max(a,b) ((a)>(b)?(a):(b))
#define WINE_IOS 1
#define NtCurrentTeb() (&teb)
#define RtlImageNtHeader(p) ((void)(p),&image_headers)
#define GetCurrentProcess() ((void *)(intptr_t)-1)
#define ProcessThreadStackAllocation 41
#define SystemWineHostPageSizeInformation 1001
#define MEM_COMMIT 4096
#define MEM_RELEASE 32768
#define PAGE_READWRITE 4
#define PAGE_GUARD 256
#define STATUS_SUCCESS 0u
#define STATUS_INVALID_PARAMETER 0xc000000du
#define STATUS_NO_MEMORY 0xc0000017u
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004u
#define STATUS_ACCESS_VIOLATION 0xc0000005u
#define STATUS_INVALID_INFO_CLASS 0xc0000003u
#define STATUS_NOT_SUPPORTED 0xc00000bbu
static const SIZE_T page_size=4096;
static SIZE_T host_unit=16384,reserved;
static unsigned char *permissions;
static uintptr_t allocation;
static unsigned alloc_calls,commit_calls,free_calls;
static int fail_step,disable_query;
static long query_unit=16384;
#ifndef REAL_PROTECTION
static long fixture_sysconf(int name) {assert(name==_SC_PAGESIZE);return query_unit;}
#define sysconf fixture_sysconf
#endif
static NTSTATUS NtQuerySystemInformation(unsigned class,void *info,ULONG size,ULONG *ret_size) {
    NTSTATUS ret=STATUS_SUCCESS;ULONG len=0;
    if(disable_query)return STATUS_INVALID_INFO_CLASS;
    switch(class) {case SystemWineHostPageSizeInformation: /*QUERY*/ default:return STATUS_INVALID_INFO_CLASS;}
    if(ret_size)*ret_size=len;return ret;
}
enum {/*WOW_ENUMS*/};
static NTSTATUS wow_query(unsigned class,void *ptr,ULONG len,ULONG *retlen) {
    /* The production no-pointer-width-conversion case group ends here. */
    switch(class) {/*WOW_CASES*/
        return NtQuerySystemInformation( class, ptr, len, retlen );
    default:return STATUS_INVALID_INFO_CLASS;}
}
static NTSTATUS NtSetInformationProcess(void *process,int class,PROCESS_STACK_ALLOCATION_INFORMATION *a,SIZE_T size) {
    assert(process==GetCurrentProcess() && class==ProcessThreadStackAllocation && size==sizeof(*a));++alloc_calls;
    if(fail_step==1)return STATUS_NO_MEMORY;
    reserved=a->ReserveSize;assert(reserved<=0x1000000 && !(reserved&65535));
    permissions=calloc(reserved/4096,1);assert(permissions);
#ifdef REAL_PROTECTION
    void *p=mmap(NULL,reserved,PROT_NONE,MAP_PRIVATE|MAP_ANON,-1,0);
    assert(p!=MAP_FAILED);allocation=(uintptr_t)p;
#else
    allocation=0x1680000;
#endif
    a->StackBase=(void *)allocation;return 0;
}
static int host_writable(uintptr_t address) {
    SIZE_T offset=address-allocation,first=(offset/host_unit)*host_unit;unsigned prot=0;
    assert(address>=allocation && offset<reserved);
    for(SIZE_T i=first;i<first+host_unit && i<reserved;i+=4096)prot|=permissions[i/4096];
    return !(prot&2) && (prot&1);
}
static NTSTATUS NtAllocateVirtualMemory(void *process,void **address,SIZE_T z,SIZE_T *size,unsigned type,unsigned prot) {
    assert(process==GetCurrentProcess() && !z && type==MEM_COMMIT);++commit_calls;
    if(fail_step==(int)commit_calls+1)return STATUS_NO_MEMORY;
    SIZE_T offset=(uintptr_t)*address-allocation;
    assert(!(offset&4095) && !(*size&4095) && offset+*size<=reserved);
    for(SIZE_T i=offset;i<offset+*size;i+=4096)permissions[i/4096]=1|((prot&PAGE_GUARD)?2:0);
#ifdef REAL_PROTECTION
    SIZE_T start=(offset/host_unit)*host_unit,end=(offset+*size+host_unit-1)&~(host_unit-1);
    for(SIZE_T i=start;i<end;i+=host_unit)
        assert(!mprotect((void *)(allocation+i),host_unit,host_writable(allocation+i)?PROT_READ|PROT_WRITE:PROT_NONE));
#endif
    return 0;
}
static NTSTATUS NtFreeVirtualMemory(void *process,void **address,SIZE_T *size,unsigned type) {
    assert(process==GetCurrentProcess() && (uintptr_t)*address==allocation && !*size && type==MEM_RELEASE);++free_calls;
#ifdef REAL_PROTECTION
    assert(!munmap((void *)allocation,reserved));
#endif
    free(permissions);permissions=NULL;allocation=reserved=0;return 0;
}
/*CREATION*/
static void reset(void) {assert(!permissions);alloc_calls=commit_calls=free_calls=0;fail_step=disable_query=0;}
static void cleanup(void) {void *a=(void *)allocation;SIZE_T n=0;NtFreeVirtualMemory(GetCurrentProcess(),&a,&n,MEM_RELEASE);}
#ifdef REAL_PROTECTION
static int kernel_writable(uintptr_t address) {
    mach_vm_address_t p=address;mach_vm_size_t n=0;mach_port_t object=0;
    vm_region_basic_info_data_64_t info;mach_msg_type_number_t count=VM_REGION_BASIC_INFO_COUNT_64;
    assert(mach_vm_region(mach_task_self(),&p,&n,VM_REGION_BASIC_INFO_64,(vm_region_info_t)&info,&count,&object)==KERN_SUCCESS);
    if(object)mach_port_deallocate(mach_task_self(),object);
    return p<=address && address<p+n && (info.protection&VM_PROT_WRITE);
}
#endif
static void create_and_check(SIZE_T unit) {
    INITIAL_TEB stack;reset();host_unit=unit;query_unit=unit;
    assert(!RtlCreateUserStack(4096,0x400000,0,4096,65536,&stack));
    assert(stack.DeallocationStack==(void *)allocation && (uintptr_t)stack.StackBase==allocation+reserved);
#ifdef BASELINE
    assert((uintptr_t)stack.StackLimit==allocation+8192);
    assert(!host_writable((uintptr_t)stack.StackLimit+0x1ff4));
#else
    assert((uintptr_t)stack.StackLimit==allocation+2*unit);
    assert(host_writable((uintptr_t)stack.StackLimit+0x1ff4));
    assert((uintptr_t)stack.StackBase-(uintptr_t)stack.StackLimit>=0x400000-8192);
    assert(!host_writable(allocation) && !host_writable(allocation+unit));
#endif
#ifdef REAL_PROTECTION
    uintptr_t target=(uintptr_t)stack.StackLimit+0x1ff4;
#ifdef BASELINE
    assert(!kernel_writable(target));
#else
    assert(kernel_writable(target) && !kernel_writable(allocation) && !kernel_writable(allocation+unit));
    __atomic_store_n((uint32_t *)target,0,__ATOMIC_RELEASE);assert(!*(uint32_t *)target);
#endif
#endif
    cleanup();
}
int main(void) {
    static struct {void *ImageBaseAddress;} peb;teb.Peb=(void *)&peb;
#ifdef REAL_PROTECTION
    host_unit=(SIZE_T)sysconf(_SC_PAGESIZE);assert(host_unit==16384);
    create_and_check(host_unit);
#else
#ifdef BASELINE
    create_and_check(16384);
#else
    for(unsigned unit=4096;unit<=65536;unit*=2)create_and_check(unit);
    for(int fail=1;fail<=3;++fail) {
        INITIAL_TEB out,before;memset(&out,0xa5,sizeof out);before=out;reset();host_unit=query_unit=16384;fail_step=fail;
        assert(RtlCreateUserStack(4096,0x400000,0,4096,65536,&out)==STATUS_NO_MEMORY);
        assert(!permissions && !memcmp(&out,&before,sizeof out) && free_calls==(unsigned)(fail!=1));
    }
    reset();INITIAL_TEB out;assert(RtlCreateUserStack(4096,0x400000,0,0,65536,&out)==STATUS_INVALID_PARAMETER && !alloc_calls);
    reset();assert(RtlCreateUserStack(4096,~(SIZE_T)0,0,4096,65536,&out)==STATUS_NO_MEMORY && !alloc_calls);
    reset();host_unit=query_unit=16384;assert(!RtlCreateUserStack(4096,0x400000,0,4096,0x100000,&out));
    assert(!(reserved&0xfffff) && (uintptr_t)out.StackBase-(uintptr_t)out.StackLimit>=0x400000-8192);cleanup();
    reset();host_unit=4096;disable_query=1;assert(!RtlCreateUserStack(1,1,0,4096,65536,&out));
    assert((uintptr_t)out.StackLimit==allocation+8192);cleanup();
    for(unsigned bad=0;bad<3;++bad) {
        reset();host_unit=4096;query_unit=bad==0?2048:bad==1?12345:131072;
        assert(!RtlCreateUserStack(4096,0x100000,0,4096,65536,&out));assert((uintptr_t)out.StackLimit==allocation+8192);cleanup();
    }
    reset();query_unit=16384;struct {ULONG pre,value,post;} word={0x12345678,0,0x87654321};ULONG length=0;
    assert(!wow_query(SystemWineHostPageSizeInformation,&word.value,4,&length));
    assert(word.pre==0x12345678 && word.value==16384 && word.post==0x87654321 && length==4);
    assert(NtQuerySystemInformation(SystemWineHostPageSizeInformation,&word.value,8,&length)==STATUS_INFO_LENGTH_MISMATCH && length==4);
    assert(NtQuerySystemInformation(SystemWineHostPageSizeInformation,NULL,4,&length)==STATUS_ACCESS_VIOLATION);
#endif
#endif
#ifdef BASELINE
    puts("REPRODUCED:932 production RtlCreateUserStack uses4KB red/guard/data boundaries; Chrome's StackLimit+0x1ff4 lies in a16KB host guard");
#else
#ifdef REAL_PROTECTION
    puts("PASS: real Darwin stack pages; exact relative Chrome932 STLR write completes without any fault handler; red and guard pages remain inaccessible");
#else
    puts("PASS:production host-page user stacks at4KB..64KB, usable reserve preserved, all failure cleanups, legacy fallback, overflow and fixed-width WoW64 host-page query");
#endif
#endif
    return 0;
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-user-stack-') as tmp:
    for baseline in (True,False):
        text=fixture.replace('/*QUERY*/',query_case).replace('/*WOW_ENUMS*/',wow_enums).replace('/*WOW_CASES*/',wow_group).replace('/*CREATION*/',function(old if baseline else thread,'NTSTATUS WINAPI RtlCreateUserStack('))
        c=Path(tmp)/('baseline.c' if baseline else 'fixed.c');c.write_text(text,encoding='utf-8')
        modes=[False,True] if hasattr(os,'uname') and os.uname().sysname=='Darwin' else [False]
        for real in modes:
            exe=c.with_name(c.stem+('-real' if real else '-model')+('.exe' if os.name=='nt' else ''))
            flags=(['-DBASELINE'] if baseline else [])+(['-DREAL_PROTECTION'] if real else [])
            if os.name!='nt' and not real:flags+=['-fsanitize=address,undefined']
            subprocess.run([compiler,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-variable',*flags,str(c),'-o',str(exe)],check=True)
            subprocess.run([str(exe)],check=True,timeout=30)
print('NOT PROVEN: complete Chrome, renderer creation, Google Sync or extension execution on iPad')
