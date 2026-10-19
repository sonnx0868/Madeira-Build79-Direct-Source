#!/usr/bin/env python3
"""Exercise the allocation policy and actual patched MadeiraCtl memory query.

The Darwin test executes the production Objective-C case with controlled device
readings, then with a real Metal device and a 32 MB allocation. It measures no
game FPS and never allocates up to the working-set limit.
"""
from pathlib import Path
import importlib.util
import json
import os
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('dxmt_patch_check', root/'tests/host/check-dxmt-patches.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
sources = module.patched_sources(root)
native = sources['src/winemetal/unix/winemetal_unix.c']
start = native.index('  case 7: {   /* ml2000: memory headroom')
query = native[start:native.index('  case 2: {', start)]
helpers = native[native.index('static uint64_t madeira_ram_cap(void)'):native.index('static uint64_t madeira_ml1042_video_budget(uint64_t metal_recommended);')]
assert 'madeira_texture_headroom(' in query and 'if (wmtr_enabled()) break;' in query

cc = os.environ.get('CC') or shutil.which('clang') or shutil.which('cc')
assert cc, 'C compiler required'
checks = r'''
#include <assert.h>
#include <stdio.h>
#include "texture_headroom.h"
#define MB (UINT64_C(1024)*1024)
int main(void) {
    /* Actual build813 Liar's Bar: jetsam headroom is ample while Metal is
     * already over its performance working set. The old query missed it. */
    assert(madeira_texture_headroom(3027*MB,10922*MB,11040*MB,1)==0);
    assert(madeira_texture_headroom(3027*MB,10922*MB,9500*MB,1)==1422*MB);
    assert(madeira_texture_headroom(6000*MB,10922*MB,7819*MB,1)==3103*MB);
    assert(madeira_texture_headroom(700*MB,10922*MB,7819*MB,1)==700*MB);
    assert(madeira_texture_headroom(3027*MB,10922*MB,11040*MB,0)==3027*MB);
    assert(madeira_texture_headroom(3027*MB,0,11040*MB,1)==3027*MB);
    assert(madeira_texture_headroom(0,10922*MB,0,1)==0);
    assert(madeira_texture_headroom(UINT64_MAX,UINT64_MAX,UINT64_MAX-1,1)==1);
    assert(madeira_texture_headroom(UINT64_MAX,1,UINT64_MAX,1)==0);
    assert(madeira_texture_headroom(UINT64_MAX,0,UINT64_MAX,1)==UINT64_MAX);
    /* Physical RAM 15709MB - iPadOS reserve 1536MB, versus a reported
     * process limit of 16384MB: plan with 14173MB, not 16384MB. */
    assert(madeira_memory_limit(3027*MB,13357*MB,14173*MB)==14173*MB);
    assert(madeira_memory_available(3027*MB,13357*MB,14173*MB)==816*MB);
    assert(madeira_memory_available(3027*MB,14319*MB,14173*MB)==MB);
    assert(madeira_memory_limit(3027*MB,13357*MB,0)==16384*MB);
    assert(madeira_memory_limit(UINT64_MAX,MB,0)==UINT64_MAX);
    assert(madeira_memory_available(0,13357*MB,14173*MB)==0);
    assert(madeira_memory_limit(512*MB,1024*MB,14173*MB)==1536*MB);
    puts("PASS: log813 pressure missed by process-only headroom; GPU/process minimum, discrete/unknown fallback and overflow bounds");
}
'''

with tempfile.TemporaryDirectory(prefix='madeira-texture-headroom-') as scratch:
    folder = Path(scratch)
    source = folder/'policy.c'
    source.write_text(checks, encoding='utf8')
    binary = folder/('policy.exe' if os.name == 'nt' else 'policy')
    subprocess.run([cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-I'+str(root/'build/dxmt-ios'),str(source),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
    if sys.platform != 'darwin':
        if '--require-darwin' in sys.argv: raise SystemExit('Darwin/Metal required')
        print('SKIP: real Metal query requires Darwin; C policy and full patch pipeline passed')
        raise SystemExit(0)

    objc = r'''
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <stdint.h>
#include <assert.h>
#include "texture_headroom.h"
#undef TARGET_OS_IOS
#define TARGET_OS_IOS 1
#define MB (UINT64_C(1024)*1024)
static uint64_t available = 3027*MB;
static uint64_t physical, footprint;
static long long reserve_mb=1536;
static int remote;
static uint64_t fixture_available_memory(void) { return available; }
static int wmtr_enabled(void) { return remote; }
static long long fixture_config(const char *key,long long fallback) { assert(!strcmp(key,"ram-reserve-mb"));return reserve_mb; }
static int fixture_sysctl(const char *name,void *out,size_t *size,const void *value,size_t value_size) {
  assert(!strcmp(name,"hw.memsize") && *size>=sizeof physical);memcpy(out,&physical,sizeof physical);*size=sizeof physical;return 0;
}
static kern_return_t fixture_task_info(task_name_t task,task_flavor_t flavor,task_info_t out,mach_msg_type_number_t *count) {
  assert(flavor==TASK_VM_INFO);memset(out,0,sizeof(task_vm_info_data_t));((task_vm_info_t)out)->phys_footprint=footprint;return KERN_SUCCESS;
}
@interface DeviceReadings : NSObject
@property uint64_t recommended, allocated;
@property BOOL unified;
@property NSUInteger reads;
@property(retain) id<MTLDevice> actual;
@end
@implementation DeviceReadings
- (uint64_t)recommendedMaxWorkingSetSize { self.reads++;return self.actual ? self.actual.recommendedMaxWorkingSetSize : self.recommended; }
- (NSUInteger)currentAllocatedSize { self.reads++;return self.actual ? self.actual.currentAllocatedSize : self.allocated; }
- (BOOL)hasUnifiedMemory { self.reads++;return self.actual ? self.actual.hasUnifiedMemory : self.unified; }
- (void)dealloc { [_actual release];[super dealloc]; }
@end
static DeviceReadings *readings;
static id<MTLDevice> fixture_device(void) { return (id)[readings retain]; }
#define MTLCreateSystemDefaultDevice fixture_device
#define os_proc_available_memory fixture_available_memory
#define madeira_cfg_int fixture_config
#define sysctlbyname fixture_sysctl
#define task_info fixture_task_info
__PRODUCTION_HELPERS__
struct Query { uint64_t len,ptr;unsigned ret; };
static struct Query query(void) {
    struct Query value={0},*a=&value;
    switch(7) {
__PRODUCTION_QUERY__
    default:break;
    }
    return value;
}
#undef MTLCreateSystemDefaultDevice
#undef os_proc_available_memory
#undef sysctlbyname
#undef task_info
int main(int argc,char **argv) {
  @autoreleasepool {
    readings=[DeviceReadings new];readings.unified=YES;
    readings.recommended=10922*MB;readings.allocated=11040*MB;
    if(argc>2 && !strcmp(argv[2],"ram-cap")) {
      physical=15709*MB;footprint=13357*MB;
      readings.allocated=7819*MB;
      struct Query capped=query();assert(capped.ret==1 && capped.len==816*MB && capped.ptr==footprint);
      assert(madeira_mem_limit(footprint)==14173*MB);
      available=0;capped=query();assert(capped.ret==0);
      puts("PASS: production physical-RAM query, configured iPadOS reserve, video-budget limit and mip headroom use the same cap");
      [readings release];return 0;
    }
    struct Query q=query();assert(q.ret==1 && q.len==0);
    readings.allocated=9500*MB;q=query();assert(q.ret==1 && q.len==1422*MB);
    available=700*MB;q=query();assert(q.ret==1 && q.len==700*MB);
    available=3027*MB;readings.unified=NO;q=query();assert(q.ret==1 && q.len==available);
    readings.unified=YES;readings.recommended=0;q=query();assert(q.ret==1 && q.len==available);
    NSUInteger count=readings.reads;
    available=0;q=query();assert(q.ret==0 && readings.reads==count);
    remote=1;available=3027*MB;q=query();assert(q.ret==0 && readings.reads==count);remote=0;
    puts("PASS: actual MadeiraCtl query honors discrete/unknown/remote paths and bounded unified-memory headroom");
    id<MTLDevice> actual=MTLCreateSystemDefaultDevice();assert(actual);
    readings.actual=actual;[actual release];
    available=UINT64_MAX/2;
    uint64_t before=readings.actual.currentAllocatedSize;
    id<MTLBuffer> allocation=[readings.actual newBufferWithLength:32*MB options:MTLResourceStorageModePrivate];assert(allocation);
    uint64_t after=readings.actual.currentAllocatedSize;assert(after>=before+32*MB);
    q=query();assert(q.ret==1 && q.len<=available);
    uint64_t recommended=readings.actual.recommendedMaxWorkingSetSize;
    if(readings.actual.hasUnifiedMemory && recommended) assert(q.len==madeira_texture_headroom(available,recommended,after,1));
    uint64_t actualPhysical=0;size_t actualSize=sizeof actualPhysical;
    assert(!sysctlbyname("hw.memsize",&actualPhysical,&actualSize,NULL,0) && actualPhysical);
    NSDictionary *result=@{@"productionQueryExecuted":@YES,@"realMetalDevice":readings.actual.name,@"physicalBytes":@(actualPhysical),
      @"unified":@(readings.actual.hasUnifiedMemory),@"recommendedBytes":@(recommended),
      @"allocationDeltaBytes":@(after-before),@"effectiveHeadroomBytes":@(q.len),@"physicalGameFpsMeasured":@NO};
    if(argc>1){NSData *data=[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:NULL];
      assert([data writeToFile:[NSString stringWithUTF8String:argv[1]] atomically:YES]);}
    [allocation release];
    puts("PASS: production query observes an actual 32MB Metal allocation; iPhoneOS branch compiled with public APIs");
    [readings release];
  }
}
'''.replace('__PRODUCTION_QUERY__',query).replace('__PRODUCTION_HELPERS__',helpers)
    path=folder/'native.m';path.write_text(objc,encoding='utf8')
    sdk=subprocess.check_output(['xcrun','--sdk','iphoneos','--show-sdk-path'],text=True).strip()
    subprocess.run(['xcrun','--sdk','iphoneos','clang','-target','arm64-apple-ios18.0','-isysroot',sdk,'-fblocks','-fno-objc-arc','-Werror','-Wno-deprecated-declarations','-I'+str(root/'build/dxmt-ios'),'-fsyntax-only',str(path)],check=True)
    executable=folder/'native'
    subprocess.run(['xcrun','clang','-fblocks','-fno-objc-arc','-O2','-I'+str(root/'build/dxmt-ios'),str(path),'-framework','Foundation','-framework','Metal','-o',str(executable)],check=True)
    result=folder/'texture-headroom-validation.json'
    subprocess.run([str(executable),str(result)],check=True,timeout=30)
    subprocess.run([str(executable),str(result),'ram-cap'],check=True,timeout=30)
    report=json.loads(result.read_text(encoding='utf8'))
    print(json.dumps(report,sort_keys=True))
    if os.environ.get('BITRISE_DEPLOY_DIR'):
        output=Path(os.environ['BITRISE_DEPLOY_DIR']);output.mkdir(parents=True,exist_ok=True)
        shutil.copy2(result,output/result.name)
