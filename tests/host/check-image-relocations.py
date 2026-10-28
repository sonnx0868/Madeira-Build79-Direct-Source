#!/usr/bin/env python3
"""Reproduce the fixed Ren'Py launcher's stale relocation walk with real pages.

The optional --exe consumes a local PE without copying game bytes into the repo.
The synthetic default carries only PE metadata and a repeated pixel word from
the report. This proves relocation safety, not complete Ren'Py gameplay.
"""
from pathlib import Path
import argparse
import json
import os
import shutil
import struct
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe', type=Path)
args = parser.parse_args()

def mapped_image(file):
    data = file.read_bytes()
    pe = struct.unpack_from('<I', data, 60)[0]
    assert data[:2] == b'MZ' and data[pe:pe+4] == b'PE\0\0'
    assert struct.unpack_from('<H', data, pe+4)[0] == 0x8664
    size, headers = struct.unpack_from('<II', data, pe+24+56)
    image = bytearray(size)
    image[:headers] = data[:headers]
    count, opt = struct.unpack_from('<H', data, pe+6)[0], struct.unpack_from('<H', data, pe+20)[0]
    for i in range(count):
        s = pe+24+opt+i*40
        rva, raw_size, raw = struct.unpack_from('<III', data, s+12)
        assert rva <= size and raw_size <= size-rva and raw_size <= len(data)-raw
        image[rva:rva+raw_size] = data[raw:raw+raw_size]
    return image

