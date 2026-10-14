#!/usr/bin/env python3
"""Assemble real RCPC2 instructions and execute the production fault decoders."""
from pathlib import Path
import os
import shutil
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
source = (root / 'build/ntdll-unix/signal_arm64_ios.c').read_text(encoding='utf-8')
bridge = (root / 'app/Madeira/WineProcessBridge.m').read_text(encoding='utf-8')
compiler = os.environ.get('CC') or shutil.which('clang')
if not compiler:
    raise SystemExit('clang required')

def function(text, name):
    start = text.index(name)
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]

def elf_text(data):
    assert data[:6] == b'\x7fELF\x02\x01'
    at = struct.unpack_from('<Q', data, 40)[0]
    size, count, names = struct.unpack_from('<HHH', data, 58)
    header = struct.unpack_from('<IIQQQQIIQQ', data, at + names * size)
    strings = data[header[4]:header[4] + header[5]]
    for i in range(count):
        h = struct.unpack_from('<IIQQQQIIQQ', data, at + i * size)
        name = strings[h[0]:].split(b'\0', 1)[0]
        if name == b'.text':
            return data[h[4]:h[4] + h[5]]
    raise AssertionError('assembler produced no text section')

cases = []
offsets = (-256, -1, 0, 1, 12, 255)
registers = ((6, 'x21'), (31, 'sp'), (30, 'x0'))
forms = [('stlurb', 'w', 1, 0), ('stlurh', 'w', 2, 0),
         ('stlur', 'w', 4, 0), ('stlur', 'x', 8, 0),
         ('ldapurb', 'w', 1, 1), ('ldapurh', 'w', 2, 1),
         ('ldapur', 'w', 4, 1), ('ldapur', 'x', 8, 1),
         ('ldapursb', 'x', 1, 2), ('ldapursb', 'w', 1, 3),
         ('ldapursh', 'x', 2, 2), ('ldapursh', 'w', 2, 3),
         ('ldapursw', 'x', 4, 2)]
for mnemonic, width, nbytes, opc in forms:
    for immediate in offsets:
        for rt, rn in registers:
            reg = width + ('zr' if rt == 31 else str(rt))
            cases.append((f'{mnemonic} {reg}, [{rn}, #{immediate}]', nbytes, opc, rt))

# Test exactly the imported Mach alias branch, without unrelated platform APIs.
alias = function(source, 'else if ((insn & 0x3fe00c00) == 0x19000000)')
alias = alias.replace('else if', 'if', 1)
helpers = '\n'.join(function(source, name) for name in (
    'static uint64_t ios_ld_extend(', 'static void ios_ld_setreg(',
    'static int ios_emulate_load(', 'static int ios_emulate_store_rel(',
    'static int ios_subfloor_classify('))
probe = function(bridge, 'static void madeira_publish_host_probe(')

