#!/usr/bin/env python3
"""Fault-inject Metal archive misses/errors using the production native cache."""
from pathlib import Path
import os, shutil, subprocess, tempfile

root = Path(__file__).resolve().parents[2]
cache = root / 'build/dxmt-ios/pipeline_cache_ios.m'
text = cache.read_text(encoding='utf-8')
assert 'QOS_CLASS_BACKGROUND' in text and 'dispatch_async(state->queue' in text
assert 'rename(' in text and 'pendingLimit = 128' in text
assert 'state && state->lookup' in text
patch = (cache.parent / 'pipeline-cache.patch').read_text(encoding='utf-8')
assert patch.count('+  params->ret_pso = (obj_handle_t)madeira_new_render_pipeline') == 2
assert 'madeira_new_compute_pipeline' in patch

fixture = r'''
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import "pipeline_cache_ios.h"
#include <assert.h>
#include <stdatomic.h>
#include <unistd.h>
@interface TestArchive : NSObject <MTLBinaryArchive>
@end
@implementation TestArchive
- (BOOL)addRenderPipelineFunctionsWithDescriptor:(MTLRenderPipelineDescriptor *)d error:(NSError **)e { return YES; }
- (BOOL)addComputePipelineFunctionsWithDescriptor:(MTLComputePipelineDescriptor *)d error:(NSError **)e { return YES; }
- (BOOL)serializeToURL:(NSURL *)url error:(NSError **)error {
    return [@"synthetic test archive" writeToURL:url atomically:NO encoding:NSUTF8StringEncoding error:error];
}
@end
@interface TestDevice : NSObject <MTLDevice>
@property(copy) NSString *name;
@property BOOL rejectArchive, cacheHit, failPipeline;
@property unsigned probes, normal, archives;
@end
@implementation TestDevice
- (id<MTLBinaryArchive>)newBinaryArchiveWithDescriptor:(MTLBinaryArchiveDescriptor *)d error:(NSError **)e {
    self.archives++;
    return self.rejectArchive ? nil : (id<MTLBinaryArchive>)[TestArchive new];
}
- (id)makePipeline:(MTLPipelineOption)o error:(NSError **)e {
    if (o & MTLPipelineOptionFailOnBinaryArchiveMiss) {
        self.probes++;
        return self.cacheHit ? [NSObject new] : nil;
    }
    self.normal++;
    if (self.failPipeline) {
        if (e) *e = [NSError errorWithDomain:@"test-pipeline-error" code:7 userInfo:nil];
        return nil;
    }
    if (e) *e = nil;
    return [NSObject new];
}
- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)d
    options:(MTLPipelineOption)o reflection:(MTLRenderPipelineReflection **)r error:(NSError **)e { return [self makePipeline:o error:e]; }
- (id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)d
    options:(MTLPipelineOption)o reflection:(MTLComputePipelineReflection **)r error:(NSError **)e { return [self makePipeline:o error:e]; }
@end
static void seedFiles(NSURL *base) {
    NSDirectoryEnumerator *files = [NSFileManager.defaultManager enumeratorAtURL:base includingPropertiesForKeys:nil options:0 errorHandler:nil];
    NSMutableArray *targets = [NSMutableArray array];
    for (NSURL *url in files) if ([url.pathExtension isEqual:@"metallib"]) [targets addObject:url];
    for (NSURL *url in targets) [@"test archive" writeToURL:url atomically:YES encoding:NSUTF8StringEncoding error:nil];
}
int main(void) { @autoreleasepool {
    NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString];
    setenv("MADEIRA_PIPELINE_CACHE_DIR", dir.UTF8String, 1);
    NSURL *base = [NSURL fileURLWithPath:dir];
    MTLRenderPipelineDescriptor *render = [MTLRenderPipelineDescriptor new];
    MTLComputePipelineDescriptor *compute = [MTLComputePipelineDescriptor new];
    NSError *error = nil;
    TestDevice *device = [TestDevice new]; device.name = @"fixture-gpu";
    setenv("MADEIRA_PIPELINE_CACHE", "0", 1);
    assert(madeira_new_render_pipeline(device, render, 0, NULL, &error, YES));
    assert(device.normal == 1 && device.archives == 0 && device.probes == 0);
    setenv("MADEIRA_PIPELINE_CACHE", "1", 1);
    // Missing cache must still produce the native pipeline; descriptor is untouched.
    assert(madeira_new_render_pipeline(device, render, 0, NULL, &error, YES));
    assert(device.normal == 2 && !render.binaryArchives.count);
    // Seed the exact filename that the production serializer will use. The
    // fixture device's archive reader accepts synthetic contents, not real GPU code.
    NSDirectoryEnumerator *walk = [NSFileManager.defaultManager enumeratorAtURL:base includingPropertiesForKeys:nil options:0 errorHandler:nil];
    NSURL *folder = walk.nextObject;
    assert(folder);
    NSArray *contents = [NSFileManager.defaultManager contentsOfDirectoryAtURL:folder includingPropertiesForKeys:nil options:0 error:nil];
    (void)contents;
    // Reuse a second device object to exercise disk lookup in a fresh state.
    // Wait for background training/serialization once, not between draw calls.
    for (unsigned i = 0; i < 120; i++) {
        usleep(100000);
        NSArray *files = [NSFileManager.defaultManager contentsOfDirectoryAtPath:folder.path error:nil];
        if ([files filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"SELF ENDSWITH '.metallib'"]].count) break;
    }
    seedFiles(base);
    TestDevice *miss = [TestDevice new]; miss.name = @"fixture-gpu";
    assert(madeira_new_compute_pipeline(miss, compute, 0, NULL, &error, YES));
    assert(miss.probes == 1 && miss.normal == 1 && !compute.binaryArchives.count);
    miss.cacheHit = YES; error = [NSError errorWithDomain:@"old" code:1 userInfo:nil];
    assert(madeira_new_render_pipeline(miss, render, 0, NULL, &error, YES));
    assert(miss.normal == 1 && miss.probes == 2 && error == nil);
    miss.cacheHit = NO; miss.failPipeline = YES;
    assert(!madeira_new_render_pipeline(miss, render, 0, NULL, &error, YES));
    assert([error.domain isEqual:@"test-pipeline-error"] && error.code == 7);
    unsigned archives = miss.archives, probes = miss.probes;
    render.binaryArchives = @[(id<MTLBinaryArchive>)[TestArchive new]];
    assert(!madeira_new_render_pipeline(miss, render, 0, NULL, &error, YES));
    assert(miss.archives == archives && miss.probes == probes); // preserve explicit caller policy
    render.binaryArchives = nil;
    TestDevice *corrupt = [TestDevice new]; corrupt.name = @"fixture-gpu"; corrupt.rejectArchive = YES;
    assert(madeira_new_render_pipeline(corrupt, render, 0, NULL, &error, YES));
    assert(corrupt.normal == 1 && !corrupt.probes);
    printf("PASS: disabled/missing/corrupt caches render normally; warm hits bypass compilation; misses/errors and explicit archive policies are preserved\n");
    // Background blocks still own archive snapshots; no app resource is removed here.
} return 0; }
'''
with tempfile.TemporaryDirectory(prefix='madeira-pipeline-cache-') as tmp:
    path = Path(tmp)
    (path / 'check.m').write_text(fixture, encoding='utf-8')
    subprocess.run([shutil.which('clang'), '-fobjc-arc', '-fblocks', '-Wno-protocol',
                    '-fsanitize=address,undefined', '-I', str(cache.parent),
                    '-framework', 'Foundation', '-framework', 'Metal', str(cache), str(path / 'check.m'),
                    '-o', str(path / 'check')], check=True)
    subprocess.run([str(path / 'check')], check=True, timeout=35)
