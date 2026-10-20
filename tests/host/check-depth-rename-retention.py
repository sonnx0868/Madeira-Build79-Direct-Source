#!/usr/bin/env python3
"""Run the actual depth rename pool before/after its retirement policy.

Texture storage is a controlled stand-in; reference counting, tracking, pool
selection and retirement code are production code. Logical bytes below are
not a measurement of Liar's Bar, iPad memory, GPU time or rendered quality.
"""
from pathlib import Path
import json
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
pin = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD:dxmt"], text=True).strip()
files = (
    "src/dxmt/dxmt_allocation.hpp", "src/dxmt/dxmt_allocation.cpp",
    "src/dxmt/dxmt_texture.hpp", "src/dxmt/dxmt_texture.cpp",
    "src/d3d11/d3d11_context_impl.cpp", "src/util/rc/util_rc_ptr.hpp",
    "src/util/util_likely.hpp",
)
original = {
    name: subprocess.check_output(["git", "-C", str(root / "dxmt"), "show", pin + ":" + name])
    for name in files
}


def pool_source(sources):
    header = sources["src/dxmt/dxmt_texture.hpp"]
    begin = header.index("class RenamableTexturePool {")
    declaration = header[begin:header.index("\n};", begin) + 3]
    source = sources["src/dxmt/dxmt_texture.cpp"]
    begin = source.index("RenamableTexturePool::RenamableTexturePool(")
    functions = source[begin:source.rindex("} // namespace dxmt")]
    return declaration, functions


fixture = r'''
#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <vector>
#include "src/dxmt/dxmt_allocation.cpp"
#include "src/util/rc/util_rc_ptr.hpp"
#include "depth_rename_retention.hpp"
namespace dxmt {
enum class TextureAllocationFlag { None };
template<typename T> using Flags = unsigned;
static unsigned created, destroyed, live, highwater;
struct TextureAllocation : Allocation {
  unsigned id;
  std::array<unsigned,16> pixels{};
  TextureAllocation() : id(++created) { ++live; if(live > highwater) highwater=live; }
  ~TextureAllocation() { --live; ++destroyed; }
};
struct Texture {
  unsigned refs = 0;
  void incRef() { ++refs; }
  void decRef() { if(--refs == 0) delete this; }
  Rc<TextureAllocation> allocate(Flags<TextureAllocationFlag>) { return new TextureAllocation; }
};
__POOL_DECLARATION__
__POOL_FUNCTIONS__
struct Batch {
  AllocationRefTracking references;
  std::vector<std::unique_ptr<std::array<uintptr_t,512>>> storage;
  void use(TextureAllocation *a, uint64_t seq) {
    if(a->checkRetained(seq)) return;
    if(!references.track(a)) {
      storage.push_back(std::make_unique<std::array<uintptr_t,512>>());
      references.addStorage(storage.back()->data(), sizeof(*storage.back()));
      assert(references.track(a));
    }
  }
  void finish() { references.clear(); storage.clear(); }
  ~Batch() { finish(); }
};
static Rc<TextureAllocation> next(RenamableTexturePool &pool,uint64_t seq,uint64_t coherent) {
__NEXT_CALL__
}
static void reset_stats() { assert(!live);created=destroyed=highwater=0; }
static void idle_peak() {
  reset_stats();
  {
    RenamableTexturePool pool(new Texture,32,0);
    Batch gpu;
    for(unsigned i=0;i<32;++i) {
      auto a=next(pool,1,0); a->pixels.fill(i+100); gpu.use(a.ptr(),1);
    }
    assert(live==32 && highwater==32);
    for(uint64_t seq=2;seq<=302;++seq) {
      auto a=next(pool,seq,seq>2?seq-2:0);
      assert(a->pixels[0]==100); // no texture writes from the retirement path
      Batch current;current.use(a.ptr(),seq);
      if(seq==3) gpu.finish();
    }
    assert(live==32); // 300 completed submissions, not wall time or current seq
    {
      auto a=next(pool,303,301);assert(a->pixels[0]==100);
      assert(live==(EXPECT_RETIRE?2u:32u));
    }
    printf("%s: idle peak retained=%u logical-MiB=%u active-capacity=32\n",
           EXPECT_RETIRE?"after":"before",live,live*64);
    // A later burst still uses all 32 slots, with the two warm slots unchanged.
    std::array<unsigned,32> ids{};
    for(unsigned i=0;i<40;++i) {
      auto a=next(pool,304,302);
      if(i<32) ids[i]=a->id;else assert(a->id==ids[i%32]);
      if(i<2) assert(a->pixels[0]==100+i);
      Batch current;current.use(a.ptr(),304);
    }
    assert(live==32 && created==(EXPECT_RETIRE?62u:32u));
  }
  assert(!live && created==destroyed);
}
static void active_peak() {
  reset_stats();
  {
    RenamableTexturePool pool(new Texture,32,0);
    for(uint64_t seq=1;seq<=1000;++seq) {
      Batch gpu;
      for(unsigned i=0;i<32;++i) {
        auto a=next(pool,seq,seq>5?seq-5:0);gpu.use(a.ptr(),seq);
      }
      assert(live==32 && created==32 && !destroyed);
    }
  }
  assert(!live && created==destroyed);
}
static void pending_and_late_use() {
  reset_stats();
  {
    RenamableTexturePool pool(new Texture,32,0);
    Batch initial;
    Rc<TextureAllocation> last;
    for(unsigned i=0;i<32;++i) {last=next(pool,1,0);initial.use(last.ptr(),1);}
    auto current=next(pool,1000,0); // queue is blocked: completed watermark is zero
    assert(live==32 && !destroyed);
    initial.finish();
    Batch delayed_draw;
    // The last rename happened at 1, but a subsequent draw actually used it at 900.
    // Retirement must use that last encoded access, not the pool-selection age.
    delayed_draw.use(last.ptr(),900);
    auto id=last->id;last=nullptr;
    current=next(pool,1001,500);
    assert(live==(EXPECT_RETIRE?3u:32u));
    delayed_draw.finish();
    current=next(pool,1002,899);
    assert(live==(EXPECT_RETIRE?3u:32u));
    current=next(pool,1200,1199);
    assert(live==(EXPECT_RETIRE?3u:32u));
    current=next(pool,1201,1200);
    assert(live==(EXPECT_RETIRE?2u:32u));
    (void)id;
  }
  assert(!live && created==destroyed);
}
static void recurring_burst_stays_warm() {
  reset_stats();
  {
    RenamableTexturePool pool(new Texture,32,0);
    for(uint64_t seq=1;seq<=1501;++seq) {
      Batch gpu;
      const unsigned clears=(seq-1)%300==0?32u:1u;
      for(unsigned i=0;i<clears;++i) {
        auto a=next(pool,seq,seq-1);gpu.use(a.ptr(),seq);
      }
      assert(live==32 && created==32 && !destroyed);
    }
  }
  assert(!live && created==destroyed);
}
static void current_name_retains_allocation() {
  reset_stats();
  {
    RenamableTexturePool pool(new Texture,32,0);
    Rc<TextureAllocation> name;
    for(unsigned i=0;i<32;++i) {name=next(pool,1,0);Batch gpu;gpu.use(name.ptr(),1);}
    name->pixels.fill(0x12345678);
    auto current=next(pool,500,400);
    assert(name->pixels[0]==0x12345678);
    assert(live==(EXPECT_RETIRE?3u:32u));
    name=nullptr;
    assert(live==(EXPECT_RETIRE?2u:32u));
  }
  assert(!live && created==destroyed);
}
}
int main() {
  using namespace dxmt;
  assert(!madeira::retire_depth_rename_slot(0,1,1000));
  assert(!madeira::retire_depth_rename_slot(1,1,1000));
  assert(!madeira::retire_depth_rename_slot(2,1000,999));
  assert(!madeira::retire_depth_rename_slot(2,1,300));
  assert(madeira::retire_depth_rename_slot(2,1,301));
  idle_peak();active_peak();pending_and_late_use();recurring_burst_stays_warm();current_name_retains_allocation();
  puts("PASS: actual pool/Rc/allocation tracking; active capacity, pending GPU work, late uses and contents preserved");
}
'''

