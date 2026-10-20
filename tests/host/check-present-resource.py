#!/usr/bin/env python3
"""Exercise actual swapchain fence waiting and native Metal creation ownership.

The deferred native temporary is controlled. This is not an iPad game FPS,
thermal, or total-memory measurement.
"""
from pathlib import Path
import importlib.util,json,os,shutil,subprocess,sys,tempfile
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('pipeline',root/'tests/host/check-dxmt-patches.py')
pipeline=importlib.util.module_from_spec(spec);spec.loader.exec_module(pipeline)
sources=pipeline.patched_sources(root)
swap=sources['src/d3d11/d3d11_swapchain.cpp']
native=sources['src/winemetal/unix/winemetal_unix.c']

def block(text,needle):
    start=text.index(needle);brace=text.index('{',start);end=brace+1;depth=1
    while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return text[start:end]

state=block(swap,'  class SyncFrameState {')+';'
wait=block(swap,'  SyncFrameState SyncFrame(')
assert wait.count('lock.unlock();')==1 and wait.count('lock.lock();')==1
present=block(swap,'  MADEIRA_SWAPCHAIN_ENTRY Present1(')
assert present.index('device_context_->PrepareFlush()')<present.index('SyncFrame(++presentation_count_, lock)')
assert present.index('if (commit_before_present_')<present.index('SyncFrame(++presentation_count_, lock)')
assert present.index('SyncFrame(++presentation_count_, lock)')<present.index('auto chunk = cmd_queue.CurrentChunk();')
assert present.count('sync_state = std::move(sync_state)')==2
assert present.index('present_lock(present_mutex_)')<present.index('SyncInterval > 4')
old_wait=wait.replace('SyncFrame(', 'OldSyncFrame(',1).replace('        lock.unlock();\n','').replace('          lock.lock();\n','')
prefix=present[present.index('    {\n      madeira::PresentWaitScope timer(madeira_present_waits_, madeira::PresentWait::Prepare);'):present.index('    auto chunk = cmd_queue.CurrentChunk();')]
assert prefix.count('auto sync_state = SyncFrame(++presentation_count_, lock);')==1
early_prefix='    auto sync_state = SyncFrame(++presentation_count_, lock);\n'+prefix.replace('    auto sync_state = SyncFrame(++presentation_count_, lock);\n','')
fixture=r'''
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <cstdio>
#include "present_waits.hpp"
using d3d11_device_mutex=std::mutex;
struct CpuFence {
  std::mutex mutex;std::condition_variable changed;
  uint64_t value=0;unsigned signals=0;std::promise<void> entered;
  bool recording=false,submitted_when_wait=false;bool *observed_submit=nullptr;
  void wait(uint64_t target){if(recording){assert(observed_submit);submitted_when_wait=*observed_submit;return;}std::unique_lock<std::mutex> l(mutex);entered.set_value();changed.wait(l,[&]{return value>=target;});}
  void signal(uint64_t target){std::lock_guard<std::mutex> l(mutex);assert(target>value);value=target;++signals;changed.notify_all();}
};
struct Chunk{bool pending=true;bool hasCommands(){return pending;}};
struct Device{std::mutex mutex;Chunk chunk;bool submitted=false;Device&GetDXMTDevice(){return *this;}Device&queue(){return *this;}Chunk*CurrentChunk(){return &chunk;}};
struct Context{Device*device;void PrepareFlush(){}void Commit(){device->submitted=true;device->chunk.pending=false;}};
using HRESULT=int;using UINT=unsigned;
constexpr HRESULT DXGI_STATUS_OCCLUDED=1;
struct Swap {
  std::unique_ptr<CpuFence> frame_latency_fence_=std::make_unique<CpuFence>();
  unsigned frame_latency=1;madeira::PresentWaits madeira_present_waits_;
  Device own_device;Device *device_=&own_device;Context own_context{device_};Context*device_context_=&own_context;
  bool commit_before_present_=true;unsigned preferred_max_frame_rate=60;double init_refresh_rate_=60;uint64_t presentation_count_=1;
__STATE__
__OLD__;
__NEW__;
  HRESULT SubmitPrefix(UINT SyncInterval=0){HRESULT hr=0;std::unique_lock<d3d11_device_mutex>lock(device_->mutex);
__PREFIX__
    return hr;
  }
  HRESULT EarlySubmitPrefix(UINT SyncInterval=0){HRESULT hr=0;std::unique_lock<d3d11_device_mutex>lock(device_->mutex);
__EARLY_PREFIX__
    return hr;
  }
};
static void run(bool baseline){
  Swap s;std::mutex device;auto entered=s.frame_latency_fence_->entered.get_future();
  std::promise<void> consumed;auto done=consumed.get_future();
  std::thread producer([&]{
    std::unique_lock<std::mutex> lock(device);
    auto state=baseline?s.OldSyncFrame(2,lock):s.SyncFrame(2,lock);
    assert(lock.owns_lock());
    // Capture/move must not signal completion early; the callback owns it.
    auto callback=[state=std::move(state)]{};
    assert(s.frame_latency_fence_->signals==1);
    callback();consumed.set_value();
  });
  entered.get();
  bool progress=device.try_lock();if(progress)device.unlock();
  assert(progress!=baseline);
  s.frame_latency_fence_->signal(1);done.get();producer.join();
  assert(s.frame_latency_fence_->value==2 && s.frame_latency_fence_->signals==2);
  printf("%s: shared-device-lock-available-during-GPU-wait=%d, fence-signals=%u\n",baseline?"before":"after",progress,s.frame_latency_fence_->signals);
}
static void submission_order(bool early,bool enabled){
 Swap s;s.commit_before_present_=enabled;s.frame_latency_fence_->recording=true;
 s.frame_latency_fence_->observed_submit=&s.device_->submitted;
 if(early)s.EarlySubmitPrefix();else s.SubmitPrefix();
 assert(s.device_->submitted==enabled);
 assert(s.frame_latency_fence_->submitted_when_wait==(enabled&&!early));
 printf("%s: frame-tail-submitted-before-fence-wait=%d optional-commit=%d\n",early?"early-wait-negative-control":"production-submit-prefix",s.frame_latency_fence_->submitted_when_wait,enabled);
}
int main(){run(true);run(false);submission_order(true,true);submission_order(false,true);submission_order(false,false);Swap s;s.frame_latency_fence_.reset();std::mutex device;std::unique_lock<std::mutex> lock(device);auto state=s.SyncFrame(2,lock);assert(lock.owns_lock());}
'''.replace('__STATE__',state).replace('__OLD__',old_wait).replace('__NEW__',wait).replace('__PREFIX__',prefix).replace('__EARLY_PREFIX__',early_prefix)

