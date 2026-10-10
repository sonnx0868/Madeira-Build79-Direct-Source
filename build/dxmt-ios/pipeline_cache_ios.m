/* SPDX-License-Identifier: MIT
 * Persistent Metal pipeline binaries, with an immutable lookup archive and
 * a separate serial writer. Rendering never waits for archive training/I/O.
 */
#import "pipeline_cache_ios.h"
#import <Foundation/Foundation.h>
#import <CommonCrypto/CommonDigest.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach/mach_time.h>

static const NSUInteger diskLimit = 64 * 1024 * 1024;
static const unsigned pendingLimit = 128, learningLimit = 2048;

@interface MadeiraPipelineCache : NSObject {
@public
    id<MTLDevice> owner;
    id<MTLBinaryArchive> lookup;
    id<MTLBinaryArchive> writer;
    dispatch_queue_t queue;
    NSURL *file;
    atomic_uint pending, learned, hits, misses;
    atomic_uint creations;
    atomic_uint_fast64_t creationTicks, longestCreation;
    BOOL dirty, flushScheduled;
}
@end
@implementation MadeiraPipelineCache
@end

static BOOL enabled(void) {
    const char *value = getenv("MADEIRA_PIPELINE_CACHE");
    return !value || strcmp(value, "0");
}

static NSURL *cacheFile(id<MTLDevice> device) {
    const char *override = getenv("MADEIRA_PIPELINE_CACHE_DIR");
    NSURL *base = override && *override ? [NSURL fileURLWithPath:[NSString stringWithUTF8String:override] isDirectory:YES]
        : [[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory inDomains:NSUserDomainMask].firstObject;
    if (!base) return nil;
    NSURL *folder = [base URLByAppendingPathComponent:@"madeira-metal-pipelines-v1" isDirectory:YES];
    if (![[NSFileManager defaultManager] createDirectoryAtURL:folder withIntermediateDirectories:YES attributes:nil error:nil]) return nil;
    // Metal also verifies shader/configuration identity. GPU/OS namespacing
    // avoids repeatedly probing files made for another driver generation.
    NSString *identity = [NSString stringWithFormat:@"pipeline-binary-v1|%@|%@", device.name, NSProcessInfo.processInfo.operatingSystemVersionString];
    NSData *data = [identity dataUsingEncoding:NSUTF8StringEncoding];
    unsigned char digest[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(data.bytes, (CC_LONG)data.length, digest);
    NSMutableString *name = [NSMutableString string];
    for (unsigned i = 0; i < sizeof(digest); i++) [name appendFormat:@"%02x", digest[i]];
    return [folder URLByAppendingPathComponent:[name stringByAppendingString:@".metallib"]];
}

static MadeiraPipelineCache *cacheForDevice(id<MTLDevice> device) {
    if (!enabled() || !device) return nil;
    static NSMutableDictionary<NSValue *, MadeiraPipelineCache *> *states;
    static NSLock *lock;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ states = [NSMutableDictionary dictionary]; lock = [NSLock new]; });
    NSValue *key = [NSValue valueWithPointer:(__bridge const void *)device];
    [lock lock];
    MadeiraPipelineCache *state = states[key];
    if (!state) {
        state = [MadeiraPipelineCache new];
        state->owner = device;
        atomic_init(&state->pending, 0); atomic_init(&state->learned, 0);
        atomic_init(&state->hits, 0); atomic_init(&state->misses, 0);
        atomic_init(&state->creations, 0); atomic_init(&state->creationTicks, 0); atomic_init(&state->longestCreation, 0);
        state->file = cacheFile(device);
        MTLBinaryArchiveDescriptor *desc = [MTLBinaryArchiveDescriptor new];
        NSDictionary *attrs = state->file ? [[NSFileManager defaultManager] attributesOfItemAtPath:state->file.path error:nil] : nil;
        if (attrs && [attrs[NSFileSize] unsignedLongLongValue] <= diskLimit) {
            desc.url = state->file;
            state->lookup = [device newBinaryArchiveWithDescriptor:desc error:nil];
        }
        // A distinct object is only mutated from the utility queue. Seed it
        // from the old file to retain pipelines learned in previous app runs.
        if (!state->lookup) desc.url = nil;
        state->writer = state->file ? [device newBinaryArchiveWithDescriptor:desc error:nil] : nil;
        if (!state->writer && desc.url) {
            desc.url = nil;
            state->writer = [device newBinaryArchiveWithDescriptor:desc error:nil];
        }
        dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_BACKGROUND, 0);
        state->queue = dispatch_queue_create("madeira.metal.pipeline-cache", attr);
        states[key] = state;
        NSLog(@"[pipeline-cache] pipeline-binary-v1 lookup=%d writer=%d pending-limit=%u", state->lookup != nil, state->writer != nil, pendingLimit);
    }
    [lock unlock];
    return state;
}

