#!/usr/bin/env python3
"""Exercise stage sampling lifecycle using fake Metal completion/counter data.

These are protocol/safety tests, never a GPU performance measurement. Actual
stage-counter availability and timings still require the original device.
"""
from pathlib import Path
import argparse, platform, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--require-darwin', action='store_true')
args = parser.parse_args()
if platform.system() != 'Darwin':
    if args.require_darwin:
        raise SystemExit('Darwin and its Metal SDK are required')
    print('SKIP: stage counter lifecycle tests require the macOS Metal SDK')
    raise SystemExit(0)

source = r'''
#import "GPUStageProfile.h"
#include <assert.h>
#include <unistd.h>

@interface FakeSet : NSObject
@property(nonatomic, strong) NSString *name;
@end
@implementation FakeSet
@end
@interface FakeSamples : NSObject
@property(nonatomic, strong) NSMutableData *data;
@property BOOL gpuCompleted;
@property unsigned resolves;
- (NSData *)resolveCounterRange:(NSRange)range;
@end
@implementation FakeSamples
- (NSData *)resolveCounterRange:(NSRange)range {
 assert(self.gpuCompleted); self.resolves++;
 assert(range.location==0 && range.length<=MGStageSamples);
 return [self.data subdataWithRange:NSMakeRange(0,range.length*sizeof(MTLCounterResultTimestamp))];
}
@end
@interface FakeDevice : NSObject
@property BOOL supported, invalid;
@property(nonatomic, strong) NSMutableArray<FakeSamples *> *samples;
- (BOOL)supportsCounterSampling:(MTLCounterSamplingPoint)point;
- (NSArray *)counterSets;
- (id)newCounterSampleBufferWithDescriptor:(MTLCounterSampleBufferDescriptor *)descriptor error:(NSError **)error;
@end
@implementation FakeDevice
- (BOOL)supportsCounterSampling:(MTLCounterSamplingPoint)point { return self.supported; }
- (NSArray *)counterSets { FakeSet *s=[FakeSet new];s.name=MTLCommonCounterSetTimestamp;return @[s]; }
- (id)newCounterSampleBufferWithDescriptor:(MTLCounterSampleBufferDescriptor *)d error:(NSError **)e {
 assert(d.sampleCount==MGStageSamples && d.storageMode==MTLStorageModeShared);
 FakeSamples *s=[FakeSamples new];s.data=[NSMutableData dataWithLength:MGStageSamples*sizeof(MTLCounterResultTimestamp)];
 MTLCounterResultTimestamp *v=s.data.mutableBytes;
 for(unsigned i=0;i<MGStageSamples;i++)v[i].timestamp=self.invalid?MTLCounterErrorValue:100+i*100;
 [self.samples addObject:s];return s;
}
@end
@interface FakeQueue : NSObject
@property(nonatomic, strong) FakeDevice *device;
@end
@implementation FakeQueue
@end
@interface FakeBuffer : NSObject
@property(nonatomic, strong) FakeQueue *commandQueue;
@property(nonatomic, strong) NSMutableArray *handlers;
@property BOOL committed;
- (id)device;
- (void)addCompletedHandler:(MTLCommandBufferHandler)handler;
- (void)complete;
@end
@implementation FakeBuffer
- (id)device { return self.commandQueue.device; }
- (double)GPUStartTime { return 1.0; }
- (double)GPUEndTime { return 1.001; }
- (NSError *)error { return nil; }
- (void)addCompletedHandler:(MTLCommandBufferHandler)handler {
 assert(!self.committed);[self.handlers addObject:[handler copy]];
}
- (void)complete {
 self.committed=YES;
 MadeiraGPUStageBuffer *record=objc_getAssociatedObject(self,&stage_buffer_key);
 ((FakeSamples *)record.samples).gpuCompleted=YES;
 for(MTLCommandBufferHandler handler in self.handlers)handler((id)self);
 [self.handlers removeAllObjects];
}
@end
static FakeBuffer *buffer(FakeQueue *q) { FakeBuffer *b=[FakeBuffer new];b.commandQueue=q;b.handlers=[NSMutableArray new];return b; }
static void boundary(FakeQueue *q) {
 FakeBuffer *old=buffer(q);stage_buffer_created((__bridge void *)old);
 assert(!objc_getAssociatedObject(old,&stage_buffer_key));
 stage_present((__bridge void *)old);madeira_gpu_stage_will_commit((__bridge void *)old);
 assert(!old.handlers.count);[old complete];
}
static NSDictionary *readResult(NSURL *url) {
 for(unsigned i=0;i<2000 && stage_waiting();i++)usleep(1000);
 assert(!stage_waiting());NSData *data=[NSData dataWithContentsOfURL:url];assert(data);
 return [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
}
static NSURL *file(NSURL *root,NSString *name) {return [root URLByAppendingPathComponent:name];}
int main(void) { @autoreleasepool {
 NSURL *root=[NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString]];
 assert([NSFileManager.defaultManager createDirectoryAtURL:root withIntermediateDirectories:YES attributes:nil error:nil]);
 FakeDevice *device=[FakeDevice new];device.supported=YES;device.samples=[NSMutableArray new];
 FakeQueue *queue=[FakeQueue new];queue.device=device;
 assert(!madeira_gpu_stage_enabled());
 NSURL *first=file(root,@"valid.json");stage_request(first,(id)queue);boundary(queue);
 FakeQueue *other=[FakeQueue new];other.device=device;
 FakeBuffer *foreign=buffer(other);stage_buffer_created((__bridge void *)foreign);
 assert(!objc_getAssociatedObject(foreign,&stage_buffer_key));
 FakeBuffer *b=buffer(queue);stage_buffer_created((__bridge void *)b);
 MTLRenderPassDescriptor *render=[MTLRenderPassDescriptor renderPassDescriptor];
 render.colorAttachments[0].loadAction=MTLLoadActionClear;
 render.colorAttachments[0].storeAction=MTLStoreActionStore;
 assert(madeira_gpu_stage_prepare((__bridge void *)b,(__bridge void *)render,MGStageRender));
 assert(render.colorAttachments[0].loadAction==MTLLoadActionClear && render.colorAttachments[0].storeAction==MTLStoreActionStore);
 NSObject *encoder=[NSObject new],*pipeline=[NSObject new];
 madeira_gpu_stage_encoder_created((__bridge void *)b,(__bridge void *)encoder);
 madeira_gpu_stage_pipeline((__bridge void *)encoder,(__bridge void *)pipeline);
 madeira_gpu_stage_pipeline((__bridge void *)encoder,(__bridge void *)pipeline);
 MTLComputePassDescriptor *compute=[MTLComputePassDescriptor computePassDescriptor];
 compute.dispatchType=MTLDispatchTypeConcurrent;
 assert(madeira_gpu_stage_prepare((__bridge void *)b,(__bridge void *)compute,MGStageCompute));
 assert(compute.dispatchType==MTLDispatchTypeConcurrent && compute.sampleBufferAttachments[0].startOfEncoderSampleIndex==4);
 madeira_gpu_stage_encoder_created((__bridge void *)b,(__bridge void *)[NSObject new]);
 assert(!device.samples.lastObject.resolves);
 stage_present((__bridge void *)b);madeira_gpu_stage_will_commit((__bridge void *)b);
 assert(!madeira_gpu_stage_enabled() && stage_waiting() && !device.samples.lastObject.resolves);
 [b complete];NSDictionary *result=readResult(first);
 assert([result[@"complete"] boolValue] && [result[@"available"] boolValue] && [result[@"validIntervals"] intValue]==3);
 NSArray *passes=result[@"buffers"][0][@"passes"];
 assert([passes[0][@"pipelines"] count]==1 && [passes[0][@"vertex"][@"duration"] unsignedLongLongValue]==100);
 assert([passes[1][@"encoder"][@"duration"] unsignedLongLongValue]==100);
 // Unavailable hardware must preserve gameplay and never report a zero cost.
 device.supported=NO;NSURL *unsupported=file(root,@"unsupported.json");stage_request(unsupported,(id)queue);
 result=readResult(unsupported);assert(![result[@"available"] boolValue] && ![result[@"complete"] boolValue]);
 // Error-valued samples remain unavailable rather than becoming fast stages.
 device.supported=YES;device.invalid=YES;NSURL *invalid=file(root,@"invalid.json");stage_request(invalid,(id)queue);boundary(queue);
 b=buffer(queue);stage_buffer_created((__bridge void *)b);
 assert(madeira_gpu_stage_prepare((__bridge void *)b,(__bridge void *)compute,MGStageCompute));
 madeira_gpu_stage_encoder_created((__bridge void *)b,(__bridge void *)[NSObject new]);
 stage_present((__bridge void *)b);madeira_gpu_stage_will_commit((__bridge void *)b);[b complete];
 result=readResult(invalid);assert(![result[@"available"] boolValue]);
 assert(![result[@"buffers"][0][@"passes"][0][@"encoder"][@"available"] boolValue]);
 // A cancelled generation's late GPU completion must not modify a later run.
 device.invalid=NO;NSURL *cancelled=file(root,@"cancelled.json");stage_request(cancelled,(id)queue);boundary(queue);
 b=buffer(queue);stage_buffer_created((__bridge void *)b);
 assert(madeira_gpu_stage_prepare((__bridge void *)b,(__bridge void *)compute,MGStageCompute));
 madeira_gpu_stage_encoder_created((__bridge void *)b,(__bridge void *)[NSObject new]);
 madeira_gpu_stage_will_commit((__bridge void *)b);stage_cancel();result=readResult(cancelled);
 assert(![result[@"complete"] boolValue]);
 device.supported=NO;NSURL *later=file(root,@"later.json");stage_request(later,(id)queue);[b complete];
 result=readResult(later);assert([result[@"buffers"] count]==0);
 [NSFileManager.defaultManager removeItemAtURL:root error:nil];
 puts("PASS: stage boundaries, descriptor preservation, completion-before-read, unavailable/error counters and cancellation generations");
} }
'''
with tempfile.TemporaryDirectory(prefix='madeira-stage-test-') as temp:
    folder = Path(temp)
    harness = folder / 'stages.m'
    hooks = folder / 'hooks.c'
    binary = folder / 'stages'
    harness.write_text(source, encoding='utf-8')
    hooks.write_text('int madeira_gpu_stage_hooks_version(void) { return 1; }\n', encoding='utf-8')
    subprocess.run(['xcrun', 'clang', '-fobjc-arc', '-fblocks', '-I', str(root / 'app/Madeira'),
                    str(harness), str(hooks), '-framework', 'Foundation', '-framework', 'Metal',
                    '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True, timeout=30)
