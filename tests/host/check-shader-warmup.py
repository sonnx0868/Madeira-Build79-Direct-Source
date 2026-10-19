#!/usr/bin/env python3
"""Exercise the real replay implementation with Metal protocol test doubles."""
import argparse
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
folder = root / "build/dxmt-ios"


def contracts():
    source = (folder / "pipeline_warmup_ios.m").read_text(encoding="utf-8")
    cache = (folder / "pipeline_cache_ios.m").read_text(encoding="utf-8")
    hooks = (folder / "pipeline-warmup.patch").read_text(encoding="utf-8")
    assert "MTLPipelineOptionFailOnBinaryArchiveMiss" in source
    assert "dispatch_sync" not in source and "dispatch_semaphore_wait" not in source
    assert "NSProcessInfoThermalStateSerious" in source and "start > 1.5" in source
    assert "[json(recipe(d, compute)) isEqual:keyBytes]" in source
    assert "learned-pipeline-replay-v1" in source
    assert "pipeline_warmup_ios.m" in (folder / "build.sh").read_text()
    assert "learned-pipeline-replay-v1" in (root / "scripts/check-ios-build.sh").read_text()
    assert "madeira_warmup_track_library" in hooks and "recorded, YES" in hooks
    assert "!info->logic_operation_enabled" in hooks
    assert "state ? state->warmup : nil" in cache