image = mapped_image(args.exe) if args.exe else bytearray(0x33000)
if not args.exe:
    image[0:2] = b'MZ'; struct.pack_into('<I', image, 60, 0x80)
    image[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<H', image, 0x84, 0x8664)
    struct.pack_into('<H', image, 0x96, 0x23)
    struct.pack_into('<Q', image, 0xb0, 0x140000000)
    struct.pack_into('<II', image, 0x130, 0x1f000, 0x68)
    for off in range(0x1f000, len(image), 4): struct.pack_into('<I', image, off, 0xff9b1924)
pe = struct.unpack_from('<I', image, 60)[0]
flags = struct.unpack_from('<H', image, pe+0x16)[0]
rva, table = struct.unpack_from('<II', image, pe+0xb0)
assert flags & 1 and rva and table

fixture = r'''
#include "ios_pe_relocations.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/resource.h>
#endif
static uint64_t u64(const void *p) { uint64_t n; memcpy(&n,p,8); return n; }
static void put32(void *p,uint32_t n) { memcpy(p,&n,4); }
static void put64(void *p,uint64_t n) { memcpy(p,&n,8); }
/* Previous production loop: unbounded block_size; ignores RELOCS_STRIPPED. */
static int old_walk(uint8_t *image,size_t image_size,uint32_t rva,uint32_t size,
                    uint64_t preferred,intptr_t delta) {
    uint8_t *block=image+rva,*end=block+size;
    int count=0;
    while(block<end) {
        uint32_t page=ios_reloc_u32(block),bytes=ios_reloc_u32(block+4);
        if(!bytes || bytes<8) break;
        int n=(bytes-8)/2;
        for(int j=0;j<n;j++) {
            uint16_t entry=ios_reloc_u16(block+8+j*2);
            if((entry>>12)==10) {
                uint8_t *p=image+(uint64_t)page+(entry&0xfff);
                uint64_t value=u64(p);
                if(value>=preferred && value<preferred+image_size) {
                    put64(p,value+delta); count++;
                }
            }
        }
        block+=bytes;
    }
    return count;
}
int main(int argc,char **argv) {
    assert(argc==3);
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
#else
    struct rlimit limit={0,0}; setrlimit(RLIMIT_CORE,&limit);
#endif
    FILE *f=fopen(argv[2],"rb");assert(f);fseek(f,0,SEEK_END);size_t size=ftell(f);rewind(f);
    size_t rounded=(size+0x3fff)&~(size_t)0x3fff;
#ifdef _WIN32
    uint8_t *image=VirtualAlloc(NULL,rounded+0x4000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(image);
    DWORD old;assert(VirtualProtect(image+rounded,0x4000,PAGE_NOACCESS,&old));
#else
    uint8_t *image=mmap(NULL,rounded+0x4000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON,-1,0);
    assert(image!=MAP_FAILED);assert(!mprotect(image+rounded,0x4000,PROT_NONE));
#endif
    assert(fread(image,1,size,f)==size);fclose(f);
    uint32_t pe=ios_reloc_u32(image+60),rva=ios_reloc_u32(image+pe+0xb0),table=ios_reloc_u32(image+pe+0xb4);
    uint16_t flags=ios_reloc_u16(image+pe+0x16);
    uint64_t preferred=u64(image+pe+0x30);
    if(!strcmp(argv[1],"old")) return old_walk(image,size,rva,table,preferred,0x8006000);
    uint8_t *saved=malloc(size);assert(saved);memcpy(saved,image,size);
    assert(ios_reloc_image_dir64(image,size,flags,&rva,&table,preferred,0x8006000)==0);
    assert(!rva && !table && !memcmp(saved,image,size));free(saved);
    /* Clearing the metadata makes subsequent IAT replay and child copies inert. */
    assert(!ios_reloc_image_dir64(image,size,flags,&rva,&table,preferred,0x9006000));
    /* The same invalid directory without STRIPPED is rejected before any write. */
    rva=ios_reloc_u32(image+pe+0xb0);table=ios_reloc_u32(image+pe+0xb4);
    assert(ios_reloc_image_dir64(image,size,flags&~1,&rva,&table,preferred,0x8006000)==-1);
    /* Real relocation semantics are retained, including unaligned targets,
     * outside-image runtime pointers and a parent/child's different deltas. */
    uint8_t good[256]={0};put32(good+0x44,12);uint16_t e=0xa083;memcpy(good+0x48,&e,2);
    for(unsigned i=0;i<3;i++) {
        put64(good+0x83,0x140000083);rva=0x40;table=12;
        assert(ios_reloc_image_dir64(good,sizeof(good),0x22,&rva,&table,0x140000000,0x10000000+i)==1);
        assert(u64(good+0x83)==0x150000083+i && rva==0x40 && table==12);
    }
    put64(good+0x83,0xdeadbeef0000);rva=0x40;table=12;
    assert(!ios_reloc_image_dir64(good,sizeof(good),0x22,&rva,&table,0x140000000,1));
    assert(u64(good+0x83)==0xdeadbeef0000);
    /* A later malformed block must leave an earlier eligible target unchanged. */
    put64(good+0x83,0x140000083);put32(good+0x50,0xfffffff0);rva=0x40;table=24;
    assert(ios_reloc_image_dir64(good,sizeof(good),0x22,&rva,&table,0x140000000,1)==-1);
    assert(u64(good+0x83)==0x140000083);
    puts("PASS: fixed image preserved; stale directory not cached; bounded parent/child relocation and atomic rejection");
    return 0;
}
'''
cc = os.environ.get('MD_RUNTIME_CC') or os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
if not cc: raise SystemExit('C compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-image-reloc-') as tmp:
    folder=Path(tmp);source=folder/'check.c';binary=folder/('check.exe' if os.name=='nt' else 'check')
    source.write_text(fixture,encoding='utf-8');(folder/'image.bin').write_bytes(image)
    subprocess.run([cc,'-std=c11','-O1','-Wall','-Wextra','-Werror','-I'+str(root/'build/ntdll-unix'),
                    str(source),'-o',str(binary)],check=True)
    old=subprocess.run([str(binary),'old',str(folder/'image.bin')],capture_output=True,timeout=10)
    expected={0xc0000005} if os.name=='nt' else {-11,-10}
    assert old.returncode&0xffffffff in {n&0xffffffff for n in expected},old.returncode
    print('PASS: previous loader really faults on the stale directory',flush=True)
    subprocess.run([str(binary),'fixed',str(folder/'image.bin')],check=True,timeout=10)
virtual=(root/'build/ntdll-unix/virtual_ios.c').read_text(encoding='utf-8')
assert virtual.count('ios_reloc_image_dir64(')==2
assert 'ios_jit_set_reloc_info(image_base, pe_image_base, delta,\n                                          reloc_rva, reloc_sz)' in virtual
print(json.dumps({'input':'local PE' if args.exe else 'synthetic stripped image',
                  'imageSize':hex(len(image)),'rva':hex(rva),'directorySize':hex(table),
                  'deviceGameplayVerified':False}),flush=True)