for function in ('_MTLDevice_newBuffer','_MTLDevice_newTexture','_MTLBuffer_newTexture',
                 '_MTLTexture_newTextureView','_MTLDevice_newSamplerState'):
    assert '@autoreleasepool {' in block(native,function+'(void *obj) {'),function
creation=block(native,'_MTLDevice_newTexture(void *obj) {')
old_creation=creation.replace('_MTLDevice_newTexture(', '_Old_MTLDevice_newTexture(',1)
old_creation=old_creation.replace('  @autoreleasepool {\n','',1)
assert old_creation.endswith('\n  }\n}')
old_creation=old_creation[:-len('\n  }\n}')]+ '\n}'

metal=r'''
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef uint64_t obj_handle_t;typedef int NTSTATUS;
#define STATUS_SUCCESS 0
#define RM_OP_NEW_TEXTURE_FULL 1
#define RM_OK 0
struct WMTTextureInfo {uint32_t width,height,pixel_format;uint64_t gpu_resource_id;unsigned mach_port;};
struct unixcall_mtldevice_newtexture {uint64_t device;struct{struct WMTTextureInfo *ptr;}info;uint64_t ret;};
struct rm_wmt_info{uint64_t owner;uint32_t info_len,extra_count;};
struct rm_ret_handle_u64{uint64_t handle,value;};
static int wmtr_enabled(void){return 0;}
static int wmtr_call(int op,void *in,size_t size,void *out,size_t output,int flags){abort();}
static void fill_texture_descriptor(MTLTextureDescriptor *d,struct WMTTextureInfo *i){
 d.textureType=MTLTextureType2D;d.pixelFormat=MTLPixelFormatBGRA8Unorm;
 d.width=i->width;d.height=i->height;d.storageMode=MTLStorageModeShared;
 d.usage=MTLTextureUsageShaderRead|MTLTextureUsageRenderTarget;
}
static unsigned temporary_live,temporary_peak;
@interface Temporary : NSObject {id<MTLBuffer> allocation;}
- (instancetype)initWithDevice:(id<MTLDevice>)device;
@end
@implementation Temporary
- (instancetype)initWithDevice:(id<MTLDevice>)device {if((self=[super init])){allocation=[device newBufferWithLength:16384 options:MTLResourceStorageModeShared];assert(allocation);++temporary_live;if(temporary_live>temporary_peak)temporary_peak=temporary_live;}return self;}
- (void)dealloc {[allocation release];--temporary_live;[super dealloc];}
@end
@interface DeviceProbe:NSObject {id<MTLDevice> underlying;}
- (instancetype)initWithDevice:(id<MTLDevice>)device;
- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor;
@end
@implementation DeviceProbe
- (instancetype)initWithDevice:(id<MTLDevice>)d {if((self=[super init]))underlying=[d retain];return self;}
- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)d {[[[Temporary alloc] initWithDevice:underlying] autorelease];return [underlying newTextureWithDescriptor:d];}
- (void)dealloc {[underlying release];[super dealloc];}
@end
static NTSTATUS
__OLD_CREATION__
static NTSTATUS
__CREATION__
static unsigned run(id device,BOOL baseline){
 temporary_live=temporary_peak=0;unsigned retained=0;
 @autoreleasepool {
  for(unsigned i=0;i<128;++i){
   struct WMTTextureInfo info={.width=1920,.height=1080};
   struct unixcall_mtldevice_newtexture p={.device=(uint64_t)device,.info={&info}};
   assert((baseline?_Old_MTLDevice_newTexture(&p):_MTLDevice_newTexture(&p))==0);
   id<MTLTexture> texture=(id)p.ret;assert(texture && texture.width==1920 && texture.height==1080 && info.gpu_resource_id);
   // The returned retained texture must remain usable after the inner pool drains.
   unsigned pixel=0xff112233,copy=0;[texture replaceRegion:MTLRegionMake2D(0,0,1,1) mipmapLevel:0 withBytes:&pixel bytesPerRow:4];
   [texture getBytes:&copy bytesPerRow:4 fromRegion:MTLRegionMake2D(0,0,1,1) mipmapLevel:0];assert(copy==pixel);[texture release];
  }
  retained=temporary_live;assert(retained==(baseline?128u:0u));
 }
 assert(!temporary_live);printf("%s: native-create-deferred-live=%u peak=%u dimensions=1920x1080 pixels=preserved\n",baseline?"before":"after",retained,temporary_peak);return retained;
}
int main(){@autoreleasepool{id<MTLDevice>d=MTLCreateSystemDefaultDevice();assert(d);DeviceProbe*p=[[DeviceProbe alloc]initWithDevice:d];run(p,YES);run(p,NO);[p release];[d release];}return 0;}
'''.replace('__CREATION__',creation).replace('__OLD_CREATION__',old_creation)

