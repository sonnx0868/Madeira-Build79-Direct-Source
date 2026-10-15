#!/usr/bin/env python3
"""Assemble STR B/H/S/D/Q register forms and execute the real Mach alias helper."""
from pathlib import Path
import ast, os, shutil, struct, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/signal_arm64_ios.c').read_text(encoding='utf-8')
compiler = os.environ.get('CC') or shutil.which('clang')
if not compiler: raise SystemExit('clang required')
def block(start):
    at = source.index(start); brace = source.index('{', at); end = brace + 1; depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}'); end += 1
    return source[at:end]
helper = block('static int ios_mach_simd_register_store(')
branch = block('else if ((insn & 0x3fe00c00u) == 0x3c200800u ||')
advance = source.split('/* Advance PC past the emulated instruction */',1)[1].split('static volatile int emul_count',1)[0]
# Reuse the existing object reader; importing its test module would execute it.
tree = ast.parse((root / 'tests/host/check-lrcpc2-alias.py').read_text())
fn = next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='elf_text')
scope={'struct':struct};exec(compile(ast.Module(body=[fn],type_ignores=[]),'<object reader>','exec'),scope)
forms=[]
for width, register in [(1,'b'),(2,'h'),(4,'s'),(8,'d'),(16,'q')]:
    scale=width.bit_length()-1
    for option, operand, value in [(2,'w11, uxtw',32),(3,'x11, lsl',32),
                                    (6,'w11, sxtw',0xffffffe0),(7,'x11, sxtx',0xffffffffffffffe0)]:
        for shifted in [False,True]:
            for rt in [0,16,19,31]:
                text=operand+f' #{scale}' if shifted else ('x11' if option==3 else operand)
                index=(2 if option<6 else ((1<<32)-2 if option==6 else (1<<64)-2)) if shifted else value
                forms.append((f'str {register}{rt}, [x19, {text}]',rt,option,scale if shifted else 0,index,width))
forms += [('str q16, [x19, w11, uxtw]',16,2,0,32,16),
          ('str q19, [x19, w24, uxtw]',19,2,0,32,16),
          ('str q31, [sp, xzr]',31,3,0,0,16),
          ('str d16, [x19, w24, uxtw]',16,2,0,32,8)]
c = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
static uintptr_t guest=0x7105217fe0ull;
static unsigned char data[128];
static int first_prot=7,last_prot=7,discontinuity,no_alias;
static unsigned width=16;
uintptr_t ios_jit_anon_alias_lookup(uintptr_t address) {
    if(no_alias || address<guest || address-guest>=sizeof(data))return 0;
    if(discontinuity && address==guest+32+width-1)return 0;
    return (uintptr_t)data+address-guest;
}
int ios_page_expected_prot(const void *address) {
    uintptr_t value=(uintptr_t)address;
    if(value<guest || value-guest>=sizeof(data))return -1;
    return value==guest+32+width-1 && width>1?last_prot:first_prot;
}
''' + helper + r'''
typedef struct {uint64_t __x[29],__sp,__pc;} test_state;
typedef struct {unsigned char __v[32][16];} test_neon;
static test_neon vectors;
#define STDERR_FILENO 2
#define dprintf(...) ((void)0)
#define __darwin_arm_thread_state64_set_pc_fptr(s,v) ((s).__pc=(uintptr_t)(v))
static int dispatch(uint32_t insn,uintptr_t fault_addr,test_state *saved,int neon) {
    test_state state=*saved;
    test_neon neon_state=vectors;
    int have_neon=neon,emulated=0,handled=0;
    uintptr_t fault_pc=state.__pc;
    if(0){}
''' + branch + r'''
    if(emulated) {
''' + advance + r'''
    }
    *saved=state;return handled;
}
struct test_case {uint32_t insn;unsigned rt,option,shift;uint64_t index;unsigned width;};
static const struct test_case cases[]={CASES};
static void reset(void) {memset(data,0xa5,sizeof(data));first_prot=last_prot=7;discontinuity=no_alias=0;}
static void intact(void) {for(unsigned i=0;i<sizeof(data);i++)assert(data[i]==0xa5);}
int main(void) {
    for(unsigned r=0;r<32;r++)for(unsigned b=0;b<16;b++)vectors.__v[r][b]=(unsigned char)(r*13+b);
    for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        struct test_case c=cases[i];test_state state={0},before;
        unsigned rn=(c.insn>>5)&31,rm=(c.insn>>16)&31;
        uint64_t base=guest+32;
        reset();width=c.width;
        if(rm!=31) {
            uint64_t offset=c.option==6?(uint64_t)(int64_t)(int32_t)c.index:c.index;
            offset<<=c.shift;
            base-=offset;
            // Dirty upper 32 bits must not affect Wm UXTW/SXTW addressing.
            state.__x[rm]=c.index | ((c.option==2||c.option==6)?0xfeedface00000000ull:0);
        }
        if(rn==31)state.__sp=base;else state.__x[rn]=base;
        state.__pc=0x1690d0684ull;before=state;
        assert(dispatch(c.insn,guest+32,&state,1));
        assert(state.__pc==before.__pc+4 && !memcmp(state.__x,before.__x,sizeof(state.__x)) && state.__sp==before.__sp);
        assert(!memcmp(data+32,vectors.__v[c.rt],width));
        for(unsigned j=0;j<sizeof(data);j++)if(j<32||j>=32+width)assert(data[j]==0xa5);
        reset();state=before;
        assert(dispatch(c.insn,guest+32+width-1,&state,1)); // FAR can name a later byte of a crossing access.
        assert(!memcmp(data+32,vectors.__v[c.rt],width) && data[32+width]==0xa5);
        reset();state=before;first_prot=5;
        assert(!dispatch(c.insn,guest+32,&state,1) && state.__pc==before.__pc);intact();
        // Simulate the existing guest SMC callback invalidating translations
        // and granting logical WRITE. The retry must complete once.
        first_prot=7;assert(dispatch(c.insn,guest+32,&state,1) && state.__pc==before.__pc+4);
        if(width>1) { reset();state=before;last_prot=5;assert(!dispatch(c.insn,guest+32,&state,1));intact(); }
        reset();state=before;first_prot=-1;assert(!dispatch(c.insn,guest+32,&state,1));intact();
        if(width>1) { reset();state=before;discontinuity=1;assert(!dispatch(c.insn,guest+32,&state,1));intact(); }
        reset();state=before;no_alias=1;assert(!dispatch(c.insn,guest+32,&state,1));intact();
        reset();state=before;assert(!dispatch(c.insn,guest+32,&state,0));intact();
        assert(!dispatch(c.insn,guest+32+width,&state,1));intact();
        assert(!dispatch(c.insn|(1u<<22),guest+32,&state,1));intact(); // LDR must not write.
        assert(!dispatch((c.insn&~(7u<<13)),guest+32,&state,1));intact(); // unallocated extend
    }
    assert(cases[160].insn==0x3cab4a70u && cases[161].insn==0x3cb84a73u && cases[163].insn==0xfc384a70u);
    for(unsigned bad=29;bad<=30;bad++) {
        reset();test_state s={0};
        assert(!dispatch((0x3cab4a70u&~(31u<<5))|(bad<<5),guest+32,&s,1));intact();
        assert(!dispatch((0x3cab4a70u&~(31u<<16))|(bad<<16),guest+32,&s,1));intact();
    }
    printf("PASS: %zu assembled STR-B/H/S/D/Q register forms, real Mach branch/PC advance, SMC-before-write barrier, exact alias span, missing NEON and register/canary preservation\n",sizeof(cases)/sizeof(cases[0]));
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-qreg-alias-') as directory:
    folder=Path(directory);asm=folder/'forms.s';obj=folder/'forms.o'
    asm.write_text('.text\n'+'\n'.join(f[0] for f in forms)+'\n')
    subprocess.run([compiler,'-target','aarch64-none-linux-gnu','-c',str(asm),'-o',str(obj)],check=True)
    words=struct.unpack('<'+'I'*len(forms),scope['elf_text'](obj.read_bytes()))
    table=',\n'.join('{0x%08xu,%d,%d,%d,0x%xull,%d}'%(word,*case[1:]) for word,case in zip(words,forms))
    (folder/'test.c').write_text(c.replace('CASES',table),encoding='utf-8')
    exe=folder/'test.exe'
    flags=['-static'] if os.name=='nt' else []
    subprocess.run([compiler,'-std=c11','-O2','-Wall','-Wextra',*flags,str(folder/'test.c'),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
