// Actual Metal tests for optional dispatch, zero extension, state restoration,
// changing input views, completion ownership and bounded reuse. No game files.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <dispatch/dispatch.h>
#include "padded_lighting.m"
#include <stdlib.h>
static void check(BOOL value, const char *message) {
  if (!value) { fprintf(stderr, "padded-light-test: %s\n", message); exit(21); }
}
static void complete(id<MTLCommandBuffer> command) {
  dispatch_semaphore_t done = dispatch_semaphore_create(0);
  [command addCompletedHandler:^(id<MTLCommandBuffer> completed) { dispatch_semaphore_signal(done); }];
  [command commit]; [command waitUntilCompleted];
  check(!dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)), "completion timeout");
  check(command.status == MTLCommandBufferStatusCompleted, "GPU failure");
}
static NSString *const source =
    @"#include <metal_stdlib>\nusing namespace metal;\n"
     "struct Root {array<ulong,19>head [[id(0)]];device const uint4*lights [[id(19)]];ulong bytes [[id(20)]];};\n"
     "kernel void shader_f4c2c563(constant Root&r [[buffer(30)]],device uint4*out [[buffer(26)]],"
     "uint3 g [[threadgroup_position_in_grid]],uint3 t [[thread_position_in_threadgroup]]){"
     "uint x=g.x*64+t.y*8+t.x;if(x<128)out[x]=x<(uint(r.bytes)>>4)?r.lights[x]:uint4(0);}\n"
     "kernel void madeira_lighting_padded_f4c2c563(constant Root&r [[buffer(30)]],device uint4*out [[buffer(26)]],"
     "uint3 g [[threadgroup_position_in_grid]],uint3 t [[thread_position_in_threadgroup]]){"
     "uint x=g.x*64+t.y*8+t.x;if(x<128)out[x]=r.lights[x];}\n"
     "kernel void probe(constant ulong*r [[buffer(30)]],constant uint4&a [[buffer(27)]],"
     "device const uint4*b [[buffer(28)]],device ulong*out [[buffer(26)]]){"
     "out[0]=a[0];out[1]=a[3];out[2]=b[0][0];out[3]=b[0][3];out[4]=r[19];out[5]=r[20];}\n";

