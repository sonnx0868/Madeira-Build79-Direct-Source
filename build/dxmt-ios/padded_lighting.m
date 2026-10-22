// Optional full-resolution light-table path. A failed prerequisite uses the
// original dispatch. Temporary buffers live through GPU completion and have a
// process-wide limit; nothing signals or bypasses a frame fence.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <objc/runtime.h>
#include <stdatomic.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <TargetConditionals.h>

#if __has_feature(objc_arc)
#define MPL_RELEASE(value) ((void)0)
#define MPL_RETAIN(value) (value)
#else
#define MPL_RELEASE(value) [(value) release]
#define MPL_RETAIN(value) [(value) retain]
#endif

// Seven pending uses plus one idle allocation bound total scratch to 112 MiB.
enum { MPLBytes = 65536 * 56 * 4 + 16, MPLSlots = 7 };
static _Atomic unsigned mpl_slots;
static _Atomic unsigned mpl_announced;
static char mpl_function_key, mpl_pipeline_key, mpl_encoder_key;

static NSString *const mpl_prepare_source =
    @"#include <metal_stdlib>\nusing namespace metal;\n"
     "struct Root {array<ulong,19> head [[id(0)]];"
     "device const uint4 *lights [[id(19)]];ulong bytes [[id(20)]];};\n"
     "kernel void madeira_prepare_padded_lights(constant Root &root [[buffer(30)]],"
     "device uint4 *output [[buffer(28)]],"
     "uint index [[thread_position_in_grid]]) {"
     "if(index>=917505u)return;"
     "if(index<(uint(root.bytes)>>4))output[index]=root.lights[index];"
     "else output[index]=uint4(0);}\n";

