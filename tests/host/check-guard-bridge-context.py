#!/usr/bin/env python3
"""Execute production guard bridge and Mach publication blocks with OS boundaries controlled."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root=Path(__file__).resolve().parents[2]
source=(root/'build/ntdll-unix/signal_arm64_ios.c').read_text(encoding='utf-8')


def function(signature):
    start=source.index(signature);opening=source.index('{',start);end=opening+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]


start=source.index('                int delivery = ios_mach_deliver_guest_exception(')
end=source.index('                else if (delivery) handled = 1;',start)+len('                else if (delivery) handled = 1;')
publish=source[start:end]
arm_start=source.index('            if (handled)\n',end)
arm_publish=source[arm_start:source.index('            else\n',arm_start)]
fixture=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ios_guard_access.h"
#include "ios_browser_child_role.h"
typedef uintptr_t SIZE_T,mach_vm_address_t,mach_vm_size_t;
typedef int BOOL;typedef void *thread_state_t;
typedef struct {int unused;} TEB;
typedef struct {uint64_t x[31],sp,pc;} STATE;
typedef struct {unsigned char __v[32][16];uint32_t fpsr,fpcr;} NEON;
typedef struct {STATE __ss;NEON __ns;} MC;
typedef struct {MC *uc_mcontext;} ucontext_t;
#define REGn_sig(n,c) ((c)->uc_mcontext->__ss.x[n])
#define FP_sig(c) ((c)->uc_mcontext->__ss.x[29])
#define LR_sig(c) ((c)->uc_mcontext->__ss.x[30])
#define SP_sig(c) ((c)->uc_mcontext->__ss.sp)
#define PC_sig(c) ((c)->uc_mcontext->__ss.pc)
#define KERN_SUCCESS 0
#define ARM_NEON_STATE64 17
#define ARM_THREAD_STATE64 6
#define mach_task_self() 1
static unsigned char backing[128];
static unsigned alias_calls;
static int deny_alias,fail_read;
int mach_vm_read_overwrite(int task,mach_vm_address_t p,SIZE_T n,mach_vm_address_t out,mach_vm_size_t *got) {
    assert(task==1 && n==4);if(fail_read)return 5;memcpy((void *)out,(void *)p,n);*got=n;return 0;
}
BOOL ios_virtual_guard_neighbor_access(TEB *teb,void *p,SIZE_T n,BOOL write,void (*callback)(void *,void *),void *opaque) {
    (void)teb;(void)write;++alias_calls;
    if(deny_alias)return 0;
    assert((uintptr_t)p>=(uintptr_t)backing && (uintptr_t)p+n<=(uintptr_t)backing+sizeof backing);
    callback(p,opaque);return 1;
}
/*BRIDGE*/
static int expected_delivery,have_neon=1,fail_neon;
static unsigned publications,neon_publications,arm_publications;
static int ios_mach_deliver_guest_exception(int thread,STATE *state,NEON *neon,int have,int exception,uintptr_t fault,uintptr_t teb) {
    (void)exception;(void)fault;(void)teb;assert(thread==42 && have==have_neon);
    state->pc=0x1234;neon->__v[31][0]=0x66;return expected_delivery;
}
static int thread_set_state(int thread,int kind,thread_state_t data,unsigned count) {
    assert(thread==42);++publications;
    if(kind==ARM_NEON_STATE64) {
        assert(count==sizeof(NEON)/4 && ((NEON *)data)->__v[31][0]==0x66 && publications==1);
        ++neon_publications;return fail_neon?5:0;
    }
    assert(kind==ARM_THREAD_STATE64 && count==sizeof(STATE)/4 && ((STATE *)data)->pc==0x1234);
    ++arm_publications;return 0;
}
static int production_publication(void) {
    int thread=42,handled=0;STATE state={0};NEON neon_state={0};
    unsigned neon_count=sizeof(NEON)/4,count=sizeof(STATE)/4;
    struct {int exception;} message={0},*req=&message;
    uintptr_t fault_addr=0,thread_teb=0;
    /*PUBLISH*/
    /*ARM_PUBLISH*/
    return handled;
}
int main(void) {
    char *gpu[]={"wine","C:\\Chrome\\chrome.exe","--type=gpu-process","https://private/?token=secret"};
    char *unknown[]={"wine","C:\\Chrome\\chrome.exe","--type","secret-profile-value"};
    char *game[]={"wine","game.exe","--type=renderer"};
    assert(!strcmp(ios_browser_child_role(4,gpu),"gpu"));
    assert(!strcmp(ios_browser_child_role(4,unknown),"other-browser-child"));
    assert(!ios_browser_child_role(3,game) && !ios_browser_child_role(0,NULL));
    TEB teb={0};MC mc,before;ucontext_t uc={&mc};uint32_t instruction;
    memset(&mc,0xa5,sizeof mc);memset(backing,0x5a,sizeof backing);
    instruction=0xb8284a64u; /* exact932 register-offset store */
    mc.__ss.pc=(uintptr_t)&instruction;mc.__ss.x[19]=(uintptr_t)backing;mc.__ss.x[8]=68;mc.__ss.x[4]=0x12345678;
    mc.__ss.sp=0x34560;before=mc;
    assert(ios_guard_neighbor_service(&teb,&uc,(uintptr_t)backing+68,1)==1);
    uint32_t word;memcpy(&word,backing+68,4);assert(word==0x12345678);
    assert(mc.__ss.pc==(uintptr_t)&instruction+4 && mc.__ss.sp==before.__ss.sp);
    assert(!memcmp(mc.__ss.x,before.__ss.x,sizeof mc.__ss.x) && !memcmp(&mc.__ns,&before.__ns,sizeof mc.__ns));
    instruction=0x3dc0031fu; /* LDR q31,[x24] */
    mc=before;mc.__ss.pc=(uintptr_t)&instruction;mc.__ss.x[24]=(uintptr_t)backing;before=mc;
    alias_calls=0;assert(!ios_guard_neighbor_service(&teb,&uc,(uintptr_t)backing,0));
    assert(!alias_calls && !memcmp(&mc,&before,sizeof mc));
    assert(ios_guard_neighbor_service(&teb,&uc,(uintptr_t)backing,1)==2);
    assert(!memcmp(mc.__ns.__v[31],backing,16));
    assert(mc.__ns.fpsr==before.__ns.fpsr && mc.__ns.fpcr==before.__ns.fpcr);
    assert(!memcmp(mc.__ss.x,before.__ss.x,sizeof mc.__ss.x));
    mc=before;deny_alias=1;assert(!ios_guard_neighbor_service(&teb,&uc,(uintptr_t)backing,1));assert(!memcmp(&mc,&before,sizeof mc));deny_alias=0;
    fail_read=1;assert(!ios_guard_neighbor_service(&teb,&uc,(uintptr_t)backing,1));assert(!memcmp(&mc,&before,sizeof mc));fail_read=0;
    assert(!ios_guard_neighbor_service(&teb,&uc,(uintptr_t)backing+64,1));assert(!memcmp(&mc,&before,sizeof mc));
    for(int delivery=0;delivery<=3;++delivery)for(int fail=0;fail<=1;++fail) {
        expected_delivery=delivery;fail_neon=fail;have_neon=1;publications=neon_publications=arm_publications=0;
        int ok=production_publication();
        assert(ok==(delivery!=0 && !(delivery==3 && fail)));
        assert(neon_publications==(unsigned)(delivery==3) && arm_publications==(unsigned)ok);
    }
    expected_delivery=3;have_neon=0;publications=0;assert(!production_publication() && !publications);
    puts("PASS:production register-offset and SIMD context bridge, missing-NEON/alias/read/span refusals, untouched FP state and Mach NEON-before-PC publication including failures");
    return 0;
}
'''
production=function('struct ios_guard_operation')+';\n'+function('static void ios_guard_alias_operation(')+'\n'+function('static int ios_guard_neighbor_service(')
text=fixture.replace('/*BRIDGE*/',production).replace('/*PUBLISH*/',publish).replace('/*ARM_PUBLISH*/',arm_publish)
compiler=os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert compiler,'C compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-guard-context-') as tmp:
    old_header=subprocess.check_output(['git','-C',str(root),'show',
        '09555b0:build/ntdll-unix/ios_guard_access.h'],text=True,encoding='utf-8')
    baseline=Path(tmp)/'baseline.c'
    baseline.write_text(old_header+r'''
#include <assert.h>
#include <stdio.h>
int main(void) {
    uint64_t regs[31]={0};struct ios_guard_access access;
    regs[19]=0x7101683f00ull;regs[8]=0xc4;
    assert(!ios_guard_decode(0xb8284a64u,regs,0,&access));
    puts("REPRODUCED:932 production decoder declines exact captured STR w4,[x19,w8,uxtw]b8284a64 after earlier guard accesses succeed");
    return 0;
}
''',encoding='utf-8')
    baseline_exe=baseline.with_suffix('.exe' if os.name=='nt' else '')
    subprocess.run([compiler,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function',str(baseline),'-o',str(baseline_exe)],check=True)
    subprocess.run([str(baseline_exe)],check=True,timeout=30)
    c=Path(tmp)/'bridge.c';c.write_text(text,encoding='utf-8');exe=c.with_suffix('.exe' if os.name=='nt' else '')
    flags=[] if os.name=='nt' else ['-fsanitize=address,undefined']
    subprocess.run([compiler,'-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-unused-function',*flags,'-I',str(root/'build/ntdll-unix'),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True,timeout=30)
