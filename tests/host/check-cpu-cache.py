#!/usr/bin/env python3
"""Exercise the actual portable cache with corrupted files and concurrent writes."""
from pathlib import Path
import os, shutil, subprocess, tempfile
root = Path(__file__).resolve().parents[2]
cxx = os.environ.get('CXX') or shutil.which('clang++')
if not cxx: raise SystemExit('clang++ is required')
fixture = r'''
#include "madeira_code_cache.h"
#include <cassert>
#include <fstream>
#include <thread>
using namespace Madeira::CPUCache;
int main(int argc, char** argv) {
    assert(argc == 2);
    std::string base = argv[1], file = base + "/normal.bin";
    Bytes key {1,2,3,4}, payload {9,8,7};
    {
        Store store(file); assert(!store.find(key));
        assert(store.observe(key, payload)); assert(!store.find(key)->validated); store.flush();
    }
    {
        Store store(file); assert(!store.find(key)->validated);
        assert(store.observe(key, payload)); assert(store.find(key)->validated); store.flush();
    }
    {
        Store store(file); assert(store.find(key)->validated); assert(!store.find(Bytes {1,2,3,5}));
        assert(!store.observe(key, Bytes {9,8,6})); assert(!store.find(key));
        assert(!store.observe(key, payload));
    }
    { Store store(file); assert(!store.find(key)); assert(!store.observe(key,payload)); }
    file = base + "/concurrent.bin";
    {
        Store store(file);
        std::vector<std::thread> threads;
        for (unsigned i=0; i<64; ++i) threads.emplace_back([&,i] {
            Bytes k {uint8_t(i)}, p {uint8_t(i),1,2};
            for (unsigned j=0; j<16; ++j) { assert(store.observe(k,p)); assert(store.find(k)->payload==p); }
        });
        for (auto& thread: threads) thread.join(); store.flush();
    }
    { Store store(file); for (unsigned i=0; i<64; ++i) assert(store.find(Bytes {uint8_t(i)})->validated); }
    // A truncated record cannot become validated and cannot conceal an appended record.
    file = base + "/truncated.bin";
    { Store store(file); assert(store.observe(key,payload)); store.flush(); }
    { std::ofstream stream(file, std::ios::binary|std::ios::app); stream.put('M'); }
    {
        Store store(file); assert(!store.find(key)->validated);
        assert(store.observe(Bytes {5},payload)); store.flush();
    }
    { Store store(file); assert(!store.find(Bytes {5})); }
    // Flip cached bytes while preserving record lengths: no invalid payload reuse.
    file = base + "/corrupt.bin";
    { Store store(file); assert(store.observe(key,payload)); store.flush(); }
    { std::fstream stream(file,std::ios::binary|std::ios::in|std::ios::out); stream.seekp(44); stream.put(0); }
    { Store store(file); assert(!store.find(key)); assert(store.observe(key,payload)); }
    { Store store(file); assert(!store.find(key)); }
    file = base + "/too-large.bin";
    { std::ofstream stream(file,std::ios::binary); stream.seekp(MaxFile); stream.put(0); }
    { Store store(file); assert(!store.find(key)); assert(!store.observe(key,payload)); }
    { Store store(base); assert(!store.find(key)); assert(store.observe(key,payload)); } // Failed writer is optional.
    { Store store(base + "/limits.bin"); assert(!store.observe(key,Bytes(MaxRecord,1))); }
    Block block;
    block.codeOnlySize=60; block.code.resize(128); uint32_t tail=80;
    memcpy(block.code.data(),&tail,4);
    block.points.push_back({0x1234,4}); block.opcodes.push_back({0,4});
    block.subblocks.push_back({4,60}); block.symbols.push_back(64);
    Bytes encoded=encode(block); assert(decode(encoded));
    auto invalid=encoded; invalid.pop_back(); assert(!decode(invalid));
    invalid=encoded; uint32_t enormous=0xffffffff; memcpy(invalid.data()+8,&enormous,4); assert(!decode(invalid));
    block.points[0].offset=80; assert(!decode(encode(block))); block.points[0].offset=4;
    block.symbols[0]=124; assert(!decode(encode(block))); block.symbols[0]=64;
    block.opcodes[0].hostOffset=-1; assert(!decode(encode(block)));
    // Bounded fuzzing of length/offset decoder, with sanitizers in the macOS gate.
    uint64_t rng=7;
    for (unsigned i=0;i<4000;++i) {
        invalid=encoded; rng=rng*6364136223846793005ULL+1;
        invalid[rng%invalid.size()]^=uint8_t((rng>>32)|1);
        (void)decode(invalid);
    }
    puts("PASS: CPU cache learning/promotion, mismatch fallback, restart, concurrency, corruption and bounded decode");
}
'''
with tempfile.TemporaryDirectory(prefix='madeira-cpu-cache-') as directory:
    tmp=Path(directory)
    (tmp/'cache.cpp').write_text(fixture,encoding='utf-8')
    binary=tmp/('check.exe' if os.name=='nt' else 'check')
    flags=['-std=c++20','-O1','-g','-Wall','-Wextra','-Werror']
    if os.name == 'nt': flags+=['-static']
    if os.name != 'nt': flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
    subprocess.run([cxx,*flags,'-I'+str(root/'build/fex-arm64ec'),str(tmp/'cache.cpp'),'-o',str(binary)],check=True)
    subprocess.run([str(binary),str(tmp)],check=True,timeout=30)
