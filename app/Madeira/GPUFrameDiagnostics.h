#ifndef MADEIRA_GPU_FRAME_DIAGNOSTICS_H
#define MADEIRA_GPU_FRAME_DIAGNOSTICS_H
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint64_t buffers, timed_buffers, gpu_ns, max_gpu_ns, inflight, max_inflight, errors;
    uint64_t union_gpu_ns, span_gpu_ns, timing_dropped;
} MadeiraGPUFrameSample;

enum { MADEIRA_GPU_INTERVAL_LIMIT = 4096 };
typedef struct { double start, end; } MadeiraGPUInterval;
typedef struct {
    pthread_mutex_t lock;
    uint64_t epoch;
    int enabled;
    MadeiraGPUFrameSample sample;
    unsigned interval_count;
    MadeiraGPUInterval intervals[MADEIRA_GPU_INTERVAL_LIMIT];
} MadeiraGPUFrameDiagnostics;
#define MADEIRA_GPU_DIAGNOSTICS_INITIALIZER {PTHREAD_MUTEX_INITIALIZER, 0, 0, {0}, 0, {{0}}}

static inline void madeira_gpu_metrics_enable(MadeiraGPUFrameDiagnostics *metrics, int enabled) {
    pthread_mutex_lock(&metrics->lock);
    if (++metrics->epoch == 0) ++metrics->epoch;
    metrics->enabled = enabled;
    metrics->sample = (MadeiraGPUFrameSample){0};
    metrics->interval_count = 0;
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
            if (metrics->interval_count < MADEIRA_GPU_INTERVAL_LIMIT)
                metrics->intervals[metrics->interval_count++] = (MadeiraGPUInterval){start, end};
            else
                ++sample->timing_dropped;
        }
    }
    pthread_mutex_unlock(&metrics->lock);
}
static int madeira_gpu_interval_compare(const void *left, const void *right) {
    double a = ((const MadeiraGPUInterval *)left)->start;
    double b = ((const MadeiraGPUInterval *)right)->start;
    return (a > b) - (a < b);
}
static inline MadeiraGPUFrameSample madeira_gpu_metrics_take(MadeiraGPUFrameDiagnostics *metrics) {
    pthread_mutex_lock(&metrics->lock);
    MadeiraGPUFrameSample sample = metrics->sample;
    /* Completion handlers from different queues can arrive out of timestamp
     * order. Merge intervals before reporting coverage; summing durations can
     * count overlapping work twice. This covers the completed batch, not an
     * on-chip utilization counter. Overflow is reported explicitly. */
    if (metrics->interval_count) {
        qsort(metrics->intervals, metrics->interval_count, sizeof(metrics->intervals[0]), madeira_gpu_interval_compare);
        double first = metrics->intervals[0].start;
        double start = first, end = metrics->intervals[0].end, covered = 0;
        for (unsigned i = 1; i < metrics->interval_count; ++i) {
            MadeiraGPUInterval item = metrics->intervals[i];
            if (item.start > end) {
                covered += end - start;
                start = item.start;
            }
            if (item.end > end) end = item.end;
        }
        sample.union_gpu_ns = (uint64_t)((covered + end - start) * 1e9);
        sample.span_gpu_ns = (uint64_t)((end - first) * 1e9);
    }
    metrics->interval_count = 0;
    metrics->sample = (MadeiraGPUFrameSample){.inflight=sample.inflight, .max_inflight=sample.inflight};
    pthread_mutex_unlock(&metrics->lock);
    return sample;
}
#endif