@interface MPLPlan : NSObject
@property(strong) id<MTLComputePipelineState> fast;
@property(strong) id<MTLComputePipelineState> prepare;
@end
@implementation MPLPlan
- (void)dealloc {
  MPL_RELEASE(_fast); MPL_RELEASE(_prepare);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
@end

@interface MPLBinding : NSObject
@property(strong) id<MTLBuffer> buffer;
@property(strong) NSData *bytes;
@property NSUInteger offset;
@property BOOL invalid;
@end
@implementation MPLBinding
- (void)dealloc {
  MPL_RELEASE(_buffer); MPL_RELEASE(_bytes);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
@end

@interface MPLAllocation : NSObject
@property(strong) id<MTLBuffer> buffer;
@property NSUInteger dirtyBytes;
@property BOOL initialized;
@end
@implementation MPLAllocation
- (void)dealloc {
  MPL_RELEASE(_buffer);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
@end
static MPLAllocation *mpl_available;

@interface MPLLease : NSObject
@property(strong) MPLAllocation *allocation;
@property NSUInteger nextDirtyBytes;
@property BOOL gpuCompleted;
@property BOOL reserved;
@end
@implementation MPLLease
- (void)dealloc {
  if (_gpuCompleted) @synchronized([MPLLease class]) {
    if (!mpl_available) mpl_available = MPL_RETAIN(_allocation);
  }
  MPL_RELEASE(_allocation);
  if (_reserved) atomic_fetch_sub_explicit(&mpl_slots, 1, memory_order_relaxed);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
@end

static void mpl_drop_cache(void) {
  @synchronized([MPLLease class]) {
    MPL_RELEASE(mpl_available); mpl_available = nil;
  }
}

@interface MPLEncoder : NSObject
@property(assign) id<MTLCommandBuffer> command;
@property(strong) id<MTLComputePipelineState> pipeline;
@property(strong) NSMutableDictionary<NSNumber *, MPLBinding *> *bindings;
@property(strong) NSMutableArray<id<MTLBuffer>> *resources;
@property(strong) NSMutableArray<MPLLease *> *leases;
@end
@implementation MPLEncoder
- (instancetype)init {
  if (!(self = [super init])) return nil;
  self.bindings = [NSMutableDictionary dictionary];
  self.resources = [NSMutableArray array];
  self.leases = [NSMutableArray array];
  return self;
}
- (void)dealloc {
  MPL_RELEASE(_pipeline); MPL_RELEASE(_bindings); MPL_RELEASE(_resources); MPL_RELEASE(_leases);
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
@end

static void mpl_track_function(id<MTLLibrary> library, id<MTLFunction> function,
                               MTLFunctionConstantValues *values) {
  if (!function || ![function.name isEqualToString:@"shader_f4c2c563"] ||
      ![library.functionNames containsObject:@"madeira_lighting_padded_f4c2c563"]) return;
  NSError *error = nil;
  id<MTLFunction> fast = values
      ? [library newFunctionWithName:@"madeira_lighting_padded_f4c2c563" constantValues:values error:&error]
      : [library newFunctionWithName:@"madeira_lighting_padded_f4c2c563"];
  if (!fast || error) { MPL_RELEASE(fast); return; }
  objc_setAssociatedObject(function, &mpl_function_key, fast, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  MPL_RELEASE(fast);
}

static void mpl_track_pipeline(id<MTLDevice> device, id<MTLComputePipelineState> pipeline,
                              MTLComputePipelineDescriptor *descriptor, MTLPipelineOption options) {
  id<MTLFunction> function = objc_getAssociatedObject(descriptor.computeFunction, &mpl_function_key);
  if (!pipeline || !function || (options & MTLPipelineOptionFailOnBinaryArchiveMiss) ||
      objc_getAssociatedObject(pipeline, &mpl_pipeline_key)) return;
  // Compile before the first dispatch. No runtime shader compilation or
  // library lookup is performed on the rendering thread.
  MTLComputePipelineDescriptor *copy = [descriptor copy];
  copy.computeFunction = function; copy.binaryArchives = nil;
  NSError *error = nil;
  id<MTLComputePipelineState> fast = nil;
#if TARGET_OS_IOS && !defined(MADEIRA_PADDED_LIGHT_STANDALONE)
  fast = madeira_new_compute_pipeline(device, copy, MTLPipelineOptionNone, NULL, &error, YES);
#else
  fast = [device newComputePipelineStateWithDescriptor:copy options:MTLPipelineOptionNone reflection:nil error:&error];
#endif
  MPL_RELEASE(copy);
  if (!fast || error) { MPL_RELEASE(fast); return; }
  id<MTLComputePipelineState> prepare = nil;
  @synchronized([MPLPlan class]) {
    static NSMapTable *cache;
    if (!cache) cache = [NSMapTable strongToStrongObjectsMapTable];
#if !__has_feature(objc_arc)
    // The process owns this bounded per-device cache.
    static BOOL retained;
    if (!retained) { [cache retain]; retained = YES; }
#endif
    prepare = [cache objectForKey:device];
    if (!prepare) {
      id<MTLLibrary> library = [device newLibraryWithSource:mpl_prepare_source options:nil error:&error];
      id<MTLFunction> kernel = [library newFunctionWithName:@"madeira_prepare_padded_lights"];
      if (kernel && !error) {
        prepare = [device newComputePipelineStateWithFunction:kernel error:&error];
        if (prepare && !error) [cache setObject:prepare forKey:device];
        MPL_RELEASE(prepare);
        prepare = [cache objectForKey:device];
      }
      MPL_RELEASE(kernel); MPL_RELEASE(library);
    }
  }
  if (!prepare || error) { MPL_RELEASE(fast); return; }
#if TARGET_OS_IOS && !defined(MADEIRA_PADDED_LIGHT_STANDALONE)
  static dispatch_once_t observers;
  dispatch_once(&observers, ^{
    NSNotificationCenter *center = NSNotificationCenter.defaultCenter;
    [center addObserverForName:UIApplicationDidReceiveMemoryWarningNotification object:nil queue:nil
                   usingBlock:^(NSNotification *notification) { mpl_drop_cache(); }];
    [center addObserverForName:UIApplicationDidEnterBackgroundNotification object:nil queue:nil
                   usingBlock:^(NSNotification *notification) { mpl_drop_cache(); }];
  });
#endif
  MPLPlan *plan = [[MPLPlan alloc] init];
  plan.fast = fast; plan.prepare = prepare;
  objc_setAssociatedObject(pipeline, &mpl_pipeline_key, plan, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  MPL_RELEASE(plan); MPL_RELEASE(fast);
}

static void mpl_encoder(id<MTLCommandBuffer> command, id<MTLComputeCommandEncoder> encoder) {
  if (!command || !encoder) return;
  MPLEncoder *state = [[MPLEncoder alloc] init];
  state.command = command;
  objc_setAssociatedObject(encoder, &mpl_encoder_key, state, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
  // Clear tracked references at completion even if this encoder never selects
  // the optional kernel. Capture/profiling code may retain an ended encoder.
  [command addCompletedHandler:^(id<MTLCommandBuffer> completed) {
    for (MPLLease *pending in state.leases) {
      pending.gpuCompleted = completed.status == MTLCommandBufferStatusCompleted;
      if (pending.gpuCompleted) {
        pending.allocation.initialized = YES;
        pending.allocation.dirtyBytes = pending.nextDirtyBytes;
      }
    }
    [state.leases removeAllObjects];
    [state.resources removeAllObjects];
    [state.bindings removeAllObjects];
    state.pipeline = nil; state.command = nil;
  }];
  MPL_RELEASE(state);
}

static MPLBinding *mpl_binding(MPLEncoder *state, NSUInteger index) {
  MPLBinding *binding = state.bindings[@(index)];
  if (!binding) {
    binding = [[MPLBinding alloc] init];
    state.bindings[@(index)] = binding;
    MPL_RELEASE(binding);
  }
  return binding;
}

static BOOL mpl_slot(NSUInteger index) { return index == 28 || index == 30; }

static void mpl_buffer(id<MTLComputeCommandEncoder> encoder, id<MTLBuffer> buffer,
                       NSUInteger offset, NSUInteger index) {
  if (!mpl_slot(index)) return;
  MPLEncoder *state = objc_getAssociatedObject(encoder, &mpl_encoder_key);
  if (!state) return;
  MPLBinding *binding = mpl_binding(state, index);
  binding.bytes = nil; binding.buffer = buffer; binding.offset = offset; binding.invalid = NO;
}

static void mpl_bytes(id<MTLComputeCommandEncoder> encoder, const void *bytes,
                      NSUInteger length, NSUInteger index) {
  if (!mpl_slot(index)) return;
  MPLEncoder *state = objc_getAssociatedObject(encoder, &mpl_encoder_key);
  if (!state) return;
  MPLBinding *binding = mpl_binding(state, index);
  binding.buffer = nil; binding.offset = 0;
  binding.invalid = length > 4096 || (length && !bytes);
  binding.bytes = binding.invalid ? nil : [NSData dataWithBytes:bytes length:length];
}

static void mpl_offset(id<MTLComputeCommandEncoder> encoder, NSUInteger offset, NSUInteger index) {
  if (!mpl_slot(index)) return;
  MPLEncoder *state = objc_getAssociatedObject(encoder, &mpl_encoder_key);
  if (!state) return;
  MPLBinding *binding = mpl_binding(state, index);
  binding.offset = offset;
  if (binding.bytes) binding.invalid = YES;
}

static void mpl_pipeline(id<MTLComputeCommandEncoder> encoder, id<MTLComputePipelineState> pipeline) {
  MPLEncoder *state = objc_getAssociatedObject(encoder, &mpl_encoder_key);
  state.pipeline = pipeline;
}

static void mpl_resource(id<MTLComputeCommandEncoder> encoder, id<MTLResource> resource) {
  if (![resource conformsToProtocol:@protocol(MTLBuffer)]) return;
  MPLEncoder *state = objc_getAssociatedObject(encoder, &mpl_encoder_key);
  if (state && ![state.resources containsObject:(id<MTLBuffer>)resource]) [state.resources addObject:(id<MTLBuffer>)resource];
}

static void mpl_restore(id<MTLComputeCommandEncoder> encoder, MPLBinding *binding, NSUInteger index) {
  if (binding.bytes) [encoder setBytes:binding.bytes.bytes length:binding.bytes.length atIndex:index];
  else [encoder setBuffer:binding.buffer offset:binding ? binding.offset : 0 atIndex:index];
}

static MPLLease *mpl_new_lease(id<MTLDevice> device) __attribute__((ns_returns_retained));
static MPLLease *mpl_new_lease(id<MTLDevice> device) {
  if (atomic_fetch_add_explicit(&mpl_slots, 1, memory_order_relaxed) >= MPLSlots) {
    atomic_fetch_sub_explicit(&mpl_slots, 1, memory_order_relaxed); return nil;
  }
  MPLLease *lease = [[MPLLease alloc] init];
  if (!lease) { atomic_fetch_sub_explicit(&mpl_slots, 1, memory_order_relaxed); return nil; }
  lease.reserved = YES;
  MPLAllocation *allocation = nil;
  @synchronized([MPLLease class]) {
    if (mpl_available.buffer.device == device) {
      allocation = MPL_RETAIN(mpl_available);
      MPL_RELEASE(mpl_available); mpl_available = nil;
    }
  }
  if (!allocation) {
    allocation = [[MPLAllocation alloc] init];
    id<MTLBuffer> buffer = [device newBufferWithLength:MPLBytes options:MTLResourceStorageModePrivate];
    allocation.buffer = buffer; MPL_RELEASE(buffer);
  }
  lease.allocation = allocation; MPL_RELEASE(allocation);
  if (!lease.allocation.buffer) { MPL_RELEASE(lease); return nil; }
  return lease;
}

static BOOL mpl_dispatch(id<MTLComputeCommandEncoder> encoder, MTLSize groups, MTLSize threads,
                          id<MTLBuffer> indirect, NSUInteger indirectOffset) {
  MPLEncoder *state = objc_getAssociatedObject(encoder, &mpl_encoder_key);
  MPLPlan *plan = objc_getAssociatedObject(state.pipeline, &mpl_pipeline_key);
  if (!plan || !state.command || threads.width != 8 || threads.height != 8 || threads.depth != 1) return NO;
  if ((!indirect && (!groups.width || !groups.height || !groups.depth)) ||
      (indirect && (indirectOffset > indirect.length || indirect.length - indirectOffset < 12 || (indirectOffset & 3)))) return NO;
  MPLBinding *binding = state.bindings[@30];
  if (!binding || binding.invalid || state.bindings[@28].invalid) return NO;
  uint64_t root[75];
  if (binding.bytes) {
    if (binding.bytes.length != sizeof root) return NO;
    memcpy(root, binding.bytes.bytes, sizeof root);
  } else {
    if (!binding.buffer || binding.buffer.storageMode != MTLStorageModeShared || binding.offset > binding.buffer.length ||
        binding.buffer.length - binding.offset < sizeof root || !binding.buffer.contents || (binding.offset & 7)) return NO;
    memcpy(root, (const char *)binding.buffer.contents + binding.offset, sizeof root);
  }
  uint64_t address = root[19];
  uint32_t bytes = (uint32_t)root[20];
  if (!address || !bytes || bytes > MPLBytes || (address & 15) || (bytes & 15)) return NO;
  id<MTLBuffer> source = nil;
  for (id<MTLBuffer> resource in state.resources) {
    uint64_t base = resource.gpuAddress;
    if (address >= base && address - base <= resource.length && bytes <= resource.length - (address - base)) {
      source = resource; break;
    }
  }
  if (!source) return NO;
  MPLLease *lease = mpl_new_lease(encoder.device);
  if (!lease) return NO;
  [state.leases addObject:lease];
  id<MTLBuffer> padded = lease.allocation.buffer;
  NSUInteger preparedBytes = lease.allocation.initialized ? MAX(lease.allocation.dirtyBytes, bytes) : MPLBytes;
  lease.nextDirtyBytes = bytes;
  MPL_RELEASE(lease);
  [encoder useResource:source usage:MTLResourceUsageRead];
  [encoder useResource:padded usage:MTLResourceUsageRead | MTLResourceUsageWrite];
  [encoder memoryBarrierWithScope:MTLBarrierScopeBuffers];
  [encoder setComputePipelineState:plan.prepare];
  [encoder setBuffer:padded offset:0 atIndex:28];
  [encoder dispatchThreads:MTLSizeMake(preparedBytes / 16, 1, 1) threadsPerThreadgroup:MTLSizeMake(128, 1, 1)];
  id<MTLResource> barrier[1] = {padded};
  [encoder memoryBarrierWithResources:barrier count:1];
  root[19] = padded.gpuAddress;
  root[20] = (root[20] & ~UINT64_C(0xffffffff)) | MPLBytes;
  [encoder setBytes:root length:sizeof root atIndex:30];
  [encoder setComputePipelineState:plan.fast];
  if (indirect) [encoder dispatchThreadgroupsWithIndirectBuffer:indirect indirectBufferOffset:indirectOffset threadsPerThreadgroup:threads];
  else [encoder dispatchThreadgroups:groups threadsPerThreadgroup:threads];
  [encoder setComputePipelineState:state.pipeline];
  mpl_restore(encoder, state.bindings[@28], 28);
  mpl_restore(encoder, binding, 30);
  if (!atomic_exchange_explicit(&mpl_announced, 1, memory_order_relaxed))
    fprintf(stderr, "[lighting] padded light table active: full-resolution math, bounded scratch=%u bytes, original fallback available\n", MPLBytes);
  return YES;
}

#undef MPL_RELEASE
#undef MPL_RETAIN
