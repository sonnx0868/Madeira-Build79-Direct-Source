#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <pthread.h>
#include <stdatomic.h>
#include "GPUTraceCapture.h"
#include "GPUTracePolicy.h"
#include "GPUStageProfile.h"

static pthread_mutex_t trace_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int trace_enabled;
static MadeiraGPUTracePolicy trace_policy;
static uint64_t trace_generation;
static BOOL trace_owned;
static NSURL *trace_url;
static NSString *trace_error = @"";
static NSMutableSet *trace_created, *trace_presented;
static id<MTLCommandQueue> trace_queue;

static MTLCaptureManager *trace_manager(void) {
#ifdef MADEIRA_GPU_TRACE_TEST
    extern MTLCaptureManager *madeira_test_capture_manager(void);
    return madeira_test_capture_manager();
#else
    return MTLCaptureManager.sharedCaptureManager;
#endif
}
static void trace_finish(NSString *error) {
    // Never preempt an Xcode/third-party capture we do not own.
    if (trace_owned) { [trace_manager() stopCapture]; trace_owned = NO; }
    if (!error && trace_policy.phase == MGTraceComplete && trace_queue) {
        NSString *name = [trace_url.URLByDeletingPathExtension.path stringByAppendingString:@"-stages.json"];
        stage_request([NSURL fileURLWithPath:name], trace_queue);
    }
    if (error) { trace_error = [error copy]; trace_policy.phase = MGTraceFailed; }
    atomic_store_explicit(&trace_enabled, 0, memory_order_release);
    [trace_created removeAllObjects]; [trace_presented removeAllObjects]; trace_queue = nil;
    fprintf(stderr, "[gpu-trace-v1] finished phase=%d buffers=%u frames=%u file=%s error=%s\n",
        trace_policy.phase, trace_policy.buffers, trace_policy.frames,
        trace_url.lastPathComponent.UTF8String, trace_error.UTF8String);
}
int madeira_gpu_trace_request(const char *path) {
    if (!path) return 0;
    pthread_mutex_lock(&trace_lock);
    if (atomic_load_explicit(&trace_enabled, memory_order_relaxed)) {
        pthread_mutex_unlock(&trace_lock); return 0;
    }
    NSString *name = [NSString stringWithUTF8String:path];
    NSURL *url = name ? [NSURL fileURLWithPath:name] : nil;
    if (!name.isAbsolutePath || ![url.pathExtension isEqualToString:@"gputrace"] ||
        [NSFileManager.defaultManager fileExistsAtPath:name]) {
        trace_error = @"The GPU trace needs a new .gputrace file.";
        trace_policy.phase = MGTraceFailed; pthread_mutex_unlock(&trace_lock); return 0;
    }
    trace_url = url; trace_error = @""; trace_generation++;
    trace_created = [NSMutableSet new]; trace_presented = [NSMutableSet new];
    trace_queue = nil; trace_owned = NO;
    madeira_trace_arm(&trace_policy, NSProcessInfo.processInfo.systemUptime);
    atomic_store_explicit(&trace_enabled, 1, memory_order_release);
    fprintf(stderr, "[gpu-trace-v1] requested file=%s boundary=next-present frames=2\n", url.lastPathComponent.UTF8String);
    pthread_mutex_unlock(&trace_lock); return 1;
}
uint64_t madeira_gpu_trace_will_create(void *rawQueue) {
    if (!atomic_load_explicit(&trace_enabled, memory_order_acquire)) return 0;
    pthread_mutex_lock(&trace_lock);
    id<MTLCommandQueue> queue = (__bridge id)rawQueue;
    if (madeira_trace_startable(&trace_policy, (uintptr_t)rawQueue)) {
        MTLCaptureManager *manager = trace_manager();
        if (manager.isCapturing) trace_finish(@"Another Metal capture is already running.");
        else if (![manager supportsDestination:MTLCaptureDestinationGPUTraceDocument])
            trace_finish(@"This installation cannot save GPU traces. MetalCaptureEnabled must be enabled in the IPA.");
        else {
            MTLCaptureDescriptor *descriptor = [MTLCaptureDescriptor new];
            descriptor.captureObject = queue;
            descriptor.destination = MTLCaptureDestinationGPUTraceDocument;
            descriptor.outputURL = trace_url;
            NSError *error = nil;
            if (![manager startCaptureWithDescriptor:descriptor error:&error]) {
                trace_finish(error.localizedDescription ?: @"Metal could not start the GPU capture.");
            } else {
                trace_owned = YES; trace_queue = queue;
                madeira_trace_started(&trace_policy, NSProcessInfo.processInfo.systemUptime);
                fprintf(stderr, "[gpu-trace-v1] started before-command-buffer file=%s uptime=%.6f\n",
                    trace_url.lastPathComponent.UTF8String, trace_policy.started);
            }
        }
    }
    uint64_t ticket = trace_policy.phase == MGTraceRecording && trace_queue == queue ? trace_generation : 0;
    pthread_mutex_unlock(&trace_lock); return ticket;
}
void madeira_gpu_trace_did_create(void *rawBuffer, uint64_t ticket) {
    stage_buffer_created(rawBuffer);
    if (!ticket || !atomic_load_explicit(&trace_enabled, memory_order_acquire)) return;
    pthread_mutex_lock(&trace_lock);
    id<MTLCommandBuffer> buffer = (__bridge id)rawBuffer;
    if (ticket == trace_generation && trace_policy.phase == MGTraceRecording) {
        if (!buffer) trace_finish(@"Metal did not create a command buffer.");
        else if (madeira_trace_created(&trace_policy, (uintptr_t)(__bridge void *)buffer.commandQueue) < 0)
            trace_finish(@"Capture stopped at its 32-buffer limit; this sample is incomplete.");
        else if (buffer.commandQueue == trace_queue) [trace_created addObject:buffer];
    }
    pthread_mutex_unlock(&trace_lock);
}
void madeira_gpu_trace_present(void *rawBuffer) {
    stage_present(rawBuffer);
    if (!atomic_load_explicit(&trace_enabled, memory_order_acquire)) return;
    pthread_mutex_lock(&trace_lock);
    id<MTLCommandBuffer> buffer = (__bridge id)rawBuffer;
    if (buffer && (trace_policy.phase == MGTraceBoundary || [trace_created containsObject:buffer]))
        [trace_presented addObject:buffer];
    pthread_mutex_unlock(&trace_lock);
}
void madeira_gpu_trace_did_commit(void *rawBuffer) {
    if (!atomic_load_explicit(&trace_enabled, memory_order_acquire)) return;
    pthread_mutex_lock(&trace_lock);
    id<MTLCommandBuffer> buffer = (__bridge id)rawBuffer;
    BOOL captured = [trace_created containsObject:buffer], presents = [trace_presented containsObject:buffer];
    if (madeira_trace_committed(&trace_policy, (uintptr_t)(__bridge void *)buffer.commandQueue, captured, presents))
        trace_finish(nil);
    [trace_created removeObject:buffer]; [trace_presented removeObject:buffer];
    pthread_mutex_unlock(&trace_lock);
}
void madeira_gpu_trace_cancel(void) {
    pthread_mutex_lock(&trace_lock);
    stage_cancel();
    if (atomic_load_explicit(&trace_enabled, memory_order_relaxed))
        trace_finish(@"GPU capture cancelled or timed out; this sample is incomplete.");
    pthread_mutex_unlock(&trace_lock);
}
MadeiraGPUTraceStatus madeira_gpu_trace_status(void) {
    pthread_mutex_lock(&trace_lock);
    if (madeira_trace_expired(&trace_policy, NSProcessInfo.processInfo.systemUptime))
        trace_finish(@"GPU capture exceeded 15 seconds; this sample is incomplete.");
    MadeiraGPUTraceStatus result = {trace_policy.phase, trace_policy.buffers, trace_policy.frames,
        trace_policy.started, trace_policy.phase == MGTraceComplete};
    if (result.phase == MGTraceComplete && stage_waiting()) result.phase = MGTraceRecording;
    pthread_mutex_unlock(&trace_lock); return result;
}
NSString *madeira_gpu_trace_error(void) {
    pthread_mutex_lock(&trace_lock); NSString *message = trace_error;
    pthread_mutex_unlock(&trace_lock); return message;
}
