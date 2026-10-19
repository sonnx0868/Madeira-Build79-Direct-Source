#!/usr/bin/env python3
"""Execute the production staging allocator with a fake Metal allocation backend."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('patches', root/'tests/host/check-dxmt-patches.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
sources = module.patched_sources(root)
pin = subprocess.check_output(['git','-C',str(root),'rev-parse','HEAD:dxmt'],text=True).strip()
before_initializer = subprocess.check_output(['git','-C',str(root/'dxmt'),'show',
    pin+':src/dxmt/dxmt_resource_initializer.hpp'],text=True)
assert sources['src/dxmt/dxmt_resource_initializer.hpp'] == before_initializer.replace(
    '  size_t cpu_command_heap_size;',
    '  size_t upload_bytes_since_flush_; // Replaces the redundant fixed CPU heap capacity; layout unchanged.')
before_header = subprocess.check_output(['git','-C',str(root/'dxmt'),'show',
    pin+':src/dxmt/dxmt_staging.hpp'],text=True)
before_header = '\n'.join(l for l in before_header.splitlines() if not l.startswith('#include') and l != '#pragma once')
before_header = before_header.replace('StagingResource','BeforeStagingResource').replace('StagingMapResult','BeforeStagingMapResult')
header = sources['src/dxmt/dxmt_staging.hpp']
header = '\n'.join(l for l in header.splitlines() if not l.startswith('#include') and l != '#pragma once')
body = sources['src/dxmt/dxmt_staging.cpp']
body = '\n'.join(l for l in body.splitlines() if not l.startswith('#include'))
fixture = r'''
#include <cassert>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <memory>
#include <queue>
#include <vector>
#include <thread>
#include <stdexcept>
#include "upload_policy.hpp"
namespace WMT {struct Device {};}
namespace dxmt {
using mutex = std::mutex;
template<class T> using Rc = std::shared_ptr<T>;
enum class BufferAllocationFlag {CpuPlaced};
template<class T> struct Flags {void set(T) {}};
struct BufferAllocation {char bytes[16]{}; void *mappedMemory(int) {return bytes;}};
struct Buffer {
    static inline unsigned allocations=0;
    static inline bool reject=false;
    Buffer(uint64_t,WMT::Device) {}
    Rc<BufferAllocation> allocate(Flags<BufferAllocationFlag>, const char *) {
        if(reject) throw std::bad_alloc();
        ++allocations; return std::make_shared<BufferAllocation>();
    }
    void rename(Rc<BufferAllocation>) {}
};
}
'''
# The real implementation assigns raw Buffer pointers into Wine's intrusive Rc.
# A shared_ptr stand-in needs explicit reset; this changes ownership syntax only.
body = body.replace('buffer_ = new Buffer(length, device);', 'buffer_.reset(new Buffer(length, device));')
fixture += header + '\n' + before_header + '\nstatic_assert(sizeof(dxmt::StagingResource)==sizeof(dxmt::BeforeStagingResource));\n' + body + r'''
int main() {
    using namespace dxmt;
    constexpr uint64_t mb=1ull<<20, size=54*mb, budget=256*mb;
    assert(!madeira::flush_upload_batch(0, UINT64_MAX));
    assert(!madeira::flush_upload_batch(32*mb,32*mb));
    assert(madeira::flush_upload_batch(64*mb,1));
    assert(madeira::flush_upload_batch(UINT64_MAX,1));
    assert(madeira::flush_upload_batch(1,UINT64_MAX));
    {
        StagingResource resource({},size,16,16);
        auto original=resource.mappedImmediateMemory();
        unsigned successful=0;
        for(unsigned n=0;n<100;++n) {
            resource.useCopySource(9);
            assert(resource.tryMap(0,false,true)==StagingMapResult::Renamable);
            auto next=resource.allocateBounded(0,budget);
            if(next==~0ull) break;
            resource.updateImmediateName(9,next); ++successful;
        }
        assert(successful==4 && Buffer::allocations==5);
        assert(StagingResource::spare_bytes==4*size);
        assert(resource.gpuBusyDistance(0)==9);
        assert(resource.tryMap(0,true,false)==StagingMapResult::Mappable);
        // A retired buffer must be reused even if today's growth budget is zero.
        auto reused=resource.allocateBounded(9,0);
        assert(reused!=~0ull && resource.mappedMemory(reused)==original);
        assert(Buffer::allocations==5 && StagingResource::spare_bytes==4*size);
        resource.updateImmediateName(10,reused);
        resource.useCopyDestination(12);
        assert(uint64_t(resource.tryMap(0,true,false))==12);
        assert(resource.tryMap(12,true,false)==StagingMapResult::Mappable);
    }
    assert(StagingResource::spare_bytes==0);
    {
        StagingResource resource({},size,16,16);
        Buffer::reject=true;
        try {resource.allocateBounded(0,budget); assert(false);} catch(const std::bad_alloc&) {}
        Buffer::reject=false;
        assert(StagingResource::spare_bytes==0); // Failed allocation rolls reservation back.
    }
    std::atomic<uint64_t> reserved=0;
    std::vector<std::thread> threads;
    for(unsigned i=0;i<16;++i) threads.emplace_back([&] {
        for(unsigned n=0;n<4096;++n) madeira::reserve_staging_spare(reserved,mb,budget);
    });
    for(auto &t:threads)t.join();
    assert(reserved==budget);
    assert(!madeira::reserve_staging_spare(reserved,UINT64_MAX,UINT64_MAX));
    puts("PASS: 54MB unretired staging maps stop at four spares, retired reuse, allocation failure and concurrent budget");
}
'''
compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('c++')
if not compiler:
    raise SystemExit('C++ compiler required')
with tempfile.TemporaryDirectory(prefix='madeira-upload-flow-') as temp:
    folder = Path(temp)
    source = folder/'check.cpp'
    source.write_text('#include <cstdio>\n'+fixture, encoding='utf-8')
    binary = folder/('check.exe' if os.name == 'nt' else 'check')
    subprocess.run([compiler,'-std=c++20','-O2','-Wall','-Wextra','-Werror','-Wno-unused-private-field','-pthread',
                    '-I'+str(root/'build/dxmt-ios'),str(source),'-o',str(binary)],check=True)
    run_env = dict(os.environ)
    if os.name == 'nt': run_env['PATH'] = str(Path(compiler).parent) + os.pathsep + run_env.get('PATH','')
    subprocess.run([str(binary)],check=True,env=run_env)
initializer = sources['src/dxmt/dxmt_resource_initializer.cpp']
assert initializer.index('flush_upload_batch') < initializer.index('ALLOC_GPU(temp, total_bytes_needed)')
assert 'upload_bytes_since_flush_ = 0' in initializer
assert 'upload_bytes_since_flush_ += size' in initializer
print('PASS: uploads flush before recording, counter resets, no budget arithmetic overflow')
