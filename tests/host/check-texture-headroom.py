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
#include <stdint.h>
#include <assert.h>
#include "texture_headroom.h"
#undef TARGET_OS_IOS
#define TARGET_OS_IOS 1
#define MB (UINT64_C(1024)*1024)
static uint64_t available = 3027*MB;
static int remote;
static uint64_t fixture_available_memory(void) { return available; }
static int wmtr_enabled(void) { return remote; }
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
int main(int argc,char **argv) {
  @autoreleasepool {
    readings=[DeviceReadings new];readings.unified=YES;
    readings.recommended=10922*MB;readings.allocated=11040*MB;
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
    NSDictionary *result=@{@"productionQueryExecuted":@YES,@"realMetalDevice":readings.actual.name,
      @"unified":@(readings.actual.hasUnifiedMemory),@"recommendedBytes":@(recommended),
      @"allocationDeltaBytes":@(after-before),@"effectiveHeadroomBytes":@(q.len),@"physicalGameFpsMeasured":@NO};
    if(argc>1){NSData *data=[NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:NULL];
      assert([data writeToFile:[NSString stringWithUTF8String:argv[1]] atomically:YES]);}
    [allocation release];
    puts("PASS: production query observes an actual 32MB Metal allocation; iPhoneOS branch compiled with public APIs");
    [readings release];
  }
}
'''.replace('__PRODUCTION_QUERY__',query)
    path=folder/'native.m';path.write_text(objc,encoding='utf8')
    sdk=subprocess.check_output(['xcrun','--sdk','iphoneos','--show-sdk-path'],text=True).strip()
    subprocess.run(['xcrun','--sdk','iphoneos','clang','-target','arm64-apple-ios18.0','-isysroot',sdk,'-fblocks','-fno-objc-arc','-Werror','-Wno-deprecated-declarations','-I'+str(root/'build/dxmt-ios'),'-fsyntax-only',str(path)],check=True)
    executable=folder/'native'
    subprocess.run(['xcrun','clang','-fblocks','-fno-objc-arc','-O2','-I'+str(root/'build/dxmt-ios'),str(path),'-framework','Foundation','-framework','Metal','-o',str(executable)],check=True)
    result=folder/'texture-headroom-validation.json'
    subprocess.run([str(executable),str(result)],check=True,timeout=30)
    report=json.loads(result.read_text(encoding='utf8'))
    print(json.dumps(report,sort_keys=True))
    if os.environ.get('BITRISE_DEPLOY_DIR'):
        output=Path(os.environ['BITRISE_DEPLOY_DIR']);output.mkdir(parents=True,exist_ok=True)
        shutil.copy2(result,output/result.name)
