#!/usr/bin/env python3
"""Reproduce Chrome926's overlapping 32-bit child initialization in Wine C.

Execute init_peb and API-set publication from production. The allocation hook
deterministically boots a sibling during the first child's parameter allocation,
matching926's thread PEB730/global PEB790 evidence. No FEX core code is changed.
"""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
env = (root / 'build/ntdll-unix/env_ios.c').read_text(encoding='utf-8')
loader = (root / 'build/ntdll-unix/loader_ios.c').read_text(encoding='utf-8')
old_env = subprocess.check_output(['git', '-C', str(root), 'show', 'd784eea:build/ntdll-unix/env_ios.c'], text=True)
old_loader = subprocess.check_output(['git', '-C', str(root), 'show', 'd784eea:build/ntdll-unix/loader_ios.c'], text=True)


def function(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


assert 'WOW_PEB *saved_wow_peb' not in loader
signal = (root / 'build/ntdll-unix/signal_arm64_ios.c').read_text(encoding='utf-8')
assert 'PEB *peb = teb->Peb;' in function(signal, 'void init_syscall_frame(')
server = (root / 'build/ntdll-unix/server_ios.c').read_text(encoding='utf-8')
assert 'PEB *peb = NtCurrentTeb()->Peb;' in function(server, 'void server_init_process_done(')
assert 'PEB *peb = NtCurrentTeb()->Peb;' in function(env, 'static void load_global_options(')
fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WINE_IOS 1
#define _WIN64 1
#define VER_PLATFORM_WIN32_NT 2
#define STDERR_FILENO 2
#define dprintf(...) ((void)0)
#define ERR(...) ((void)0)
#define TRACE(...) ((void)0)
#define IOS_WOW_WINDOW_SIZE 0x100000000ull
typedef unsigned WORD,UINT;
typedef uintptr_t ULONG_PTR,SIZE_T;
typedef struct {unsigned Version;} API_SET_NAMESPACE;
typedef struct {unsigned MaximumLength;} STRING;
typedef struct {STRING ImagePathName;unsigned tag;} RTL_USER_PROCESS_PARAMETERS;
#define FIELDS(T) T ImageBaseAddress,ProcessParameters; \
    unsigned OSMajorVersion,OSMinorVersion,OSBuildNumber,OSPlatformId; \
    unsigned ImageSubSystem,ImageSubSystemMajorVersion,ImageSubSystemMinorVersion; \
    unsigned NumberOfProcessors,NtGlobalFlag; \
    struct {uint64_t QuadPart;} CriticalSectionTimeout; \
    uintptr_t HeapSegmentReserve,HeapSegmentCommit,HeapDeCommitTotalFreeThreshold,HeapDeCommitFreeBlockThreshold; \
    unsigned SessionId; T ApiSetMap
typedef void *PTR;
typedef struct {FIELDS(PTR);} PEB;
typedef struct {FIELDS(uint32_t);} PEB32,WOW_PEB;
static PEB *peb;
static WOW_PEB *wow_peb;
static const size_t page_size=0x1000,teb_offset=0x2000;
static struct {unsigned Machine,SubSystemType,MajorSubsystemVersion,MinorSubsystemVersion;} main_image_info;
static struct {PEB *Peb;unsigned WowTebOffset;struct {void *ExceptionList;} Tib;} tebs[3];
static unsigned current;
#define NtCurrentTeb() (&tebs[current])
static unsigned char *arenas[3];
static RTL_USER_PROCESS_PARAMETERS params[3];
static unsigned inject_params,inject_apiset,building;
static int is64[3];
static unsigned native_machine=0xaa64,current_machine=0xaa64;
static void init_peb(RTL_USER_PROCESS_PARAMETERS *,void *);
static void ios_child_load_apiset(WORD);
static void select_process(unsigned n) {
    current=n;peb=tebs[n].Peb;main_image_info.Machine=is64[n]?0xaa64:0x14c;
}
static ULONG_PTR ios_wow_base(void) {return is64[current]?0:(uintptr_t)arenas[current];}
static unsigned ios_wow_guest_addr(const void *p) {return (unsigned)((uintptr_t)p-ios_wow_base());}
static int ios_wow_in_window(const void *p) {
    return ios_wow_base() && (uintptr_t)p>=ios_wow_base() && (uintptr_t)p<ios_wow_base()+0x800000;
}
static int is_machine_64bit(unsigned m) {return m!=0x14c;}
void ios_exe_win_note_owner(void *m,void *p) {(void)m;(void)p;}
static unsigned GetCurrentProcessId(void) {return current+1;}
static unsigned GetCurrentThreadId(void) {return current+0x24;}
static void set_thread_id(void *t,unsigned p,unsigned id) {(void)t;(void)p;(void)id;}
static void virtual_set_large_address_space(void) {}
static void load_global_options(const void *s) {(void)s;}
static void ios_wow_map_user_shared_data(void) {}
static void *build_wow64_parameters(const RTL_USER_PROCESS_PARAMETERS *p) {
    unsigned owner=current;assert(!is64[owner] && p==&params[owner]);
    if(inject_params && !building) {
        building=1;select_process(1);init_peb(&params[1],arenas[1]+0x400000);
        current=owner;building=0; // TLS returns to owner; globals stay at sibling
    }
    return arenas[owner]+0x500000;
}
static const char *get_pe_dir(WORD m) {(void)m;return "i386-windows";}
static int NtCurrentProcess(void) {return -1;}
static void NtUnmapViewOfSection(int p,void *v) {(void)p;(void)v;}
static uintptr_t user_space_wow_limit;
static int map_apiset_schema(WORD m,void **v,SIZE_T *size,API_SET_NAMESPACE **map) {
    unsigned owner=current;(void)m;
    if(inject_apiset && !building) {
        building=1;select_process(1);ios_child_load_apiset(0x14c);
        current=owner;building=0;
    }
    *v=arenas[owner]+0x330000;*size=0x1000;*map=*v;(*map)->Version=6;return 0;
}
/*PRODUCTION*/
static WOW_PEB *wow(unsigned n) {return (void *)(arenas[n]+0x311000);}
static void reset(void) {
    wow_peb=NULL;inject_params=inject_apiset=building=0;
    for(unsigned n=0;n<3;++n) {
        memset(arenas[n],0,0x800000);is64[n]=0;
        tebs[n].Peb=(void *)(arenas[n]+0x310000);params[n].tag=n+1;
        tebs[n].Peb->ApiSetMap=arenas[n]+0x320000;
        ((API_SET_NAMESPACE *)tebs[n].Peb->ApiSetMap)->Version=6;
    }
    select_process(0);
}
int main(void) {
    for(unsigned n=0;n<3;++n) {arenas[n]=calloc(1,0x800000);assert(arenas[n]);}
    reset();inject_params=1;init_peb(&params[0],arenas[0]+0x400000);
#ifdef BASELINE
    assert(!wow(0)->ProcessParameters && wow(1)->ProcessParameters==0x500000);
    assert(!wow(0)->ImageBaseAddress);
    reset();inject_apiset=1;
    // Old callback has no process-local PEB and no stable PEB32 target.
    // Set the legacy pointer to the sibling, just as926's log records.
    wow_peb=wow(1);ios_child_load_apiset(0x14c);
    assert(!wow(0)->ApiSetMap && wow(1)->ApiSetMap==0x330000);
    puts("REPRODUCED: production926 overlapping child boot leaves PEB32.ProcessParameters NULL and publishes API-set to sibling");
#else
    assert(wow(0)->ProcessParameters==0x500000 && wow(1)->ProcessParameters==0x500000);
    assert(wow(0)->ImageBaseAddress==0x400000 && wow(1)->ImageBaseAddress==0x400000);
    assert(tebs[0].Peb->ProcessParameters==&params[0] && tebs[1].Peb->ProcessParameters==&params[1]);
    reset();inject_apiset=1;ios_child_load_apiset(0x14c);
    assert(wow(0)->ApiSetMap==0x330000 && wow(1)->ApiSetMap==0x330000);
    reset();wow_peb=wow(1);load_apiset_dll();
    assert(wow(0)->ApiSetMap==0x330000 && !wow(1)->ApiSetMap);
    // A64-bit sibling must not enter build_wow64_parameters, even while
    // the legacy session pointer still points at a32-bit child.
    reset();init_peb(&params[0],arenas[0]+0x400000);
    is64[1]=1;select_process(1);init_peb(&params[1],arenas[1]+0x400000);
    assert(!wow(1)->ProcessParameters && wow(0)->ProcessParameters==0x500000);
    // Reused window/PEB memory is initialized from this process's new params.
    select_process(0);memset(wow(0),0,sizeof(*wow(0)));
    init_peb(&params[0],arenas[0]+0x400000);assert(wow(0)->ProcessParameters==0x500000);
    for(unsigned n=0;n<1000;++n) {
        select_process(0);inject_params=1;init_peb(&params[0],arenas[0]+0x400000);
        assert(wow(0)->ProcessParameters==0x500000);
    }
    puts("PASS: production overlapping PEB32 parameter/API-set publication, parent/sibling ownership,64-bit exclusion and repeated/reused child boots");
#endif
    for(unsigned n=0;n<3;++n)free(arenas[n]);return 0;
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-peb-isolation-') as temp:
    work = Path(temp)
    for old in (True, False):
        production = function(old_env if old else env, 'static void init_peb(')
        production += '\n' + function(old_loader if old else loader, 'static void load_apiset_dll(')
        production += '\n' + function(old_loader if old else loader, 'static void ios_child_load_apiset(')
        c = work / ('baseline.c' if old else 'fixed.c')
        exe = c.with_suffix('.exe' if os.name == 'nt' else '')
        c.write_text(fixture.replace('/*PRODUCTION*/', production), encoding='utf-8')
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-Wno-unused-function', '-Wno-unused-variable',
                        *([] if os.name == 'nt' else ['-fsanitize=address,undefined']),
                        *(['-DBASELINE'] if old else []), str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True, timeout=30)
print('NOT MEASURED: physical Chrome startup or Google account Sync')