static void scheduleFlush(MadeiraPipelineCache *state) {
    if (state->flushScheduled) return;
    state->flushScheduled = YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC), state->queue, ^{
        @autoreleasepool {
            state->flushScheduled = NO;
            if (!state->dirty) return;
            NSURL *temporary = [NSURL fileURLWithPath:[state->file.path stringByAppendingString:@".tmp"]];
            NSError *error = nil;
            BOOL serialized = [state->writer serializeToURL:temporary error:&error];
            NSDictionary *attrs = serialized ? [[NSFileManager defaultManager] attributesOfItemAtPath:temporary.path error:nil] : nil;
            BOOL bounded = attrs && [attrs[NSFileSize] unsignedLongLongValue] <= diskLimit;
            BOOL saved = bounded && rename(temporary.fileSystemRepresentation, state->file.fileSystemRepresentation) == 0;
            if (!saved) [[NSFileManager defaultManager] removeItemAtURL:temporary error:nil];
            state->dirty = NO;
            NSLog(@"[pipeline-cache] saved=%d entries=%u bytes=%llu", saved,
                atomic_load_explicit(&state->learned, memory_order_relaxed), [attrs[NSFileSize] unsignedLongLongValue]);
        }
    });
}

static void noteLookup(MadeiraPipelineCache *state, BOOL hit) {
    unsigned count = atomic_fetch_add_explicit(hit ? &state->hits : &state->misses, 1, memory_order_relaxed) + 1;
    if (count == 1 || count % 64 == 0)
        NSLog(@"[pipeline-cache] hits=%u misses=%u", atomic_load_explicit(&state->hits, memory_order_relaxed),
              atomic_load_explicit(&state->misses, memory_order_relaxed));
}

static void noteCreation(MadeiraPipelineCache *state, uint64_t start) {
    if (!state) return;
    uint64_t elapsed = mach_absolute_time() - start;
    atomic_fetch_add_explicit(&state->creationTicks, elapsed, memory_order_relaxed);
    uint_fast64_t longest = atomic_load_explicit(&state->longestCreation, memory_order_relaxed);
    while (longest < elapsed && !atomic_compare_exchange_weak_explicit(&state->longestCreation, &longest, elapsed,
            memory_order_relaxed, memory_order_relaxed)) {}
    unsigned count = atomic_fetch_add_explicit(&state->creations, 1, memory_order_relaxed) + 1;
    if (count == 1 || count % 64 == 0) {
        mach_timebase_info_data_t tb;
        mach_timebase_info(&tb);
        double ms = (double)tb.numer / tb.denom / 1000000.0;
        NSLog(@"[pipeline-cache] native-creations=%u avg-ms=%.2f max-ms=%.2f", count,
            atomic_load_explicit(&state->creationTicks, memory_order_relaxed) * ms / count,
            atomic_load_explicit(&state->longestCreation, memory_order_relaxed) * ms);
    }
}