static void runCase(id<MTLDevice> device, id<MTLCommandQueue> queue, id<MTLComputePipelineState> original,
                    id<MTLComputePipelineState> probe, id<MTLBuffer> data, unsigned length,
                    unsigned start, unsigned mode, unsigned repetitions) {
  @autoreleasepool {
    id<MTLBuffer> root = [device newBufferWithLength:600 options:MTLResourceStorageModeShared];
    uint64_t words[75] = {0}; words[19] = data.gpuAddress + start;
    words[20] = UINT64_C(0x1234567800000000) | length;
    memcpy(root.contents, words, sizeof words);
    id<MTLBuffer> boundRoot = root;
    if (mode == 2) {
      boundRoot = [device newBufferWithLength:600 options:MTLResourceStorageModePrivate];
      id<MTLCommandBuffer> upload = [queue commandBuffer];
      id<MTLBlitCommandEncoder> blit = [upload blitCommandEncoder];
      [blit copyFromBuffer:root sourceOffset:0 toBuffer:boundRoot destinationOffset:0 size:600];
      [blit endEncoding]; complete(upload);
    }
    id<MTLBuffer> output = [device newBufferWithLength:128 * 16 options:MTLResourceStorageModeShared];
    id<MTLBuffer> report = [device newBufferWithLength:64 options:MTLResourceStorageModeShared];
    id<MTLBuffer> sentinel = [device newBufferWithLength:32 options:MTLResourceStorageModeShared];
    uint32_t marker[4] = {21, 22, 23, 24};
    memcpy((char *)sentinel.contents + 16, marker, sizeof marker);
    id<MTLBuffer> arguments = [device newBufferWithLength:16 options:MTLResourceStorageModeShared];
    uint32_t groups[4] = {mode == 4 ? 0 : 2, 1, 1, 0}; memcpy(arguments.contents, groups, sizeof groups);
    memset(output.contents, 0x6a, output.length);
    id<MTLCommandBuffer> command = [queue commandBuffer];
    id<MTLComputeCommandEncoder> encoder = [command computeCommandEncoder]; mpl_encoder(command, encoder);
    [encoder setComputePipelineState:original]; mpl_pipeline(encoder, original);
    if (mode == 1) { [encoder setBytes:words length:600 atIndex:30]; mpl_bytes(encoder, words, 600, 30); }
    else { [encoder setBuffer:boundRoot offset:0 atIndex:30]; mpl_buffer(encoder, boundRoot, 0, 30); mpl_offset(encoder, 0, 30); }
    [encoder setBuffer:output offset:0 atIndex:26];
    [encoder setBytes:marker length:sizeof marker atIndex:27];
    [encoder setBuffer:sentinel offset:16 atIndex:28]; mpl_buffer(encoder, sentinel, 16, 28);
    [encoder useResource:data usage:MTLResourceUsageRead];
    if (mode != 3) mpl_resource(encoder, data);
    unsigned active = 0;
    for (unsigned n = 0; n < repetitions; ++n) {
      BOOL indirect = mode == 4 || mode == 5;
      BOOL handled = mpl_dispatch(encoder, MTLSizeMake(2, 1, 1), MTLSizeMake(8, 8, 1),
                                  indirect ? arguments : nil, 0);
      active += handled;
      if (!handled) {
        if (indirect) [encoder dispatchThreadgroupsWithIndirectBuffer:arguments indirectBufferOffset:0 threadsPerThreadgroup:MTLSizeMake(8, 8, 1)];
        else [encoder dispatchThreadgroups:MTLSizeMake(2, 1, 1) threadsPerThreadgroup:MTLSizeMake(8, 8, 1)];
      }
    }
    check(active == ((mode == 2 || mode == 3) ? 0 : MIN(repetitions, MPLSlots)), "incorrect activation or bounded fallback");
    check(atomic_load(&mpl_slots) == active, "lease count before commit");
    [encoder memoryBarrierWithScope:MTLBarrierScopeBuffers];
    [encoder setComputePipelineState:probe];
    [encoder setBuffer:report offset:0 atIndex:26];
    [encoder dispatchThreadgroups:MTLSizeMake(1, 1, 1) threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
    [encoder endEncoding]; complete(command);
    check(!atomic_load(&mpl_slots), "pending scratch after GPU completion");
    MPLEncoder *tracked = objc_getAssociatedObject(encoder, &mpl_encoder_key);
    check(!tracked.resources.count && !tracked.bindings.count && !tracked.pipeline && !tracked.command,
          "ended encoder kept buffer references after GPU completion");
    check(!memcmp(root.contents, words, 600), "caller root table was mutated");
    uint64_t *observed = report.contents;
    check(observed[0] == 21 && observed[1] == 24 && observed[2] == 21 && observed[3] == 24 &&
          observed[4] == words[19] && observed[5] == words[20], "Metal bindings were not restored");
    uint32_t *values = output.contents, *input = (uint32_t *)((char *)data.contents + start);
    for (unsigned index = 0; index < 128 * 4; ++index) {
      uint32_t expected = mode == 4 ? 0x6a6a6a6a : index * 4 < length ? input[index] : 0;
      check(values[index] == expected, "view data or zero extension differs");
    }
#if !__has_feature(objc_arc)
    if (boundRoot != root) [boundRoot release];
    [root release]; [output release]; [report release]; [sentinel release]; [arguments release];
#endif
  }
}

int main(void) {
  @autoreleasepool {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice(); check(device != nil, "no Metal device");
    id<MTLCommandQueue> queue = [device newCommandQueue]; NSError *error = nil;
    id<MTLLibrary> library = [device newLibraryWithSource:source options:nil error:&error]; check(library && !error, "test library");
    id<MTLFunction> function = [library newFunctionWithName:@"shader_f4c2c563"]; mpl_track_function(library, function, nil);
    MTLComputePipelineDescriptor *descriptor = [MTLComputePipelineDescriptor new];
    descriptor.computeFunction = function; descriptor.maxTotalThreadsPerThreadgroup = 64;
    id<MTLComputePipelineState> original = [device newComputePipelineStateWithDescriptor:descriptor options:MTLPipelineOptionNone reflection:nil error:&error];
    check(original && !error, "test original pipeline"); mpl_track_pipeline(device, original, descriptor, MTLPipelineOptionNone);
    check(objc_getAssociatedObject(original, &mpl_pipeline_key) != nil, "optional pipeline creation");
    id<MTLFunction> probeFunction = [library newFunctionWithName:@"probe"];
    id<MTLComputePipelineState> probe = [device newComputePipelineStateWithFunction:probeFunction error:&error]; check(probe && !error, "probe pipeline");
    id<MTLBuffer> data = [device newBufferWithLength:4096 options:MTLResourceStorageModeShared];
    for (unsigned i = 0; i < data.length / 4; ++i) ((uint32_t *)data.contents)[i] = 0x12340000 + i;
    // A previously larger view must not expose stale tail data when reused.
    runCase(device, queue, original, probe, data, 1024, 0, 0, 1);
    runCase(device, queue, original, probe, data, 64, 0, 0, 1);
    runCase(device, queue, original, probe, data, 256, 16, 1, 1);
    for (unsigned i = 0; i < data.length / 4; ++i) ((uint32_t *)data.contents)[i] = 0x76540000 + i;
    runCase(device, queue, original, probe, data, 512, 32, 5, 1);
    runCase(device, queue, original, probe, data, 64, 0, 2, 1);
    runCase(device, queue, original, probe, data, 64, 0, 3, 1);
    runCase(device, queue, original, probe, data, 64, 0, 4, 1);
    runCase(device, queue, original, probe, data, 64, 0, 0, MPLSlots + 1);
    check(!atomic_load(&mpl_slots) && mpl_available != nil, "bounded idle reuse");
    mpl_drop_cache(); check(mpl_available == nil, "cache trim");
    printf("PASS: 8 native Metal cases; updated data, shorter/offset/inline/private views, indirect/zero dispatch, slot restoration, bounded fallback, GPU completion leases and cache trim\n");
#if !__has_feature(objc_arc)
    [data release]; [probe release]; [probeFunction release]; [original release]; [descriptor release]; [function release]; [library release]; [queue release]; [device release];
#endif
  }
  return 0;
}