compiler=os.environ.get('CXX') or shutil.which('clang++') or shutil.which('c++')
assert compiler,'C++ compiler required'
report={'productionFenceWaitExecuted':False,'deviceLockReleasedDuringWait':False,
        'orderedFenceOwnershipPreserved':False,'frameTailSubmittedBeforeFenceWait':False,
        'earlyWaitNegativeControlDetected':False,'nativeCreationRetainedReturnTested':False,
        'iPadGameFPSMeasured':False,'iPadTotalRAMReductionMeasured':False}
with tempfile.TemporaryDirectory(prefix='madeira-present-resource-') as work:
    folder=Path(work);source=folder/'test.cpp';source.write_text(fixture,encoding='utf-8')
    binary=folder/('check.exe' if os.name=='nt' else 'check')
    sanitize=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if sys.platform=='darwin' else []
    subprocess.run([compiler,'-std=c++20','-O1','-g','-pthread',*sanitize,*(['-static'] if os.name=='nt' else []),
        '-I'+str(root/'build/dxmt-ios'),str(source),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True,timeout=20)
    report.update(productionFenceWaitExecuted=True,deviceLockReleasedDuringWait=True,orderedFenceOwnershipPreserved=True,
                  frameTailSubmittedBeforeFenceWait=True,earlyWaitNegativeControlDetected=True)
    if sys.platform=='darwin':
        source=folder/'create.m';source.write_text(metal,encoding='utf-8');binary=folder/'native'
        subprocess.run(['xcrun','clang','-fno-objc-arc','-O1','-g',*sanitize,str(source),'-framework','Foundation','-framework','Metal','-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True,timeout=90)
        report['nativeCreationRetainedReturnTested']=True
        if os.environ.get('BITRISE_DEPLOY_DIR'):
            path=Path(os.environ['BITRISE_DEPLOY_DIR'])/'present-resource-validation.json'
            path.write_text(json.dumps(report,indent=2)+'\n')
    elif '--require-darwin' in sys.argv:raise SystemExit('Darwin/Metal required for native creation ownership')
    else:print('SKIP: native Metal creation ownership requires Darwin')
print(json.dumps(report,sort_keys=True))
