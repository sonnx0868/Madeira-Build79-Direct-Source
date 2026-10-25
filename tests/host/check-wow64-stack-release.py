#!/usr/bin/env python3
"""Run the production WoW stack-release block with different freeing/owning PEBs."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
new = (root/'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
old = subprocess.check_output(['git', '-C', str(root), 'show',
                              'ac9770e:build/ntdll-unix/virtual_ios.c'], text=True, encoding='utf-8')


def release_block(text):
    start = text.index('    if (wow_teb && (ptr = ULongToPtr( wow_teb->DeallocationStack )))',
                       text.index('void virtual_free_teb('))
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
typedef size_t SIZE_T;
typedef struct {void *Peb;} TEB;
typedef struct {uint32_t DeallocationStack;} WOW_TEB;
#define ULongToPtr(v) ((void *)(uintptr_t)(uint32_t)(v))
#define GetCurrentProcess() ((void *)(intptr_t)-1)
#define MEM_RELEASE 0x8000
#define WINE_IOS 1
static uintptr_t freeing_base,released;
static unsigned calls;
static uintptr_t ios_wow_base(void) {return freeing_base;}
static uintptr_t ios_wow_base_for_peb(void *peb) {return (uintptr_t)peb;}
static int NtFreeVirtualMemory(void *process,void **address,SIZE_T *size,unsigned type) {
    assert(process==GetCurrentProcess() && !*size && type==MEM_RELEASE);
    released=(uintptr_t)*address;++calls;return 0;
}
static void production(TEB *teb,WOW_TEB *wow_teb) {void *ptr;SIZE_T size;/*BLOCK*/}
int main(void) {
    uintptr_t bases[]={0,0x7100000000ull,0x7200000000ull,0x7300000000ull};
    for(unsigned owner=0;owner<4;++owner)for(unsigned caller=0;caller<4;++caller) {
        TEB target={(void *)bases[owner]};WOW_TEB wow={0x1680000};
        freeing_base=bases[caller];calls=0;released=0;production(&target,&wow);
#ifdef BASELINE
        assert(released==bases[caller]+wow.DeallocationStack && calls==1);
#else
        assert(released==bases[owner]+wow.DeallocationStack && calls==1);
#endif
        calls=0;wow.DeallocationStack=0;production(&target,&wow);assert(!calls);
        production(&target,NULL);assert(!calls);
    }
#ifdef BASELINE
    puts("REPRODUCED:930 free of previous thread uses freeing process window, targets a different process's stack at the same guest address");
#else
    puts("PASS:production stack release uses target PEB for all16 native/windowed owner-caller pairs; NULL stack and missing TEB32 free nothing");
#endif
    return 0;
}
'''
compiler = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler, 'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-wow-release-') as temp:
    for baseline in (True, False):
        c = Path(temp)/('baseline.c' if baseline else 'fixed.c')
        c.write_text(fixture.replace('/*BLOCK*/', release_block(old if baseline else new)), encoding='utf-8')
        exe = c.with_suffix('.exe' if os.name == 'nt' else '')
        flags = ['-DBASELINE'] if baseline else []
        if os.name != 'nt': flags += ['-fsanitize=address,undefined']
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-Wno-unused-function', '-Wno-unused-parameter', *flags, str(c), '-o', str(exe)], check=True)
        subprocess.run([str(exe)], check=True, timeout=30)
