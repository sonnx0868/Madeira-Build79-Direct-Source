/* SPDX-License-Identifier: MIT
 * Persistent Metal pipeline binaries, with an immutable lookup archive and
 * a separate serial writer. Rendering never waits for archive training/I/O.
 */
#import "pipeline_cache_ios.h"
#import "pipeline_warmup_ios.h"
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
static const unsigned residentLimit = 128, archiveLimit = 4, diskArchiveLimit = 8;

@interface MadeiraPipelineBuild : NSObject
@property(strong) id descriptor; // retains the function objects used by the key
@property(strong) id pipeline;
@property(strong) NSError *error;
@property(strong) NSException *exception;
@property(strong) NSThread *owner;
@property BOOL done;
@property uint64_t used;
@end
@implementation MadeiraPipelineBuild
@end

@interface MadeiraPipelineCache : NSObject {
@public
    id<MTLDevice> owner;
    id<MTLBinaryArchive> lookup;
    id<MTLBinaryArchive> writer;
    NSArray<id<MTLBinaryArchive>> *lookups;
    NSCondition *gate;
    NSMutableDictionary<NSString *, MadeiraPipelineBuild *> *resident, *building;
    uint64_t clock;
    unsigned residentHits, sharedHits;
    dispatch_queue_t queue;
    NSURL *file, *identityFile;
    atomic_uint pending, learned, hits, misses;
    atomic_uint creations;
    atomic_uint_fast64_t creationTicks, longestCreation;
    BOOL dirty, flushScheduled;
    MadeiraPipelineWarmup *warmup;
}
@end
@implementation MadeiraPipelineCache
@end

static BOOL enabled(void) {
    const char *value = getenv("MADEIRA_PIPELINE_CACHE");
    return !value || strcmp(value, "0");
}

