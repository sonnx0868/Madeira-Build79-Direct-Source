// Included once by GPUTraceCapture.m (ARC). Public Metal stage timestamp
// sampling for one follow-up frame, after the command capture has stopped.
// Inactive during ordinary gameplay; it neither changes shaders nor skips work.
#pragma once
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <objc/runtime.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <mach/mach_time.h>

// A restored older native bundle can still link the app. A refreshed boundary
// supplies the strong version=1 definition, as IOSDisplayShim does for pacing.
__attribute__((weak, noinline)) int madeira_gpu_stage_hooks_version(void) { return 0; }

enum { MGStageRender = 0, MGStageCompute = 1, MGStageBlit = 2,
       MGStageSamples = 1024, MGStageBuffers = 8 };

@interface MadeiraGPUStageScope : NSObject
@property(nonatomic, strong) NSURL *url;
@property(nonatomic, strong) id<MTLCommandQueue> queue;
@property(nonatomic, strong) NSMutableArray *results;
@property(nonatomic, strong) NSString *error;
@property(nonatomic) unsigned created, pending, completed;
@property(nonatomic) unsigned validIntervals;
@property(nonatomic) BOOL presented, finished, written;
@property(nonatomic) BOOL ready;
@property(nonatomic) uintptr_t boundaryBuffer;
@property(nonatomic) double requested;
@property(nonatomic) uint64_t cpuStart, gpuStart;
@end
@implementation MadeiraGPUStageScope
@end

@interface MadeiraGPUStageBuffer : NSObject
@property(nonatomic, weak) MadeiraGPUStageScope *scope;
@property(nonatomic, strong) id<MTLCounterSampleBuffer> samples;
@property(nonatomic, strong) NSMutableArray<NSMutableDictionary *> *passes;
@property(nonatomic, strong) NSMutableDictionary *prepared;
@property(nonatomic) NSUInteger used, index;
@property(nonatomic) BOOL presented, committed;
@end
@implementation MadeiraGPUStageBuffer
@end

static pthread_mutex_t stage_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int stage_enabled;
static atomic_int stage_callbacks;
static MadeiraGPUStageScope *stage_scope;
static char stage_buffer_key, stage_encoder_key;

// Caller holds stage_lock. Snapshot only; JSON/file I/O runs on a utility queue.
static void stage_finish_locked(MadeiraGPUStageScope *scope, NSString *error) {
    if (scope.finished) return;
    scope.finished = YES;
    if (error) scope.error = error;
    atomic_store_explicit(&stage_enabled, 0, memory_order_release);
    uint64_t cpuEnd = 0, gpuEnd = 0;
    id<MTLDevice> device = scope.queue.device;
    if ([device respondsToSelector:@selector(sampleTimestamps:gpuTimestamp:)])
        [device sampleTimestamps:&cpuEnd gpuTimestamp:&gpuEnd];
    mach_timebase_info_data_t timebase; mach_timebase_info(&timebase);
    NSDictionary *result = @{@"format": @"madeira-gpu-stages-v1",
        @"sampleRole": @"one complete following frame after a presentation boundary",
        @"complete": @(scope.presented && scope.completed == scope.created && scope.validIntervals > 0 && !scope.error),
        @"available": @(scope.validIntervals > 0), @"error": scope.error ?: @"",
        @"timestampFormat": @"MTLCounterResultTimestamp", @"validIntervals": @(scope.validIntervals),
        @"clockCalibration": @{@"cpuStart": @(scope.cpuStart), @"gpuStart": @(scope.gpuStart),
            @"cpuEnd": @(cpuEnd), @"gpuEnd": @(gpuEnd), @"cpuMachNumer": @(timebase.numer),
            @"cpuMachDenom": @(timebase.denom),
            @"available": @(cpuEnd > scope.cpuStart && gpuEnd > scope.gpuStart && scope.gpuStart > 0)},
        @"requestedUptime": @(scope.requested), @"buffersCreated": @(scope.created),
        @"buffersCompleted": @(scope.completed), @"buffers": [scope.results copy],
        @"note": @"Sampling perturbs performance. Stage intervals may overlap; do not sum them as frame time."};
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_UTILITY, 0), ^{
        NSError *failure = nil;
        NSData *data = [NSJSONSerialization dataWithJSONObject:result options:NSJSONWritingPrettyPrinted error:&failure];
        if (data) [data writeToURL:scope.url options:NSDataWritingAtomic error:&failure];
        pthread_mutex_lock(&stage_lock);
        scope.written = YES;
        scope.queue = nil;
        if (stage_scope == scope) atomic_store_explicit(&stage_callbacks, 0, memory_order_release);
        if (failure) scope.error = failure.localizedDescription;
        pthread_mutex_unlock(&stage_lock);
        fprintf(stderr, "[gpu-stages-v1] buffers=%u complete=%d file=%s error=%s\n",
            scope.completed, [result[@"complete"] boolValue] && !failure,
            scope.url.lastPathComponent.UTF8String, scope.error.UTF8String ?: "");
    });
}

