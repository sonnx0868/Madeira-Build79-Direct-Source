#!/usr/bin/env python3
"""Execute the production GPU retirement loop before/after the pool fix.

The controlled backend defers temporary objects as Metal/Foundation may do.
Darwin additionally uses real autorelease pools, GPU command buffers and
buffers. It checks lifetime/completion correctness, not iPad game FPS.
"""
from pathlib import Path
import argparse
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--require-darwin', action='store_true')
args = parser.parse_args()
spec = importlib.util.spec_from_file_location('patches', root/'tests/host/check-dxmt-patches.py')
pipeline = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pipeline)
sources = pipeline.patched_sources(root)
source = sources['src/dxmt/dxmt_command_queue.cpp']
start = source.index('uint32_t\nCommandQueue::WaitForFinishThread()')
end = source.index('\nvoid CommandQueue::Retain(', start)
production = source[start:end]
baseline = production.replace('WaitForFinishThread()', 'OldWaitForFinishThread()').replace(
    '    auto retirement_pool = WMT::MakeAutoreleasePool();\n', '')
assert production.count('MakeAutoreleasePool()') == 1

fixture = r'''
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <thread>
#include <vector>
#include "present_waits.hpp"
#ifdef NATIVE_METAL
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
static id<MTLDevice> actual_device;
static id<MTLCommandQueue> actual_queue;
#endif
static unsigned deferred_live, deferred_peak, pool_drains, wait_calls, callbacks;
static bool defer_temporary = true;
static constexpr unsigned frame_count = 64;
static constexpr unsigned kCommandChunkCount = frame_count + 1;
enum {WMTCommandBufferStatusScheduled=3, WMTCommandBufferStatusCompleted=4, WMTCommandBufferStatusError=5};
enum class GpuCompletionStatus {Failed,Complete};
struct Lease {
#ifdef NATIVE_METAL
  id<MTLBuffer> buffer;
  explicit Lease(id<MTLBuffer> value) : buffer([value retain]) {
#else
  explicit Lease(void *) {
#endif
    ++deferred_live; if(deferred_live>deferred_peak)deferred_peak=deferred_live;
  }
  ~Lease() {
#ifdef NATIVE_METAL
    [buffer release];
#endif
    --deferred_live;
  }
};
#ifdef NATIVE_METAL
@interface DeferredMetalLease : NSObject { @public Lease *lease; }
@end
@implementation DeferredMetalLease
- (void)dealloc {delete lease;[super dealloc];}
@end
#else
static std::vector<std::unique_ptr<Lease>> deferred;
#endif
namespace WMT {
struct Pool {
#ifdef NATIVE_METAL
  NSAutoreleasePool *value=[[NSAutoreleasePool alloc] init];
  ~Pool(){[value drain];++pool_drains;}
#else
  std::size_t start=deferred.size();
  ~Pool(){deferred.resize(start);++pool_drains;}
#endif
  Pool()=default;
  Pool(const Pool &)=delete;
};
static Pool MakeAutoreleasePool(){return {};}
}
template<class... T>static void discard(T &&...){}
#define TRACE(...) discard(__VA_ARGS__)
#define ERR(...) discard(__VA_ARGS__)
#define THREAD_PRIORITY_TIME_CRITICAL 15
static int GetCurrentThread(){return 0;}
static void SetThreadPriority(int,int){}
namespace dxmt {
namespace env {static void setThreadName(const char *){}}
template<class T>static void atomic_wait(std::atomic<T> &word,T old,std::memory_order){assert(word.load()!=old);}
template<class T>static void atomic_notify_one(std::atomic<T> &){}
}
struct Text {Text description(){return {};}const char *getUTF8String(){return "controlled error";}};
struct Logs {explicit operator bool(){return false;}std::vector<Text> elements(){return {};}};
struct Buffer {
  unsigned state=WMTCommandBufferStatusScheduled, number=0;
#ifdef NATIVE_METAL
  id<MTLCommandBuffer> actual=nil;
  id<MTLBuffer> allocation=nil;
#endif
  void prepare(unsigned i) {
    number=i; state=(i%2)?WMTCommandBufferStatusScheduled:WMTCommandBufferStatusCompleted;
#ifdef NATIVE_METAL
    auto pool=WMT::MakeAutoreleasePool();
    allocation=[actual_device newBufferWithLength:1024*1024 options:MTLResourceStorageModeShared];assert(allocation);
    actual=[[actual_queue commandBuffer] retain];assert(actual);
    id<MTLBlitCommandEncoder> encoder=[actual blitCommandEncoder];
    [encoder fillBuffer:allocation range:NSMakeRange(0,1024*1024) value:(uint8_t)i];[encoder endEncoding];
    [actual commit];
    if(!(i%2))[actual waitUntilCompleted];
#endif
  }
  unsigned status(){
#ifdef NATIVE_METAL
    return (unsigned)actual.status;
#else
    return state;
#endif
  }
  void waitUntilCompleted(){++wait_calls;
#ifdef NATIVE_METAL
    [actual waitUntilCompleted];
#else
    state=WMTCommandBufferStatusCompleted;
#endif
  }
  Text error(){return {};}
  Logs logs(){
    assert(status()>=WMTCommandBufferStatusCompleted);
#ifdef NATIVE_METAL
    (void)actual.logs;
    if(defer_temporary){DeferredMetalLease *temporary=[DeferredMetalLease new];temporary->lease=new Lease(allocation);[temporary autorelease];}
#else
    if(defer_temporary)deferred.emplace_back(new Lease(nullptr));
#endif
    return {};
  }
  void reset(){
    assert(status()>=WMTCommandBufferStatusCompleted);
#ifdef NATIVE_METAL
    [actual release];actual=nil;[allocation release];allocation=nil;
#endif
  }
};
struct Fence {uint64_t value=0;void signal(uint64_t n){assert(n>value);value=n;}};
struct Completion {
  Buffer *buffer;
  void CompleteGpuWork(GpuCompletionStatus value){assert(buffer->status()>=4);assert((buffer->status()==5)==(value==GpuCompletionStatus::Failed));++callbacks;}
};
struct Chunk {
  Buffer attached_cmdbuf;
  uint64_t frame_=0,signal_frame_latency_fence_=~0ull;
  std::vector<std::shared_ptr<Completion>> completion_targets;
  void reset(){attached_cmdbuf.reset();completion_targets.clear();}
};
struct Ring {
  std::atomic<bool> *stop;bool last;
  void free_blocks(uint64_t n){if(last && n==frame_count)stop->store(true);}
};
struct CommandQueue {
  std::atomic<bool> stopped=false;
  std::atomic<uint64_t> ready_for_commit=frame_count+1,chunk_ongoing=frame_count;
  std::array<Chunk,kCommandChunkCount> chunks;
  Fence cpu_coherent,frame_latency_fence_;
  Ring staging_allocator{&stopped,false},copy_temp_allocator{&stopped,false},argbuf_allocator{&stopped,true};
  bool error=false;
  CommandQueue(){for(unsigned n=1;n<=frame_count;++n){auto &c=chunks[n];c.frame_=n;c.signal_frame_latency_fence_=n;c.attached_cmdbuf.prepare(n);c.completion_targets.push_back(std::make_shared<Completion>(Completion{&c.attached_cmdbuf}));}}
  void MarkDeviceError(){error=true;}
  uint32_t WaitForFinishThread();uint32_t OldWaitForFinishThread();
};
using dxmt::env::setThreadName;
namespace env=dxmt::env;
__BASELINE__
__PRODUCTION__
struct FakeClock {
  using duration=std::chrono::nanoseconds;using time_point=std::chrono::time_point<FakeClock>;
  static inline time_point current{};static time_point now(){return current;}
};
static void check_phase_accounting(){
  madeira::PresentWaits waits;
  using S=madeira::PresentWaitScope<FakeClock>;using P=madeira::PresentWait;
  // The existing end-of-queue latency may be zero while an earlier fence is long.
  {S timer(waits,P::FrameFence);FakeClock::current+=std::chrono::milliseconds(83);}
  {S timer(waits,P::Queue);}
  {S timer(waits,P::DeviceLock);FakeClock::current+=std::chrono::milliseconds(7);}
  {S timer(waits,P::Layer);FakeClock::current+=std::chrono::milliseconds(2);}
  assert(waits.get(P::FrameFence)==83 && waits.get(P::Queue)==0 && waits.get(P::DeviceLock)==7 && waits.get(P::Layer)==2);
  waits.clear();for(double v:waits.milliseconds)assert(v==0);
}
static void run(bool baseline){
  deferred_live=deferred_peak=pool_drains=wait_calls=callbacks=0;
  unsigned retained=0;
  {
    auto outer=WMT::MakeAutoreleasePool();
    CommandQueue queue;
#ifndef NATIVE_METAL
    queue.chunks[3].attached_cmdbuf.state=WMTCommandBufferStatusError;
#endif
    if(baseline)queue.OldWaitForFinishThread();else queue.WaitForFinishThread();
    retained=deferred_live;
    assert(callbacks==frame_count && queue.cpu_coherent.value==frame_count && queue.frame_latency_fence_.value==frame_count && queue.chunk_ongoing==0);
#ifndef NATIVE_METAL
    assert(queue.error && wait_calls==frame_count/2-1);
#endif
    if(defer_temporary){if(baseline)assert(retained==frame_count);else assert(retained==0 && deferred_peak==1);}
  }
  assert(deferred_live==0);
  printf("%s: deferred-live-at-return=%u peak=%u pool-drains=%u completions=%u\n",baseline?"before":"after",retained,deferred_peak,pool_drains,callbacks);
}
int main(){
  check_phase_accounting();
  std::thread worker([]{
#ifdef NATIVE_METAL
    actual_device=MTLCreateSystemDefaultDevice();assert(actual_device);actual_queue=[actual_device newCommandQueue];assert(actual_queue);
#endif
    run(true);run(false);
#ifdef NATIVE_METAL
    // Also execute the production loop using only actual driver temporaries.
    defer_temporary=false;run(false);
    [actual_queue release];[actual_device release];
#endif
  });worker.join();
  puts("PASS: production retirement loop bounds deferred resources, drains after completion, preserves error/completion/fence order; separate Present wait phases");
}
'''.replace('__BASELINE__',baseline).replace('__PRODUCTION__',production)