compiler = os.environ.get("CXX") or shutil.which("clang++") or shutil.which("c++")
assert compiler, "C++ compiler required"
with tempfile.TemporaryDirectory(prefix="madeira-depth-retention-") as work:
    folder = Path(work)
    for name, contents in original.items():
        path = folder / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(contents)
    patch = folder / "retention.patch"
    patch.write_bytes((root / "patches/dxmt-depth-rename-retention.patch").read_bytes().replace(b"\r\n", b"\n"))
    baseline = {name: data.decode("utf-8") for name, data in original.items()}
    subprocess.run(["git", "-C", str(folder), "apply", "--check", str(patch)], check=True)
    subprocess.run(["git", "-C", str(folder), "apply", str(patch)], check=True)
    subprocess.run(["git", "-C", str(folder), "apply", "--reverse", "--check", str(patch)], check=True)
    patched = {name: (folder / name).read_text(encoding="utf-8") for name in files}
    assert "getNext(enc.currentSeqId(), enc.queue().CoherentSeqId())" in patched["src/d3d11/d3d11_context_impl.cpp"]
    for after, sources in ((False, baseline), (True, patched)):
        # The production pool declaration and methods are used verbatim, with
        # the corresponding production allocation/reference tracking code.
        for name in ("src/dxmt/dxmt_allocation.hpp", "src/dxmt/dxmt_allocation.cpp"):
            (folder / name).write_text(sources[name], encoding="utf-8")
        declaration, functions = pool_source(sources)
        text = fixture.replace("__POOL_DECLARATION__", declaration).replace("__POOL_FUNCTIONS__", functions)
        text = text.replace("__NEXT_CALL__", "return pool.getNext(seq,coherent);" if after else "(void)coherent;return pool.getNext(seq);")
        source = folder / "test.cpp"
        source.write_text(text, encoding="utf-8")
        binary = folder / ("after" if after else "before")
        if os.name == "nt": binary = binary.with_suffix(".exe")
        sanitize = ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"] if sys.platform == "darwin" else []
        subprocess.run([compiler,"-std=c++20","-O1","-g",*sanitize,
            *(["-static"] if os.name=="nt" else []), "-DEXPECT_RETIRE="+str(int(after)),
            "-I"+str(folder), "-I"+str(folder / "src/util"), "-I"+str(root / "build/dxmt-ios"),
            str(source),"-o",str(binary)], check=True)
        env = dict(os.environ, UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
        subprocess.run([str(binary)],check=True,timeout=20,env=env)

print(json.dumps({"actualPoolExecuted":True,"actualReferenceTrackingExecuted":True,
    "baselineRetainedSlots":32,"idleRetainedSlots":2,"activeCapacity":32,
    "waitsAdded":False,"textureContentsChanged":False,"realMetalAllocationMeasured":False,
    "liarsBarCauseProved":False,"physicalGameFPSMeasured":False}, sort_keys=True))