static void stage_request(NSURL *url, id<MTLCommandQueue> queue) {
    if (madeira_gpu_stage_hooks_version() != 1) return;
    pthread_mutex_lock(&stage_lock);
    if (stage_scope && !stage_scope.written) {
        pthread_mutex_unlock(&stage_lock); return;
    }
    MadeiraGPUStageScope *scope = [MadeiraGPUStageScope new];
    scope.url = url; scope.queue = queue; scope.results = [NSMutableArray new];
    scope.requested = NSProcessInfo.processInfo.systemUptime;
    stage_scope = scope;
    atomic_store_explicit(&stage_callbacks, 1, memory_order_release);
    id<MTLDevice> device = queue.device;
    if ([device respondsToSelector:@selector(sampleTimestamps:gpuTimestamp:)]) {
        uint64_t cpuStart = 0, gpuStart = 0;
        [device sampleTimestamps:&cpuStart gpuTimestamp:&gpuStart];
        scope.cpuStart = cpuStart; scope.gpuStart = gpuStart;
    }
    if (![device supportsCounterSampling:MTLCounterSamplingPointAtStageBoundary])
        stage_finish_locked(scope, @"This Metal device does not support stage-boundary counters.");
    else atomic_store_explicit(&stage_enabled, 1, memory_order_release);
    pthread_mutex_unlock(&stage_lock);
}

static BOOL stage_waiting(void) {
    pthread_mutex_lock(&stage_lock);
    if (stage_scope && !stage_scope.finished &&
        NSProcessInfo.processInfo.systemUptime - stage_scope.requested > 15)
        stage_finish_locked(stage_scope, @"Stage sampling timed out; no complete frame was obtained.");
    BOOL waiting = stage_scope && !stage_scope.written;
    pthread_mutex_unlock(&stage_lock); return waiting;
}

static void stage_cancel(void) {
    pthread_mutex_lock(&stage_lock);
    if (stage_scope && !stage_scope.finished) stage_finish_locked(stage_scope, @"Stage sampling cancelled.");
    pthread_mutex_unlock(&stage_lock);
}

int madeira_gpu_stage_enabled(void) {
    return atomic_load_explicit(&stage_enabled, memory_order_acquire);
}

