#!/usr/bin/env python3
"""Run the actual ARM64EC frame walker against alias and non-progress frames."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
pin = subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD:wine'],text=True).strip()
name = 'dlls/ntdll/signal_arm64ec.c'
original = subprocess.check_output(['git','-C',str(root/'wine'),'show',pin+':'+name]).decode()
def function(text):
    start = text.index('ULONG WINAPI RtlWalkFrameChain(')
    end = text.index('\n\n/*************************************************************************',start)
    return text[start:end]
with tempfile.TemporaryDirectory(prefix='madeira-stackwalk-') as temporary:
    folder = Path(temporary); path = folder/name; path.parent.mkdir(parents=True)
    path.write_text(original,encoding='utf-8')
    patch = root/'patches/wine-stackwalk-jit-alias.patch'
    subprocess.run(['git','-C',str(folder),'apply','--check',str(patch)],check=True)
    subprocess.run(['git','-C',str(folder),'apply',str(patch)],check=True)
    subprocess.run(['git','-C',str(folder),'apply','--reverse','--check',str(patch)],check=True)
    production = function(path.read_text(encoding='utf-8'))
    fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uintptr_t ULONG_PTR;
typedef unsigned ULONG;
typedef int RUNTIME_FUNCTION, PEXCEPTION_ROUTINE;
typedef struct { unsigned Count; } UNWIND_HISTORY_TABLE;
typedef struct { ULONG_PTR Rip,Rsp; unsigned ContextFlags; } CONTEXT;
typedef struct { struct { ULONG_PTR StackBase; } Tib; } TEB;
#define WINAPI
#define UNW_FLAG_NHANDLER 0
#define CONTEXT_UNWOUND_TO_CALL 1
static TEB teb={{0x9000}};
#define NtCurrentTeb() (&teb)
static CONTEXT seed;
static int aliases,scenario,lookups,unwinds,canonical,errors;
static ULONG_PTR lookup_pcs[16];
static RUNTIME_FUNCTION entry;
static void *xlate_ios_jit_rev(void *p) {
    ULONG_PTR n=(ULONG_PTR)p;
    return (void *)(aliases && n>=0x100000 && n<0x110000 ? n-0x100000 : n);
}
static void RtlCaptureContext(CONTEXT *p) { *p=seed; }
static int RtlIsEcCode(ULONG_PTR pc) { return pc>=0x1000 && pc<0x2000; }
static RUNTIME_FUNCTION *RtlLookupFunctionEntry(ULONG_PTR pc,ULONG_PTR *base,UNWIND_HISTORY_TABLE *table) {
    (void)table; *base=0x1000;
    if(lookups<16)lookup_pcs[lookups]=pc;lookups++;
    if(pc>=0x1000 && pc<0x6000) { canonical++;return &entry; }
    return NULL;
}
static int RtlVirtualUnwind2(int type,ULONG_PTR base,ULONG_PTR pc,RUNTIME_FUNCTION *f,CONTEXT *c,
                           void *m,void **data,ULONG_PTR *frame,void *a,void *b,void *d,PEXCEPTION_ROUTINE *h,int flags) {
    (void)type;(void)base;(void)pc;(void)m;(void)data;(void)a;(void)b;(void)d;(void)h;(void)flags;
    unwinds++;*frame=c->Rsp;
    if(errors)return 1;
    if(!f || scenario==1)return 0; // native leaf fallback repeats the captured caller
    if(scenario==2){*frame=0;return 0;}
    if(scenario==3){c->Rsp=teb.Tib.StackBase;return 0;}
    if(scenario==4){c->Rsp+=16;if(unwinds>=3)c->Rip=0;return 0;}
    c->Rsp+=16;
    if(c->Rip==0x101100 || c->Rip==0x1100)c->Rip=aliases?0x101200:0x1200;
    else if(c->Rip==0x101200 || c->Rip==0x1200)c->Rip=0x3100; // mixed guest x64 return: unchanged
    else c->Rip=0;
    c->ContextFlags=CONTEXT_UNWOUND_TO_CALL;
    return 0;
}
static int is_valid_frame(ULONG_PTR frame){return frame>=0x8000 && frame<teb.Tib.StackBase;}
__BASELINE__
__PRODUCTION__
static void reset(void){seed=(CONTEXT){aliases?0x101100:0x1100,0x8000,0};scenario=lookups=unwinds=canonical=errors=0;memset(lookup_pcs,0,sizeof lookup_pcs);}
int main(void){
    void *frames[4096];aliases=1;reset();
    assert(old_walk(frames,4096,0)==4096 && unwinds==4096 && canonical==0);
    reset();assert(RtlWalkFrameChain(frames,4096,0)==2 && unwinds==3 && canonical==3);
    assert(frames[0]==(void *)0x1200 && frames[1]==(void *)0x3100);
    assert(lookup_pcs[1]==0x11fc && lookup_pcs[2]==0x3100); // -4 only for native return
    reset();assert(RtlWalkFrameChain(frames,8,1<<8)==1 && frames[0]==(void *)0x3100);
    reset();assert(RtlWalkFrameChain(frames,0,0)==0 && lookups==0);
    reset();scenario=1;assert(RtlWalkFrameChain(frames,4096,0)==0 && unwinds==1);
    reset();scenario=2;assert(RtlWalkFrameChain(frames,8,0)==0);
    reset();scenario=3;assert(RtlWalkFrameChain(frames,8,0)==0);
    reset();scenario=4;assert(RtlWalkFrameChain(frames,8,0)==2 && unwinds==3); // same PC with advancing SP is valid
    reset();errors=1;assert(RtlWalkFrameChain(frames,8,0)==0);
    aliases=0;reset();assert(RtlWalkFrameChain(frames,8,0)==2 && frames[0]==(void *)0x1200);
    aliases=1;reset();seed.Rip=0x701100;assert(RtlWalkFrameChain(frames,4096,0)==0 && unwinds==1);
    puts("PASS: actual production frame walker: old alias loop 4096 steps; fixed mixed walk 3; canonical return PCs, skip/count, recursive frames, invalid frame/status and no-progress termination");
}
'''
    fixture = fixture.replace('__BASELINE__',function(original).replace('RtlWalkFrameChain(','old_walk(',1)).replace('__PRODUCTION__',production)
    c = folder/'walk.c';c.write_text(fixture,encoding='utf-8')
    cc = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
    assert cc, 'C compiler required'
    out = folder/('walk.exe' if os.name=='nt' else 'walk')
    subprocess.run([cc,'-std=c11','-O2','-Wall','-Werror',str(c),'-o',str(out)],check=True)
    subprocess.run([str(out)],check=True,timeout=10)
