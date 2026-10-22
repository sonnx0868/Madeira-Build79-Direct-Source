#!/usr/bin/env python3
"""Exercise actual ring allocator and marshal functions; no game-FPS claim."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
ring = (root / 'dxmt/src/dxmt/dxmt_ring_bump_allocator.hpp').read_text(encoding='utf-8')
declaration = ring[ring.index('template <typename Allocator, size_t BlockSize ='):ring.index('\nclass GpuPrivateBufferBlockAllocator')]
implementation = ring[ring.index('template <typename Allocator, size_t BlockSize, class mutex>'):ring.rindex('} // namespace dxmt')]
marshal = (root / 'dxmt/src/dxmt/dxmt_command.metal').read_text(encoding='utf-8')
marshal = marshal[marshal.index('struct DXMTDispatchArguments'):marshal.index('struct depth_stencil_out')]
marshal = marshal.replace('[[vertex]] ', '').replace(' [[buffer(0)]]', '')
marshal = marshal.replace('constant ', 'const ').replace('device ', '')
code = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <cstdio>
#include "command_cache.hpp"
#include "dynamic_trim.hpp"
#include <deque>
#include <random>
#define WARN(...) ((void)0)
template<class T> T align(T value,T alignment) { return (value+alignment-1)&~(alignment-1); }
namespace dxmt {
using mutex = std::mutex;
constexpr size_t kStagingBlockSize=4096, kStagingBlockLifetime=300;
static bool ringOversizeReuseEnabled() { return false; }
''' + declaration + implementation + r'''
}
static size_t live_bytes, peak_bytes, creations;
struct Alloc {
  struct Block {
    void *ptr=nullptr; size_t size=0;
    Block() = default;
    Block(const Block &) = delete;
    Block &operator=(const Block &) = delete;
    Block(Block &&other) : ptr(other.ptr),size(other.size) { other.ptr=nullptr;other.size=0; }
    ~Block() { if(ptr) { free(ptr);live_bytes-=size; } }
  };
  Block allocate(size_t n) {
    Block b; b.ptr=malloc(n);assert(b.ptr);b.size=n;
    live_bytes+=n;peak_bytes=std::max(peak_bytes,live_bytes);creations++;return b;
  }
};
using uint=uint32_t; using ulong=uint64_t; using ushort=uint16_t;
struct uint2 { uint x,y; };
''' + marshal + r'''
int main() {
  {
    dxmt::RingBumpState<Alloc,4096> arena(Alloc{});
    for (unsigned i=1;i<=10000;++i) {
      auto result=arena.allocate(i,i,4000,16);
      assert(result.second==0);
      assert(arena.free_blocks_keep_first());
      assert(live_bytes==4096 && creations==1);
    }
    // Oversized blocks are freed instead of being retained under a false budget.
    arena.free_blocks(~0ull);assert(live_bytes==0);
    arena.allocate(10001,10001,8192,16);
    assert(!arena.free_blocks_keep_first());assert(live_bytes==0);
    // Active allocations are not recycled by the ordinary completed-sequence path.
    auto first=arena.allocate(10002,10001,4096,16).first.ptr;
    auto second=arena.allocate(10003,10001,4096,16).first.ptr;
    assert(first!=second);arena.free_blocks(10001);assert(live_bytes==8192);
  }
  assert(live_bytes==0);
  // The process-wide lease cannot grow with hundreds of cached command lists.
  std::vector<std::unique_ptr<dxmt::CommandCacheLease>> leases;
  unsigned owned=0;
  for(unsigned i=0;i<1000;++i) {
    auto lease=std::make_unique<dxmt::CommandCacheLease>();owned+=lease->acquire();
    leases.push_back(std::move(lease));
  }
  assert(owned==4 && dxmt::command_cache_slots==4);
  leases.clear();assert(dxmt::command_cache_slots==0);
  // Compare retained identities against the old linear algorithm, including
  // delayed/out-of-order recycling and fenced entries. No extra retention.
  struct Entry { uint64_t will_free_at; unsigned id; };
  std::mt19937 random(42);
  for(unsigned trial=0;trial<10000;++trial) {
    std::deque<Entry> old, fast;
    uint64_t fence=random()%40, last=0;
    bool ordered=trial%2;
    for(unsigned n=0, count=random()%1000;n<count;++n) {
      last=ordered?last+random()%2:random()%50;
      old.push_back({last,n});
    }
    fast=old;
    size_t count=0;
    for(auto &e:old) count+=e.will_free_at<=fence;
    size_t drop=count>64?count-64:0;
    while(drop && !old.empty() && old.front().will_free_at<=fence) { old.pop_front();--drop; }
    count=dxmt::completedNames(fast,fence,ordered);
    while(count>64 && !fast.empty() && fast.front().will_free_at<=fence) { fast.pop_front();--count; }
    assert(old.size()==fast.size());
    for(size_t i=0;i<old.size();++i) assert(old[i].id==fast[i].id);
  }
  // Both Metal marshal loops must complete a batch, not spin on task zero.
  const uint2 draws[]={{64,1},{128,2},{0,1}};
  DXMTDispatchArguments out[3]={{99,99,99},{99,99,99},{99,99,99}};
  DXMTGSDispatchMarshal gs[]={{draws[0],out[0],10000,32,0},
                              {draws[1],out[1],10000,32,0},
                              {draws[2],out[2],10000,32,1}};
  gs_draw_arguments_marshal(gs);
  assert(out[0].x==2 && out[1].x==4 && out[1].y==2 && out[2].x==0);
  DXMTTSDispatchMarshal ts[]={{draws[0],out[0],10000,4,4,0},
                              {draws[1],out[1],10000,4,4,0},
                              {draws[2],out[2],10000,4,4,1}};
  ts_draw_arguments_marshal(ts);
  assert(out[0].x==4 && out[1].x==8 && out[2].x==0);
  printf("PASS: production arena reused 10000 times; one allocation, zero leaked bytes; "
         "four global leases; GS/TS batches and zero draws complete; peak fixture bytes=%zu\n",peak_bytes);
}
'''
compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('g++')
assert compiler, 'C++ compiler required (enforced in Mac CI)'
with tempfile.TemporaryDirectory(prefix='madeira-upstream-render-') as directory:
    path = Path(directory)
    (path / 'check.cpp').write_text(code, encoding='utf-8')
    subprocess.run([compiler, '-std=c++20', '-O2', '-Wall', '-Wextra', '-Werror',
                    '-fsanitize=address,undefined', '-I' + str(root / 'build/dxmt-ios'),
                    str(path / 'check.cpp'), '-o', str(path / 'check')], check=True)
    subprocess.run([str(path / 'check')], check=True, timeout=30)