static void stage_buffer_created(void *rawBuffer) {
    if (!madeira_gpu_stage_enabled() || !rawBuffer) return;
    id<MTLCommandBuffer> buffer = (__bridge id)rawBuffer;
    pthread_mutex_lock(&stage_lock);
    MadeiraGPUStageScope *scope = stage_scope;
    if (!scope || !scope.ready || scope.finished || scope.presented || buffer.commandQueue != scope.queue) {
        pthread_mutex_unlock(&stage_lock); return;
    }
    if (scope.created >= MGStageBuffers) {
        stage_finish_locked(scope, @"Stage sampling reached its command-buffer limit.");
        pthread_mutex_unlock(&stage_lock); return;
    }
    id<MTLCounterSet> timestamps = nil;
    for (id<MTLCounterSet> set in buffer.device.counterSets)
        if ([set.name isEqualToString:MTLCommonCounterSetTimestamp]) { timestamps = set; break; }
    MTLCounterSampleBufferDescriptor *descriptor = [MTLCounterSampleBufferDescriptor new];
    descriptor.counterSet = timestamps; descriptor.sampleCount = MGStageSamples;
    descriptor.storageMode = MTLStorageModeShared;
    NSError *error = nil;
    id<MTLCounterSampleBuffer> samples = timestamps ? [buffer.device newCounterSampleBufferWithDescriptor:descriptor error:&error] : nil;
    if (!samples) {
        stage_finish_locked(scope, error.localizedDescription ?: @"Timestamp counter set is unavailable.");
        pthread_mutex_unlock(&stage_lock); return;
    }
    MadeiraGPUStageBuffer *record = [MadeiraGPUStageBuffer new];
    record.scope = scope; record.samples = samples; record.passes = [NSMutableArray new];
    record.index = scope.created++;
    objc_setAssociatedObject(buffer, &stage_buffer_key, record, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    pthread_mutex_unlock(&stage_lock);
}

int madeira_gpu_stage_prepare(void *rawBuffer, void *rawDescriptor, unsigned kind) {
    if (!madeira_gpu_stage_enabled()) return 0;
    id<MTLCommandBuffer> buffer = (__bridge id)rawBuffer;
    pthread_mutex_lock(&stage_lock);
    MadeiraGPUStageBuffer *record = objc_getAssociatedObject(buffer, &stage_buffer_key);
    unsigned count = kind == MGStageRender ? 4 : 2;
    if (!record || record.scope != stage_scope || record.scope.finished || record.committed) {
        pthread_mutex_unlock(&stage_lock); return 0;
    }
    if (record.used + count > MGStageSamples) {
        stage_finish_locked(record.scope, @"Stage sampling reached its sample-buffer limit.");
        pthread_mutex_unlock(&stage_lock); return 0;
    }
    NSUInteger first = record.used; record.used += count;
    record.prepared = [@{@"pass": @(record.passes.count), @"kind": @(kind),
        @"firstSample": @(first), @"sampleCount": @(count), @"pipelines": [NSMutableArray new]} mutableCopy];
    [record.passes addObject:record.prepared];
    if (kind == MGStageRender) {
        MTLRenderPassDescriptor *descriptor = (__bridge id)rawDescriptor;
        MTLRenderPassSampleBufferAttachmentDescriptor *attachment = descriptor.sampleBufferAttachments[0];
        attachment.sampleBuffer = record.samples;
        attachment.startOfVertexSampleIndex = first; attachment.endOfVertexSampleIndex = first + 1;
        attachment.startOfFragmentSampleIndex = first + 2; attachment.endOfFragmentSampleIndex = first + 3;
    } else if (kind == MGStageCompute) {
        MTLComputePassDescriptor *descriptor = (__bridge id)rawDescriptor;
        MTLComputePassSampleBufferAttachmentDescriptor *attachment = descriptor.sampleBufferAttachments[0];
        attachment.sampleBuffer = record.samples;
        attachment.startOfEncoderSampleIndex = first; attachment.endOfEncoderSampleIndex = first + 1;
    } else {
        MTLBlitPassDescriptor *descriptor = (__bridge id)rawDescriptor;
        MTLBlitPassSampleBufferAttachmentDescriptor *attachment = descriptor.sampleBufferAttachments[0];
        attachment.sampleBuffer = record.samples;
        attachment.startOfEncoderSampleIndex = first; attachment.endOfEncoderSampleIndex = first + 1;
    }
    pthread_mutex_unlock(&stage_lock); return 1;
}

void madeira_gpu_stage_encoder_created(void *rawBuffer, void *rawEncoder) {
    if (!rawBuffer) return;
    pthread_mutex_lock(&stage_lock);
    MadeiraGPUStageBuffer *record = objc_getAssociatedObject((__bridge id)rawBuffer, &stage_buffer_key);
    if (record.prepared) {
        if (rawEncoder) {
            objc_setAssociatedObject((__bridge id)rawEncoder, &stage_encoder_key, record.prepared,
                                     OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        } else {
            record.prepared[@"encoderCreationFailed"] = @YES;
            record.scope.error = @"A sampled encoder was refused; gameplay used the normal encoder.";
        }
        record.prepared = nil;
    }
    pthread_mutex_unlock(&stage_lock);
}

void madeira_gpu_stage_pipeline(void *rawEncoder, void *rawPipeline) {
    if (!madeira_gpu_stage_enabled() || !rawEncoder || !rawPipeline) return;
    pthread_mutex_lock(&stage_lock);
    NSMutableDictionary *pass = objc_getAssociatedObject((__bridge id)rawEncoder, &stage_encoder_key);
    NSMutableArray *pipelines = pass[@"pipelines"];
    NSString *identity = [NSString stringWithFormat:@"0x%llx", (unsigned long long)(uintptr_t)rawPipeline];
    if (pipelines.count < 32 && ![pipelines containsObject:identity]) [pipelines addObject:identity];
    pthread_mutex_unlock(&stage_lock);
}

static void stage_present(void *rawBuffer) {
    if (!madeira_gpu_stage_enabled() || !rawBuffer) return;
    pthread_mutex_lock(&stage_lock);
    MadeiraGPUStageBuffer *record = objc_getAssociatedObject((__bridge id)rawBuffer, &stage_buffer_key);
    if (record.scope == stage_scope) record.presented = YES;
    else if (stage_scope && !stage_scope.ready &&
             ((__bridge id<MTLCommandBuffer>)rawBuffer).commandQueue == stage_scope.queue)
        stage_scope.boundaryBuffer = (uintptr_t)rawBuffer;
    pthread_mutex_unlock(&stage_lock);
}

static NSDictionary *stage_interval(const MTLCounterResultTimestamp *values, NSUInteger first) {
    uint64_t begin = values[first].timestamp, end = values[first + 1].timestamp;
    BOOL valid = begin != MTLCounterErrorValue && end != MTLCounterErrorValue && begin && end >= begin;
    return valid ? @{@"available": @YES, @"start": @(begin), @"end": @(end), @"duration": @(end - begin)}
                 : @{@"available": @NO};
}

// This hook must run BEFORE Metal's commit: completed handlers cannot be
// installed after commit. No waits or GPU synchronization commands are added.
void madeira_gpu_stage_will_commit(void *rawBuffer) {
    if (!rawBuffer || !atomic_load_explicit(&stage_callbacks, memory_order_acquire)) return;
    id<MTLCommandBuffer> buffer = (__bridge id)rawBuffer;
    MadeiraGPUStageBuffer *record = objc_getAssociatedObject(buffer, &stage_buffer_key);
    pthread_mutex_lock(&stage_lock);
    if (!record) {
        if (stage_scope && !stage_scope.finished && stage_scope.boundaryBuffer == (uintptr_t)rawBuffer)
            stage_scope.ready = YES;
        pthread_mutex_unlock(&stage_lock); return;
    }
    MadeiraGPUStageScope *scope = record.scope;
    if (!scope || scope.finished || record.committed) { pthread_mutex_unlock(&stage_lock); return; }
    record.committed = YES; scope.pending++;
    if (record.presented) { scope.presented = YES; atomic_store_explicit(&stage_enabled, 0, memory_order_release); }
    [buffer addCompletedHandler:^(id<MTLCommandBuffer> completed) {
        NSData *resolved = record.used ? [record.samples resolveCounterRange:NSMakeRange(0, record.used)] : nil;
        BOOL valid = resolved && resolved.length >= record.used * sizeof(MTLCounterResultTimestamp);
        NSMutableArray *passes = [NSMutableArray new];
        unsigned validIntervals = 0;
        for (NSDictionary *pass in record.passes) {
            NSMutableDictionary *output = [pass mutableCopy];
            NSUInteger first = [pass[@"firstSample"] unsignedIntegerValue];
            if (valid && ![pass[@"encoderCreationFailed"] boolValue]) {
                const MTLCounterResultTimestamp *values = resolved.bytes;
                if ([pass[@"kind"] unsignedIntValue] == MGStageRender) {
                    output[@"vertex"] = stage_interval(values, first);
                    output[@"fragment"] = stage_interval(values, first + 2);
                    validIntervals += [output[@"vertex"][@"available"] boolValue] +
                                      [output[@"fragment"][@"available"] boolValue];
                } else {
                    output[@"encoder"] = stage_interval(values, first);
                    validIntervals += [output[@"encoder"][@"available"] boolValue];
                }
            } else output[@"available"] = @NO;
            [passes addObject:output];
        }
        NSDictionary *result = @{@"buffer": @(record.index), @"presented": @(record.presented),
            @"samples": @(record.used), @"resolved": @(valid), @"passes": passes,
            @"gpuStartSeconds": @(completed.GPUStartTime), @"gpuEndSeconds": @(completed.GPUEndTime),
            @"commandBufferError": completed.error.localizedDescription ?: @""};
        pthread_mutex_lock(&stage_lock);
        if (!scope.finished) {
            [scope.results addObject:result]; scope.completed++; scope.pending--;
            scope.validIntervals += validIntervals;
            if (record.used && !valid) scope.error = @"Timestamp samples could not be resolved.";
            if (completed.error) scope.error = completed.error.localizedDescription;
            if (scope.presented && !scope.pending && scope.completed == scope.created) stage_finish_locked(scope, nil);
        }
        objc_setAssociatedObject(completed, &stage_buffer_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        pthread_mutex_unlock(&stage_lock);
    }];
    pthread_mutex_unlock(&stage_lock);
}