compiler = os.environ.get('CXX') or shutil.which('clang++') or shutil.which('c++')
assert compiler, 'C++ compiler required'
with tempfile.TemporaryDirectory(prefix='madeira-retirement-') as scratch:
    folder=Path(scratch)
    native=sys.platform=='darwin'
    path=folder/('check.mm' if native else 'check.cpp');path.write_text(fixture,encoding='utf-8')
    output=folder/('check.exe' if os.name=='nt' else 'check')
    command=[compiler,'-std=c++20','-O2','-Wall','-Wextra','-Werror','-Wno-unused-private-field','-pthread',
             '-I'+str(root/'build/dxmt-ios'),str(path),'-o',str(output)]
    if native:command+=['-DNATIVE_METAL','-fno-objc-arc','-fblocks','-framework','Foundation','-framework','Metal','-fsanitize=address,undefined']
    subprocess.run(command,check=True)
    env=dict(os.environ)
    if os.name=='nt':env['PATH']=str(Path(compiler).parent)+os.pathsep+env.get('PATH','')
    subprocess.run([str(output)],env=env,check=True,timeout=60)
    # Verify instrumentation is present at all seven production wait sites.
    present=sources['src/d3d11/d3d11_swapchain.cpp']
    for part in ('DeviceLock','Prepare','Submit','Layer','FrameFence','Commit','Queue'):
        assert 'PresentWait::'+part in present
    assert '[PRESENT_WAIT]' in present
    assert present.count('state = SynchronizeLayerProperties()')==2
    if not native:
        if args.require_darwin:raise SystemExit('Darwin Metal test required')
        print('SKIP: real Metal/Foundation loop requires Darwin; production C++ lifetime and wait attribution tests passed')
    elif os.environ.get('BITRISE_DEPLOY_DIR'):
        report={'productionRetirementLoopExecuted':True,'nativeAutoreleasePools':True,'actualMetalCommandBuffers':True,
                'controlledDeferredTemporaryBefore':64,'controlledDeferredTemporaryAfter':0,
                'gpuCompletionOrderPreserved':True,'presentPhasesSeparated':True,'iPadGameFpsMeasured':False}
        (Path(os.environ['BITRISE_DEPLOY_DIR'])/'gpu-retirement-validation.json').write_text(json.dumps(report,indent=2)+'\n')