static NSURL *cacheFile(id<MTLDevice> device, BOOL legacy) {
    const char *override = legacy ? NULL : getenv("MADEIRA_PIPELINE_CACHE_DIR");
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

static NSMutableDictionary<NSString *, MadeiraPipelineCache *> *states;
static NSLock *lock;
static NSLock *diskLock;

// Each live device writer owns a file. Separate devices/queues must not race
// on one .tmp or overwrite each other's newly learned pipelines. Old v1 files
// are still read; only our fingerprinted shards participate in eviction.
static NSArray<NSURL *> *archiveFiles(NSURL *identity) {
    if (!identity) return @[];
    NSString *stem = identity.lastPathComponent.stringByDeletingPathExtension;
    NSMutableArray<NSURL *> *files = [NSMutableArray array];
    for (NSURL *url in [NSFileManager.defaultManager contentsOfDirectoryAtURL:identity.URLByDeletingLastPathComponent
             includingPropertiesForKeys:@[NSURLContentModificationDateKey, NSURLFileSizeKey, NSURLIsSymbolicLinkKey, NSURLIsRegularFileKey]
             options:NSDirectoryEnumerationSkipsHiddenFiles error:nil]) {
        NSString *name = url.lastPathComponent;
        BOOL shard = ![name isEqual:identity.lastPathComponent] &&
            name.length == stem.length+1+36+9 && [name hasPrefix:[stem stringByAppendingString:@"."]] && [name.pathExtension isEqual:@"metallib"];
        if (![name isEqual:identity.lastPathComponent] && !shard) continue;
        if (shard) {
            NSString *part = [name substringWithRange:NSMakeRange(stem.length+1, name.length-stem.length-1-9)];
            if (![[NSUUID alloc] initWithUUIDString:part]) continue;
        }
        NSDictionary *a = [url resourceValuesForKeys:@[NSURLContentModificationDateKey, NSURLFileSizeKey, NSURLIsSymbolicLinkKey, NSURLIsRegularFileKey] error:nil];
        if ([a[NSURLIsSymbolicLinkKey] boolValue] || ![a[NSURLIsRegularFileKey] boolValue] || !a[NSURLFileSizeKey]) continue;
        [files addObject:url];
    }
    [files sortUsingComparator:^NSComparisonResult(NSURL *a, NSURL *b) {
        NSDate *aa = nil, *bb = nil;
        [a getResourceValue:&aa forKey:NSURLContentModificationDateKey error:nil];
        [b getResourceValue:&bb forKey:NSURLContentModificationDateKey error:nil];
        return [(bb ?: NSDate.distantPast) compare:(aa ?: NSDate.distantPast)];
    }];
    return files;
}

static NSArray<id<MTLBinaryArchive>> *readArchives(id<MTLDevice> device, NSArray<NSURL *> *files, NSURL **firstAccepted) {
    NSMutableArray *archives = [NSMutableArray array];
    NSUInteger bytes = 0;
    for (NSURL *url in files) {
        if (archives.count == archiveLimit) break;
        NSDictionary *a = [NSFileManager.defaultManager attributesOfItemAtPath:url.path error:nil];
        NSUInteger size = [a[NSFileSize] unsignedIntegerValue];
        if (!size || size > diskLimit-bytes) continue;
        MTLBinaryArchiveDescriptor *d = [MTLBinaryArchiveDescriptor new]; d.url = url;
        id<MTLBinaryArchive> archive = [device newBinaryArchiveWithDescriptor:d error:nil];
        if (archive) { if (firstAccepted && !archives.count) *firstAccepted = url; [archives addObject:archive]; bytes += size; }
    }
    return archives;
}

static NSArray<id<MTLBinaryArchive>> *lookupSnapshot(MadeiraPipelineCache *state) {
    [state->gate lock]; NSArray *snapshot = state->lookups; [state->gate unlock]; return snapshot;
}

static void pruneArchives(MadeiraPipelineCache *state) {
    NSUInteger kept = 0, bytes = 0;
    NSMutableArray *files = [archiveFiles(state->identityFile) mutableCopy];
    // Count the current writer first, including under identical mtimes. It is
    // never evicted underneath this state and still fits the total disk bound.
    if ([files containsObject:state->file]) { [files removeObject:state->file]; [files insertObject:state->file atIndex:0]; }
    for (NSURL *url in files) {
        NSUInteger size = [[[NSFileManager.defaultManager attributesOfItemAtPath:url.path error:nil] objectForKey:NSFileSize] unsignedIntegerValue];
        if (kept < diskArchiveLimit && size <= diskLimit-bytes) { ++kept; bytes += size; continue; }
        if (![url isEqual:state->file]) [NSFileManager.defaultManager removeItemAtURL:url error:nil];
    }
}
static void initializeRegistry(void) {
    static dispatch_once_t once;
    dispatch_once(&once, ^{ states = [NSMutableDictionary dictionary]; lock = [NSLock new]; diskLock = [NSLock new]; });
}
static MadeiraPipelineCache *cacheForDevice(id<MTLDevice> device) {
    if (!enabled() || !device) return nil;
    initializeRegistry();
    const char *directory = getenv("MADEIRA_PIPELINE_CACHE_DIR");
    NSString *key = [NSString stringWithFormat:@"%p|%s", (__bridge const void *)device, directory ?: "default"];
    [lock lock];
    MadeiraPipelineCache *state = states[key];
    if (!state) {
        state = [MadeiraPipelineCache new];
        state->owner = device;
        state->gate = [NSCondition new];
        state->resident = [NSMutableDictionary dictionary]; state->building = [NSMutableDictionary dictionary];
        atomic_init(&state->pending, 0); atomic_init(&state->learned, 0);
        atomic_init(&state->hits, 0); atomic_init(&state->misses, 0);
        atomic_init(&state->creations, 0); atomic_init(&state->creationTicks, 0); atomic_init(&state->longestCreation, 0);
        state->identityFile = cacheFile(device, NO);
        NSURL *identity = state->identityFile;
        state->file = identity ? [[identity URLByDeletingPathExtension]
            URLByAppendingPathExtension:[NSString stringWithFormat:@"%@.metallib", NSUUID.UUID.UUIDString]] : nil;
        MTLBinaryArchiveDescriptor *desc = [MTLBinaryArchiveDescriptor new];
        NSArray *files = archiveFiles(identity);
        BOOL legacySeed = NO;
        if (!files.count && directory && *directory) {
            // Preserve the existing shared binary archive on the first move
            // to a game's scope. Metal verifies each requested pipeline.
            NSURL *previous = cacheFile(device, YES);
            files = archiveFiles(previous); legacySeed = files.count != 0;
        }
        NSURL *seed = nil;
        state->lookups = readArchives(device, files, &seed);
        state->lookup = state->lookups.firstObject;
        if (state->lookup) desc.url = seed;
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
        NSURL *warmFolder = identity ? [[identity URLByDeletingPathExtension] URLByAppendingPathExtension:@"warmup"] : nil;
        state->warmup = [[MadeiraPipelineWarmup alloc] initWithDevice:device folder:warmFolder archive:state->lookup];
        if (states.count >= 8) [states removeObjectForKey:states.allKeys.firstObject];
        states[key] = state;
        NSLog(@"[pipeline-cache] pipeline-binary-v1 lookup=%d writer=%d pending-limit=%u legacy-seed=%d", state->lookup != nil, state->writer != nil, pendingLimit, legacySeed);
        NSLog(@"[pipeline-cache] live-pipeline-v2 resident-limit=%u archives=%lu writer-isolated=1", residentLimit, (unsigned long)state->lookups.count);
    }
    [lock unlock];
    return state;
}

void madeira_prepare_pipeline_cache(id<MTLDevice> device) {
    const char *mode = getenv("MADEIRA_SHADER_WARMUP");
    if (mode && !strcmp(mode, "1")) (void)cacheForDevice(device);
}
MadeiraPipelineWarmup *madeira_pipeline_warmup_for_device(id<MTLDevice> device) {
    const char *mode = getenv("MADEIRA_SHADER_WARMUP");
    if (!mode || strcmp(mode, "1")) return nil;
    MadeiraPipelineCache *state = cacheForDevice(device);
    return state ? state->warmup : nil;
}

static void flushCache(MadeiraPipelineCache *state) {
    if (!state->dirty) return;
    NSURL *temporary = [NSURL fileURLWithPath:[state->file.path stringByAppendingFormat:@".%@.tmp", NSUUID.UUID.UUIDString]];
    NSError *error = nil;
    BOOL serialized = [state->writer serializeToURL:temporary error:&error];
    NSDictionary *attrs = serialized ? [[NSFileManager defaultManager] attributesOfItemAtPath:temporary.path error:nil] : nil;
    BOOL bounded = attrs && [attrs[NSFileSize] unsignedLongLongValue] <= diskLimit;
    [diskLock lock];
    BOOL saved = bounded && rename(temporary.fileSystemRepresentation, state->file.fileSystemRepresentation) == 0;
    if (!saved) [[NSFileManager defaultManager] removeItemAtURL:temporary error:nil];
    state->dirty = !saved;
    if (saved) {
        pruneArchives(state);
        NSArray *archives = readArchives(state->owner, archiveFiles(state->identityFile), NULL);
        if (archives.count) { [state->gate lock]; state->lookups = archives; state->lookup = archives.firstObject; [state->gate unlock]; }
    }
    [diskLock unlock];
    NSLog(@"[pipeline-cache] saved=%d entries=%u bytes=%llu", saved,
        atomic_load_explicit(&state->learned, memory_order_relaxed), [attrs[NSFileSize] unsignedLongLongValue]);
    if (!saved && error) NSLog(@"[pipeline-cache] serialization error domain=%@ code=%ld", error.domain, (long)error.code);
}

// Only identical, fully described ordinary pipelines share a build. No Metal
// call, archive I/O or warmup work runs while this condition is locked.
static BOOL beginBuild(MadeiraPipelineCache *state, NSString *key, id descriptor,
                       MadeiraPipelineBuild **ticket, id *result, NSError **error) {
    if (!state || !key) return NO;
    [state->gate lock];
    MadeiraPipelineBuild *entry = state->resident[key];
    if (entry) {
        entry.used = ++state->clock; ++state->residentHits;
    } else if ((entry = state->building[key])) {
        if (entry.owner == NSThread.currentThread) { [state->gate unlock]; return NO; }
        while (!entry.done) [state->gate wait];
        ++state->sharedHits;
    } else {
        if (state->building.count < pendingLimit) {
            entry = [MadeiraPipelineBuild new]; entry.owner = NSThread.currentThread;
            entry.descriptor = [descriptor copy]; state->building[key] = entry; *ticket = entry;
        }
        [state->gate unlock]; return NO;
    }
    *result = entry.pipeline; if (error) *error = entry.error;
    NSException *exception = entry.exception;
    unsigned hits = state->residentHits + state->sharedHits;
    if (hits == 1 || !(hits % 64)) NSLog(@"[pipeline-cache] live-pipeline-v2 reused=%u shared-builds=%u", state->residentHits, state->sharedHits);
    [state->gate unlock]; if (exception) @throw exception; return YES;
}

static void finishBuild(MadeiraPipelineCache *state, NSString *key, MadeiraPipelineBuild *ticket, id pipeline, NSError *error) {
    if (!ticket) return;
    [state->gate lock];
    ticket.pipeline = pipeline; ticket.error = error; ticket.done = YES; ticket.owner = nil;
    [state->building removeObjectForKey:key];
    if (pipeline) {
        if (state->resident.count >= residentLimit) {
            NSString *oldest = nil; uint64_t used = UINT64_MAX;
            for (NSString *candidate in state->resident) {
                if (state->resident[candidate].used < used) { oldest = candidate; used = state->resident[candidate].used; }
            }
            if (oldest) [state->resident removeObjectForKey:oldest];
        }
        ticket.used = ++state->clock; state->resident[key] = ticket;
    }
    [state->gate broadcast]; [state->gate unlock];
}

static void abortBuild(MadeiraPipelineCache *state, NSString *key, MadeiraPipelineBuild *ticket, NSException *exception) {
    if (!ticket) return;
    [state->gate lock];
    if (!ticket.done) {
        ticket.exception = exception; ticket.done = YES; ticket.owner = nil;
        [state->building removeObjectForKey:key]; [state->gate broadcast];
    }
    [state->gate unlock];
}

void madeira_flush_pipeline_cache(void) {
    initializeRegistry();
    [lock lock]; NSArray *snapshot = states.allValues; [lock unlock];
    // Queue behind learned descriptors, preserving ownership and ordering.
    // Neither the main thread nor the renderer waits for serialization.
    for (MadeiraPipelineCache *state in snapshot) {
        dispatch_async(state->queue, ^{ @autoreleasepool { flushCache(state); } });
    }
    if (snapshot.count) NSLog(@"[pipeline-cache] pipeline-flush-v1 scopes=%lu", (unsigned long)snapshot.count);
}

static void scheduleFlush(MadeiraPipelineCache *state) {
    if (state->flushScheduled) return;
    state->flushScheduled = YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC), state->queue, ^{
        @autoreleasepool {
            state->flushScheduled = NO;
            flushCache(state);
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
    NSString *key = state && !options && !reflection ? madeira_pipeline_runtime_key(descriptor, NO) : nil;
    MadeiraPipelineBuild *ticket = nil;
    id result = nil;
    if (beginBuild(state, key, descriptor, &ticket, &result, error)) return result;
    @try {
    if (state && !options && !reflection) {
        id<MTLRenderPipelineState> warmed = [state->warmup take:descriptor compute:NO];
        if (warmed) { finishBuild(state, key, ticket, warmed, nil); if (error) *error = nil; [state->warmup record:descriptor compute:NO]; return warmed; }
    }
    NSArray *archives = state ? lookupSnapshot(state) : nil;
    if (archives.count) {
        MTLRenderPipelineDescriptor *probe = [descriptor copy];
        probe.binaryArchives = archives;
        id<MTLRenderPipelineState> cached = [device newRenderPipelineStateWithDescriptor:probe
            options:options | MTLPipelineOptionFailOnBinaryArchiveMiss reflection:reflection error:nil];
        noteLookup(state, cached != nil);
        if (cached) { finishBuild(state, key, ticket, cached, nil); if (error) *error = nil; if (!options && !reflection) [state->warmup record:descriptor compute:NO]; return cached; }
    } else if (state) noteLookup(state, NO);
    uint64_t started = mach_absolute_time();
    NSError *creationError = nil;
    id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:descriptor options:options reflection:reflection error:&creationError];
    if (error) *error = creationError;
    finishBuild(state, key, ticket, pipeline, creationError);
    noteCreation(state, started);
    if (pipeline && state) {
        learn(state, descriptor, NO);
        if (!options && !reflection) [state->warmup record:descriptor compute:NO];
    }
    return pipeline;
    } @catch (NSException *exception) {
        abortBuild(state, key, ticket, exception);
        @throw;
    }
}

id<MTLComputePipelineState> madeira_new_compute_pipeline(id<MTLDevice> device,
    MTLComputePipelineDescriptor *descriptor, MTLPipelineOption options,
    MTLComputePipelineReflection **reflection, NSError **error, BOOL cacheAllowed) {
    if (!cacheAllowed || descriptor.binaryArchives.count || (options & MTLPipelineOptionFailOnBinaryArchiveMiss))
        return [device newComputePipelineStateWithDescriptor:descriptor options:options reflection:reflection error:error];
    MadeiraPipelineCache *state = cacheForDevice(device);
    NSString *key = state && !options && !reflection ? madeira_pipeline_runtime_key(descriptor, YES) : nil;
    MadeiraPipelineBuild *ticket = nil;
    id result = nil;
    if (beginBuild(state, key, descriptor, &ticket, &result, error)) return result;
    @try {
    if (state && !options && !reflection) {
        id<MTLComputePipelineState> warmed = [state->warmup take:descriptor compute:YES];
        if (warmed) { finishBuild(state, key, ticket, warmed, nil); if (error) *error = nil; [state->warmup record:descriptor compute:YES]; return warmed; }
    }
    NSArray *archives = state ? lookupSnapshot(state) : nil;
    if (archives.count) {
        MTLComputePipelineDescriptor *probe = [descriptor copy];
        probe.binaryArchives = archives;
        id<MTLComputePipelineState> cached = [device newComputePipelineStateWithDescriptor:probe
            options:options | MTLPipelineOptionFailOnBinaryArchiveMiss reflection:reflection error:nil];
        noteLookup(state, cached != nil);
        if (cached) { finishBuild(state, key, ticket, cached, nil); if (error) *error = nil; if (!options && !reflection) [state->warmup record:descriptor compute:YES]; return cached; }
    } else if (state) noteLookup(state, NO);
    uint64_t started = mach_absolute_time();
    NSError *creationError = nil;
    id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithDescriptor:descriptor options:options reflection:reflection error:&creationError];
    if (error) *error = creationError;
    finishBuild(state, key, ticket, pipeline, creationError);
    noteCreation(state, started);
    if (pipeline && state) {
        learn(state, descriptor, YES);
        if (!options && !reflection) [state->warmup record:descriptor compute:YES];
    }
    return pipeline;
    } @catch (NSException *exception) {
        abortBuild(state, key, ticket, exception);
        @throw;
    }
}