static void learn(MadeiraPipelineCache *state, id descriptor, BOOL compute) {
    if (!state->writer || atomic_load_explicit(&state->learned, memory_order_relaxed) >= learningLimit) return;
    unsigned previous = atomic_fetch_add_explicit(&state->pending, 1, memory_order_relaxed);
    if (previous >= pendingLimit) { atomic_fetch_sub_explicit(&state->pending, 1, memory_order_relaxed); return; }
    id copy = [descriptor copy];
    // Copy retains its shader functions until training finishes. Queue work
    // has no dependency back to the renderer/PE worker or a command buffer.
    dispatch_async(state->queue, ^{
        @autoreleasepool {
            if (atomic_load_explicit(&state->learned, memory_order_relaxed) < learningLimit) {
                NSError *error = nil;
                BOOL added = compute ? [state->writer addComputePipelineFunctionsWithDescriptor:copy error:&error]
                                     : [state->writer addRenderPipelineFunctionsWithDescriptor:copy error:&error];
                if (added) {
                    atomic_fetch_add_explicit(&state->learned, 1, memory_order_relaxed);
                    state->dirty = YES;
                    scheduleFlush(state);
                }
            }
            atomic_fetch_sub_explicit(&state->pending, 1, memory_order_relaxed);
        }
    });
}

id<MTLRenderPipelineState> madeira_new_render_pipeline(id<MTLDevice> device,
    MTLRenderPipelineDescriptor *descriptor, MTLPipelineOption options,
    MTLRenderPipelineReflection **reflection, NSError **error, BOOL cacheAllowed) {
    if (!cacheAllowed || descriptor.binaryArchives.count || (options & MTLPipelineOptionFailOnBinaryArchiveMiss))
        return [device newRenderPipelineStateWithDescriptor:descriptor options:options reflection:reflection error:error];
    MadeiraPipelineCache *state = cacheForDevice(device);
    if (state && state->lookup) {
        MTLRenderPipelineDescriptor *probe = [descriptor copy];
        probe.binaryArchives = @[state->lookup];
        id<MTLRenderPipelineState> cached = [device newRenderPipelineStateWithDescriptor:probe
            options:options | MTLPipelineOptionFailOnBinaryArchiveMiss reflection:reflection error:nil];
        noteLookup(state, cached != nil);
        if (cached) { if (error) *error = nil; return cached; }
    } else if (state) noteLookup(state, NO);
    uint64_t started = mach_absolute_time();
    id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:descriptor options:options reflection:reflection error:error];
    noteCreation(state, started);
    if (pipeline && state) learn(state, descriptor, NO);
    return pipeline;
}

id<MTLComputePipelineState> madeira_new_compute_pipeline(id<MTLDevice> device,
    MTLComputePipelineDescriptor *descriptor, MTLPipelineOption options,
    MTLComputePipelineReflection **reflection, NSError **error, BOOL cacheAllowed) {
    if (!cacheAllowed || descriptor.binaryArchives.count || (options & MTLPipelineOptionFailOnBinaryArchiveMiss))
        return [device newComputePipelineStateWithDescriptor:descriptor options:options reflection:reflection error:error];
    MadeiraPipelineCache *state = cacheForDevice(device);
    if (state && state->lookup) {
        MTLComputePipelineDescriptor *probe = [descriptor copy];
        probe.binaryArchives = @[state->lookup];
        id<MTLComputePipelineState> cached = [device newComputePipelineStateWithDescriptor:probe
            options:options | MTLPipelineOptionFailOnBinaryArchiveMiss reflection:reflection error:nil];
        noteLookup(state, cached != nil);
        if (cached) { if (error) *error = nil; return cached; }
    } else if (state) noteLookup(state, NO);
    uint64_t started = mach_absolute_time();
    id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithDescriptor:descriptor options:options reflection:reflection error:error];
    noteCreation(state, started);
    if (pipeline && state) learn(state, descriptor, YES);
    return pipeline;
}