fixture = r'''
#import "pipeline_cache_ios.h"
#include <assert.h>
#include <stdatomic.h>
#include <unistd.h>
@interface FakeFunction : NSObject <MTLFunction>
@property(copy) NSString *name;
@property MTLFunctionType functionType;
@end
@implementation FakeFunction
- (MTLPatchType)patchType { return MTLPatchTypeNone; }
@end
@interface FakeLibrary : NSObject <MTLLibrary>
@property(strong) id<MTLDevice> device;
@end
@implementation FakeLibrary
- (id<MTLFunction>)newFunctionWithName:(NSString *)name {
    FakeFunction *f = [FakeFunction new]; f.name = name;
    f.functionType = [name isEqual:@"vertex"] ? MTLFunctionTypeVertex : [name isEqual:@"kernel"] ? MTLFunctionTypeKernel : MTLFunctionTypeFragment;
    return f;
}
- (id<MTLFunction>)newFunctionWithName:(NSString *)name constantValues:(MTLFunctionConstantValues *)v error:(NSError **)e {
    return [self newFunctionWithName:name];
}
@end
@interface FakeArchive : NSObject <MTLBinaryArchive>
@end
@implementation FakeArchive
- (BOOL)addRenderPipelineFunctionsWithDescriptor:(MTLRenderPipelineDescriptor *)d error:(NSError **)e { return YES; }
- (BOOL)addComputePipelineFunctionsWithDescriptor:(MTLComputePipelineDescriptor *)d error:(NSError **)e { return YES; }
- (BOOL)serializeToURL:(NSURL *)url error:(NSError **)e { return [@"test archive" writeToURL:url atomically:YES encoding:NSUTF8StringEncoding error:e]; }
@end
@interface FakeDevice : NSObject <MTLDevice> {
@public atomic_uint probes, normal;
}
@property BOOL rejectProbe;
@end
@implementation FakeDevice
- (NSString *)name { return @"warmup-fixture-gpu"; }
- (id<MTLBinaryArchive>)newBinaryArchiveWithDescriptor:(MTLBinaryArchiveDescriptor *)d error:(NSError **)e { return [FakeArchive new]; }
- (id<MTLLibrary>)newLibraryWithData:(dispatch_data_t)d error:(NSError **)e { FakeLibrary *l = [FakeLibrary new]; l.device = self; return l; }
- (id)makePipeline:(MTLPipelineOption)o {
    atomic_fetch_add(o & MTLPipelineOptionFailOnBinaryArchiveMiss ? &probes : &normal, 1);
    if ((o & MTLPipelineOptionFailOnBinaryArchiveMiss) && self.rejectProbe) return nil;
    return [NSObject new];
}
- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)d options:(MTLPipelineOption)o reflection:(MTLRenderPipelineReflection **)r error:(NSError **)e { return [self makePipeline:o]; }
- (id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)d options:(MTLPipelineOption)o reflection:(MTLComputePipelineReflection **)r error:(NSError **)e { return [self makePipeline:o]; }
@end
static NSURL *manifest(NSURL *base) {
    for (NSURL *u in [NSFileManager.defaultManager enumeratorAtURL:base includingPropertiesForKeys:nil options:0 errorHandler:nil])
        if ([u.lastPathComponent isEqual:@"recipes.json"]) return u;
    return nil;
}
int main(void) { @autoreleasepool {
    NSURL *base = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:NSUUID.UUID.UUIDString]];
    setenv("MADEIRA_PIPELINE_CACHE_DIR", base.path.UTF8String, 1);
    setenv("MADEIRA_SHADER_WARMUP", "1", 1);
    FakeDevice *first = [FakeDevice new];
    madeira_prepare_pipeline_cache(first);
    NSData *blob = [@"fixture metallib bytes" dataUsingEncoding:NSUTF8StringEncoding];
    dispatch_data_t data = dispatch_data_create(blob.bytes, blob.length, dispatch_get_global_queue(0, 0), ^{ (void)blob; });
    FakeLibrary *lib = [FakeLibrary new]; lib.device = first;
    madeira_warmup_track_library(lib, data);
    id<MTLFunction> vs = [lib newFunctionWithName:@"vertex"], ps = [lib newFunctionWithName:@"pixel"];
    madeira_warmup_track_function(lib, vs, @"vertex", @[], NO);
    uint32_t constant = 42;
    NSDictionary *c = madeira_warmup_constant(&constant, MTLDataTypeUInt, 7);
    assert(c && !madeira_warmup_constant(&constant, MTLDataTypeFloat4, 1));
    madeira_warmup_track_function(lib, ps, @"pixel", @[c], YES);
    MTLRenderPipelineDescriptor *render = [MTLRenderPipelineDescriptor new];
    render.vertexFunction = vs; render.fragmentFunction = ps;
    render.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    render.colorAttachments[0].blendingEnabled = YES;
    render.fragmentBuffers[3].mutability = MTLMutabilityImmutable;
    madeira_pipeline_allow_warmup(render, YES);
    id<MTLFunction> kernel = [lib newFunctionWithName:@"kernel"];
    madeira_warmup_track_function(lib, kernel, @"kernel", @[c], YES);
    MTLComputePipelineDescriptor *compute = [MTLComputePipelineDescriptor new]; compute.computeFunction = kernel;
    compute.threadGroupSizeIsMultipleOfThreadExecutionWidth = YES;
    madeira_pipeline_allow_warmup(compute, YES);
    assert(madeira_new_render_pipeline(first, render, 0, NULL, NULL, YES));
    assert(madeira_new_compute_pipeline(first, compute, 0, NULL, NULL, YES));
    assert(atomic_load(&first->normal) == 2); // cold replay didn't compile anything
    NSURL *saved = nil;
    for (unsigned i = 0; i < 100 && !(saved = manifest(base)); ++i) usleep(50000);
    assert(saved);
    NSDictionary *inventory = [NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:saved] options:0 error:nil];
    NSLog(@"[warmup-fixture] render-class=%@ compute-class=%@ render-public-class=%d compute-public-class=%d vertex-layout=%d recipes=%@",
          NSStringFromClass(render.class), NSStringFromClass(compute.class),
          [render isKindOfClass:MTLRenderPipelineDescriptor.class], [compute isKindOfClass:MTLComputePipelineDescriptor.class],
          render.vertexDescriptor != nil, [inventory[@"recipes"] valueForKey:@"kind"]);
    assert([inventory[@"recipes"] count] == 2);
    NSURL *warmFolder = saved.URLByDeletingLastPathComponent;
    NSURL *archive = [[warmFolder URLByDeletingPathExtension] URLByAppendingPathExtension:@"metallib"];
    [@"test archive" writeToURL:archive atomically:YES encoding:NSUTF8StringEncoding error:nil];
    FakeDevice *second = [FakeDevice new]; madeira_prepare_pipeline_cache(second);
    for (unsigned i = 0; i < 100 && atomic_load(&second->probes) != 2; ++i) usleep(50000);
    usleep(100000); // completed worker publishes its pipeline after the mock call
    assert(atomic_load(&second->probes) == 2 && !atomic_load(&second->normal));
    MTLVertexDescriptor *custom = [MTLVertexDescriptor new];
    custom.attributes[0].format = MTLVertexFormatFloat3; custom.layouts[0].stride = 12;
    render.vertexDescriptor = custom;
    assert([madeira_pipeline_warmup_for_device(second) take:render compute:NO] == nil);
    render.vertexDescriptor = nil; // empty/default descriptors remain eligible
    assert(madeira_new_render_pipeline(second, render, 0, NULL, NULL, YES));
    assert(madeira_new_compute_pipeline(second, compute, 0, NULL, NULL, YES));
    assert(atomic_load(&second->probes) == 2); // both exact warm PSOs reused
    render.colorAttachments[0].blendingEnabled = NO;
    assert(madeira_new_render_pipeline(second, render, 0, NULL, NULL, YES));
    assert(atomic_load(&second->probes) == 3); // changed descriptor cannot use warm PSO
    // An unsupported/private descriptor is never used for replay.
    madeira_pipeline_allow_warmup(render, NO);
    assert(madeira_new_render_pipeline(second, render, 0, NULL, NULL, YES));
    assert(atomic_load(&second->probes) == 4);
    // Corrupt library bytes cannot be submitted to Metal, but real game
    // requests still fall back to the normal cache/creation path.
    NSURL *libraryFile = nil;
    for (NSURL *u in [NSFileManager.defaultManager contentsOfDirectoryAtURL:warmFolder includingPropertiesForKeys:nil options:0 error:nil])
        if ([u.pathExtension isEqual:@"metallib"]) { libraryFile = u; [@"corrupt" writeToURL:u atomically:YES encoding:NSUTF8StringEncoding error:nil]; }
    FakeDevice *third = [FakeDevice new]; madeira_prepare_pipeline_cache(third); usleep(300000);
    assert(!atomic_load(&third->probes));
    assert(madeira_new_compute_pipeline(third, compute, 0, NULL, NULL, YES));
    assert(atomic_load(&third->probes) == 1);
    FakeLibrary *repair = [FakeLibrary new]; repair.device = third; madeira_warmup_track_library(repair, data);
    for (unsigned i = 0; i < 100 && ![[NSData dataWithContentsOfURL:libraryFile] isEqual:blob]; ++i) usleep(50000);
    assert([[NSData dataWithContentsOfURL:libraryFile] isEqual:blob]);
    // A valid library with corrupt function-stage metadata must also skip,
    // without ever handing a nil/wrong function to Metal's asserting setters.
    NSURL *stageFolder = [base URLByAppendingPathComponent:@"invalid-stages"];
    [NSFileManager.defaultManager createDirectoryAtURL:stageFolder withIntermediateDirectories:YES attributes:nil error:nil];
    [blob writeToURL:[stageFolder URLByAppendingPathComponent:libraryFile.lastPathComponent] atomically:YES];
    NSMutableDictionary *bad = [NSJSONSerialization JSONObjectWithData:[NSJSONSerialization dataWithJSONObject:inventory options:0 error:nil]
                                                              options:NSJSONReadingMutableContainers error:nil];
    for (NSMutableDictionary *r in bad[@"recipes"]) {
        if ([r[@"kind"] isEqual:@"compute"]) r[@"function"][@"name"] = @"vertex";
        else r[@"vertex"][@"name"] = @"kernel";
    }
    [[NSJSONSerialization dataWithJSONObject:bad options:0 error:nil] writeToURL:[stageFolder URLByAppendingPathComponent:@"recipes.json"] atomically:YES];
    FakeDevice *wrong = [FakeDevice new];
    MadeiraPipelineWarmup *wrongWarmup = [[MadeiraPipelineWarmup alloc] initWithDevice:wrong folder:stageFolder archive:[FakeArchive new]];
    assert(wrongWarmup); usleep(300000);
    assert(!atomic_load(&wrong->probes) && !atomic_load(&wrong->normal));
    FakeDevice *miss = [FakeDevice new]; miss.rejectProbe = YES; madeira_prepare_pipeline_cache(miss);
    for (unsigned i = 0; i < 100 && atomic_load(&miss->probes) != 2; ++i) usleep(50000);
    assert(!atomic_load(&miss->normal)); // replay never falls back to a cold compile
    assert(madeira_new_compute_pipeline(miss, compute, 0, NULL, NULL, YES));
    assert(atomic_load(&miss->normal) == 1); // the actual request compiles normally
    setenv("MADEIRA_PIPELINE_CACHE", "0", 1);
    assert(!madeira_pipeline_warmup_for_device(third)); // no nil cache dereference
    assert(madeira_new_render_pipeline(third, render, 0, NULL, NULL, YES));
    assert(atomic_load(&third->normal) == 1);
    // Loading many later scenes must not evict the startup inventory.
    // Exercise the real asynchronous recorder and saved manifest, not a model.
    NSURL *priorityFolder = [base URLByAppendingPathComponent:@"priority"];
    MadeiraPipelineWarmup *priority = [[MadeiraPipelineWarmup alloc] initWithDevice:first folder:priorityFolder archive:nil];
    for (unsigned i = 0; i < 342; ++i) {
        FakeFunction *fn = [FakeFunction new]; fn.functionType = MTLFunctionTypeKernel;
        fn.name = i < 32 ? [NSString stringWithFormat:@"intro.%u", i] :
                  i < 42 ? @"popular" : [NSString stringWithFormat:@"scene.%u", i];
        madeira_warmup_track_function(lib, fn, fn.name, @[], NO);
        MTLComputePipelineDescriptor *d = [MTLComputePipelineDescriptor new]; d.computeFunction = fn;
        madeira_pipeline_allow_warmup(d, YES);
        [priority record:d compute:YES];
        if (i % 16 == 0) usleep(10000); // Let the bounded writer drain.
    }
    NSURL *priorityFile = [priorityFolder URLByAppendingPathComponent:@"recipes.json"];
    NSDictionary *kept = nil;
    for (unsigned i = 0; i < 80; ++i) {
        usleep(50000);
        kept = [NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfURL:priorityFile] ?: [NSData data] options:0 error:nil];
        if ([kept[@"recipes"] count] == 256) break;
    }
    assert([kept[@"schema"] isEqual:@2] && [kept[@"recipes"] count] == 256 && [kept[@"startup"] count] == 32);
    NSMutableSet *names = [NSMutableSet set];
    for (NSDictionary *r in kept[@"recipes"]) [names addObject:r[@"function"][@"name"]];
    for (unsigned i = 0; i < 32; ++i) assert([names containsObject:[NSString stringWithFormat:@"intro.%u", i]]);
    assert([names containsObject:@"popular"] && ![names containsObject:@"scene.42"]);
    printf("PASS: cold learning, exact render/compute replay, constants, changed/private descriptors, corrupt libraries and disabled cache\n");
    printf("PASS: startup-hot-v2 retains startup and popular pipelines across 300 later scene pipelines\n");
} return 0; }
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-only", action="store_true")
    args = parser.parse_args()
    contracts()
    if args.source_only:
        print("PASS: integration source contracts; native replay fixture NOT RUN")
        return
    if platform.system() != "Darwin":
        raise SystemExit("Native shader replay fixture requires macOS Metal/Foundation")
    with tempfile.TemporaryDirectory(prefix="madeira-shader-warmup-") as scratch:
        tmp = Path(scratch)
        (tmp / "check.m").write_text(fixture, encoding="utf-8")
        subprocess.run([shutil.which("clang"), "-fobjc-arc", "-fblocks", "-Wno-protocol", "-fsanitize=address,undefined",
            "-Wno-objc-protocol-property-synthesis", "-I", str(folder), "-framework", "Foundation", "-framework", "Metal",
            str(folder / "pipeline_cache_ios.m"), str(folder / "pipeline_warmup_ios.m"),
            str(tmp / "check.m"), "-o", str(tmp / "check")], check=True)
        subprocess.run([str(tmp / "check")], check=True, timeout=20)


if __name__ == "__main__":
    main()
