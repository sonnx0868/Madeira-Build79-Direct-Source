#!/usr/bin/env python3
"""Exercise Wine's alias filter against the unmodified FEX registration code."""
from pathlib import Path
import argparse
import os
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def function(text, signature):
    start = text.index(signature)
    brace = text.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cxx', default=os.environ.get('CXX') or shutil.which('clang++'))
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    assert args.cxx, 'C++ compiler required'
    source = (ROOT / 'FEX/Source/Windows/ARM64EC/IosJitAlias.cpp').read_text(encoding='utf-8')
    # Read-only reuse of the actual producer. The log sink records entry to the
    # allocating overflow branch, without needing a PE syscall on the host.
    callback = function(source, 'void BTCpu64IosAddAliasMapping(')
    translate = function(source, 'uint64_t IosJitTranslate(')
    harness = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include "ios_fex_alias_policy.h"
struct IosAliasEntry { uint64_t PeBase, JitBase, Size, Padding; };
constexpr int kMaxEntries = 256;
constexpr uint64_t kIosSubfloorFloor = 0x100000000ull;
IosAliasEntry entries[kMaxEntries]{};
IosAliasEntry *g_Entries = entries;
int g_EntryCount, overflow, subfloor;
void IosSubfloorAdd(uint64_t, uint64_t, uint64_t) { ++subfloor; }
namespace LogMan { namespace Msg {
template <class... Args> void EFmt(const char *, Args...) { ++overflow; }
}}
CALLBACK
TRANSLATE
void push(unsigned long long p, unsigned long long j, unsigned long long s) {
    BTCpu64IosAddAliasMapping(p,j,s);
}
void reset() { memset(entries,0,sizeof(entries)); g_EntryCount=overflow=subfloor=0; }
constexpr uint64_t base=0x7000000000ull, jit=0x148000000ull, stride=0x10000;
std::vector<unsigned char> pe(1024);
void w16(size_t p,uint16_t v) { memcpy(pe.data()+p,&v,2); }
void w32(size_t p,uint32_t v) { memcpy(pe.data()+p,&v,4); }
void w64(size_t p,uint64_t v) { memcpy(pe.data()+p,&v,8); }
int read_pe(uintptr_t a,void *out,size_t n) {
    if(a<base||a-base>pe.size()||n>pe.size()-(a-base))return 0;
    memcpy(out,pe.data()+(a-base),n);return 1;
}
void image(uint16_t machine=0xaa64) {
    std::fill(pe.begin(),pe.end(),0);
    w16(0,0x5a4d);w32(60,64);w32(64,0x4550);w16(68,machine);
    w16(84,240);w16(88,0x20b);w32(196,16);
    w32(280,512);w32(284,320);w32(512,320);
}
int main() {
    image(); assert(ios_alias_native_arm64(base,pe.size(),read_pe));
    w64(712,0x170000000); assert(!ios_alias_native_arm64(base,pe.size(),read_pe)); // ARM64X
    for(auto machine:{0x8664,0xa641,0xa64e,0x14c,0}) {
        image(machine);assert(!ios_alias_native_arm64(base,pe.size(),read_pe));
    }
    image();w32(60,0xffffffff);assert(!ios_alias_native_arm64(base,pe.size(),read_pe));
    image();w32(280,1000);assert(!ios_alias_native_arm64(base,pe.size(),read_pe));
    image();w32(512,204);assert(!ios_alias_native_arm64(base,pe.size(),read_pe));
    image();w32(512,200);assert(ios_alias_native_arm64(base,pe.size(),read_pe));
    image();assert(!ios_alias_native_arm64(base+2048,pe.size(),read_pe));
    image();assert(!ios_alias_native_arm64(UINTPTR_MAX-16,pe.size(),read_pe));
    for(size_t n=0;n<pe.size();++n) (void)ios_alias_native_arm64(base,n,read_pe);
    // Original path: mapping 257 enters the logger/allocator on the native
    // loader stack, matching the supplied Ren'Py log after 256 completed maps.
    reset(); for(unsigned i=0;i<257;++i)push(base+i*stride,jit+i*stride,4096);
    assert(g_EntryCount==256&&overflow==1);
    // Fixed path: x64/EC stays registered while 400 native helper images
    // remain in Wine's table and do not consume the FEX table.
    reset();ios_alias_history h{};
    for(unsigned i=0;i<440;++i) {
        ios_alias_publish(&h,push,base+i*stride,jit+i*stride,4096,i>=40);
    }
    assert(g_EntryCount==40&&overflow==0);
    for(unsigned i=0;i<40;++i)assert(IosJitTranslate(base+i*stride+17)==jit+i*stride+17);
    // Catch-up, duplicate registration and tombstones must not fill the table.
    for(unsigned i=0;i<440;++i)ios_alias_publish(&h,push,base+i*stride,jit+i*stride,4096,i>=40);
    ios_alias_publish(&h,push,0,0,0,0);assert(g_EntryCount==40&&subfloor==0);
    // Reuse of an EC range by native code must replace the stale alias,
    // including a partially overlapping base and repeated native reloads.
    ios_alias_publish(&h,push,base+stride+32,jit+0x800000,4096,1);
    assert(IosJitTranslate(base+stride+33)==jit+0x800001&&g_EntryCount==40);
    ios_alias_publish(&h,push,base+stride+32,jit+0x900000,4096,1);
    assert(IosJitTranslate(base+stride+33)==jit+0x900001&&overflow==0);
    ios_alias_publish(&h,push,0x400000,base,4096,1);assert(subfloor==1&&g_EntryCount==40);
    // Unknown image headers are not classified as native-only.
    ios_alias_publish(&h,push,base+500*stride,jit+500*stride,4096,0);
    assert(g_EntryCount==41);
    puts("PASS: original map257 enters FEX overflow; filtered440 maps retain40 x64/EC aliases with no overflow");
    puts("PASS: ARM64X, malformed headers, catch-up, tombstones, native VA reuse and sub-floor routing");
}
'''.replace('CALLBACK', callback).replace('TRANSLATE', translate)
    with tempfile.TemporaryDirectory(prefix='madeira-alias-') as td:
        td = Path(td)
        unit = td / 'check.cpp'
        unit.write_text(harness, encoding='utf-8')
        exe = td / ('check.exe' if os.name == 'nt' else 'check')
        flags = ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror']
        if args.sanitize:
            flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
        subprocess.run([args.cxx, *flags, '-I'+str(ROOT/'build/ntdll-unix'), str(unit), '-o', str(exe)], check=True)
        environment = dict(os.environ)
        environment['PATH'] = str(Path(args.cxx).resolve().parent) + os.pathsep + environment.get('PATH', '')
        subprocess.run([str(exe)], check=True, env=environment)


if __name__ == '__main__':
    main()