fixture = r'''
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef struct { uint64_t regs[32]; } ucontext_t;
#define REGn_sig(r,c) ((c)->regs[r])
static uint64_t ios_get_reg(ucontext_t *c,int r) { return r==31 ? 0 : c->regs[r]; }
static int ios_neon_valid;
static int ios_ld_setvec(ucontext_t *c,int r,uintptr_t a,int n) { assert(0);return 0; }
'''
# The imported store helper also contains existing SIMD operations. Supply the
# minimal context shape those operations compile against; these tests use GPRs.
fixture = fixture.replace('typedef struct { uint64_t regs[32]; } ucontext_t;', r'''
typedef struct { struct { unsigned char __v[32][16]; } __ns; } vector_context;
typedef struct { uint64_t regs[32]; vector_context *uc_mcontext; } ucontext_t;
''')
fixture += helpers + r'''
static int alias_store(uint32_t insn,uintptr_t rw_addr,uint64_t value) {
    struct { uint64_t __x[32]; } state;
    for (unsigned i=0;i<32;i++) state.__x[i]=value;
    int emulated=0;
#define IOS_STORE_SRC(r) ((r)==31 ? 0ULL : state.__x[r])
''' + alias + r'''
    return emulated;
}
static const char *decoder="lrcpc2-alias-v1";
static const char *ios_rcpc2_alias_decoder(void) { return decoder; }
static const char *probe_result;
static int hardware=1,probe_rc;
static int sysctlbyname(const char *name,int32_t *v,size_t *s,void *a,size_t b) {
    *v=hardware;return probe_rc;
}
static int save_probe(const char *key,const char *value,int overwrite) {
    static char saved[512];snprintf(saved,sizeof(saved),"%s",value);probe_result=saved;return 0;
}
#define setenv save_probe
#define dprintf(...) ((void)0)
#define STDERR_FILENO 2
''' + probe + r'''
struct testcase {uint32_t insn;unsigned bytes,opc,rt;};
static struct testcase cases[]={ CASES };
int main(void) {
    for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        struct testcase c=cases[i];
        unsigned bytes=0;int write=-1;
        assert(ios_subfloor_classify(c.insn,&bytes,&write)==1);
        assert(bytes==c.bytes && write==(c.opc==0));
        for(unsigned offset=0;offset<16;offset++) {
            unsigned char data[64];memset(data,0xa5,sizeof(data));
            uintptr_t addr=(uintptr_t)(data+16+offset);
            uint64_t value=0xfedcba987654ff80ull,expected=c.rt==31 ? 0 : value;
            ucontext_t ctx={0};
            for(unsigned r=0;r<32;r++) ctx.regs[r]=value;
            if(c.opc==0) {
                assert(alias_store(c.insn,addr,value));
                assert(!memcmp((void*)addr,&expected,c.bytes));
                for(unsigned n=0;n<sizeof(data);n++)
                    if(n<16+offset || n>=16+offset+c.bytes) assert(data[n]==0xa5);
                memset(data,0xa5,sizeof(data));
                assert(ios_emulate_store_rel(&ctx,c.insn,addr));
                assert(!memcmp((void*)addr,&expected,c.bytes));
                for(unsigned n=0;n<sizeof(data);n++)
                    if(n<16+offset || n>=16+offset+c.bytes) assert(data[n]==0xa5);
                assert(!ios_emulate_load(&ctx,c.insn,0)); // a store must never be read
            } else {
                memcpy((void*)addr,&value,c.bytes);
                expected=0;memcpy(&expected,&value,c.bytes);
                if(c.opc>=2 && c.bytes==1) expected=(uint64_t)(int64_t)(int8_t)value;
                if(c.opc>=2 && c.bytes==2) expected=(uint64_t)(int64_t)(int16_t)value;
                if(c.opc>=2 && c.bytes==4) expected=(uint64_t)(int64_t)(int32_t)value;
                if(c.opc==3) expected=(uint32_t)expected;
                assert(ios_emulate_load(&ctx,c.insn,addr));
                for(unsigned r=0;r<32;r++)
                    assert(ctx.regs[r]==(r==c.rt && r!=31 ? expected : value));
                assert(!alias_store(c.insn,addr,value));
                assert(!ios_emulate_store_rel(&ctx,c.insn,addr));
            }
        }
    }
    // Exact first failing Mono JIT write in both Hollow Knight reports.
    unsigned char byte=0;assert(alias_store(0x190002a6,(uintptr_t)&byte,0x55) && byte==0x55);
    for(unsigned size=2;size<=3;size++) for(unsigned opc=2;opc<=3;opc++) {
        if(size==2 && opc==2) continue;
        uint32_t insn=0x19000000u|(size<<30)|(opc<<22);
        ucontext_t ctx={0};unsigned bytes=0;int write;
        assert(!ios_subfloor_classify(insn,&bytes,&write));
        assert(!ios_emulate_load(&ctx,insn,0));
        assert(!alias_store(insn,0,42));
        assert(!ios_emulate_store_rel(&ctx,insn,0));
    }
    for(const uint32_t *p=(uint32_t[]){0xd503201f,0xf8000020,0xf8400020,0xf8bfc020,0xc89ffc20,0};*p;p++)
        assert(!alias_store(*p,0,42)); // no false STLUR match
    madeira_publish_host_probe();assert(strstr(probe_result,"LRCPC2=1"));
    decoder="old";madeira_publish_host_probe();assert(strstr(probe_result,"LRCPC2=0"));
    decoder=NULL;madeira_publish_host_probe();assert(strstr(probe_result,"LRCPC2=0"));
    decoder="lrcpc2-alias-v1";hardware=0;
    madeira_publish_host_probe();assert(strstr(probe_result,"LRCPC2=0"));
    probe_rc=-1;madeira_publish_host_probe();assert(strstr(probe_result,"LRCPC2=?"));
    printf("PASS: %zu assembled RCPC2 forms, unaligned/canary/ZR/signed-load cases, invalid encodings and native-consumer probe gate\n",sizeof(cases)/sizeof(cases[0]));
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-lrcpc2-alias-') as scratch:
    tmp = Path(scratch)
    asm = tmp / 'forms.s'; obj = tmp / 'forms.o'
    asm.write_text('.text\n' + '\n'.join(c[0] for c in cases) + '\n', encoding='utf-8')
    subprocess.run([compiler, '-target', 'aarch64-none-linux-gnu', '-march=armv8.4-a',
                    '-c', str(asm), '-o', str(obj)], check=True)
    data = elf_text(obj.read_bytes()); assert len(data) == len(cases) * 4
    words = struct.unpack('<' + 'I' * len(cases), data)
    table = ',\n'.join('{0x%08xu,%d,%d,%d}' % (word, c[1], c[2], c[3]) for word, c in zip(words, cases))
    cfile = tmp / 'check.c'; exe = tmp / 'check.exe'
    cfile.write_text(fixture.replace('CASES', table), encoding='utf-8')
    subprocess.run([compiler, '-std=c11', '-O2', '-Werror=implicit-function-declaration',
                    str(cfile), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
