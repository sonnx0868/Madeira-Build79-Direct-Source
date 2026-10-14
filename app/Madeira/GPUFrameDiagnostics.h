#ifndef MADEIRA_GPU_FRAME_DIAGNOSTICS_H
#define MADEIRA_GPU_FRAME_DIAGNOSTICS_H
#include <pthread.h>
#include <stdint.h>

typedef struct {
    uint64_t buffers, timed_buffers, gpu_ns, max_gpu_ns, inflight, max_inflight, errors;
} MadeiraGPUFrameSample;
typedef struct {
    pthread_mutex_t lock;
    uint64_t epoch;
    int enabled;
    MadeiraGPUFrameSample sample;
} MadeiraGPUFrameDiagnostics;
#define MADEIRA_GPU_DIAGNOSTICS_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, 0, 0, {0}}

static inline void madeira_gpu_metrics_enable(MadeiraGPUFrameDiagnostics *metrics, int enabled) {
    pthread_mutex_lock(&metrics->lock);
    if (++metrics->epoch == 0) ++metrics->epoch;
    metrics->enabled = enabled;
    metrics->sample = (MadeiraGPUFrameSample){0};
    pthread_mutex_unlock(&metrics->lock);
}
static inline uint64_t madeira_gpu_metrics_begin(MadeiraGPUFrameDiagnostics *metrics) {
    pthread_mutex_lock(&metrics->lock);
    uint64_t ticket = metrics->enabled ? metrics->epoch : 0;
    if (ticket) {
        uint64_t depth = ++metrics->sample.inflight;
        if (depth > metrics->sample.max_inflight) metrics->sample.max_inflight = depth;
    }
    pthread_mutex_unlock(&metrics->lock);
    return ticket;
}
static inline void madeira_gpu_metrics_complete(MadeiraGPUFrameDiagnostics *metrics,
                                                uint64_t ticket, double start, double end, int error) {
    if (!ticket) return;
    pthread_mutex_lock(&metrics->lock);
    if (metrics->enabled && metrics->epoch == ticket) {
        MadeiraGPUFrameSample *sample = &metrics->sample;
        if (sample->inflight) --sample->inflight;
        ++sample->buffers;
        if (error) ++sample->errors;
        // Unavailable timestamps (including failed buffers) must not look like fast GPU work.
        if (!error && start > 0 && end > start && end - start < 60) {
            uint64_t ns = (uint64_t)((end - start) * 1e9);
            ++sample->timed_buffers;
            sample->gpu_ns += ns;
            if (ns > sample->max_gpu_ns) sample->max_gpu_ns = ns;
        }
    }
    pthread_mutex_unlock(&metrics->lock);
}
static inline MadeiraGPUFrameSample madeira_gpu_metrics_take(MadeiraGPUFrameDiagnostics *metrics) {
    pthread_mutex_lock(&metrics->lock);
    MadeiraGPUFrameSample sample = metrics->sample;
    metrics->sample = (MadeiraGPUFrameSample){.inflight=sample.inflight, .max_inflight=sample.inflight};
    pthread_mutex_unlock(&metrics->lock);
    return sample;
}
#endif
