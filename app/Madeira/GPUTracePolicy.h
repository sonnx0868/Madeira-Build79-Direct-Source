/* SPDX-License-Identifier: MIT
 * Caller serializes access. Queue identity and complete presentation boundaries
 * matter: a capture started at commit time misses the encoded render work. */
#pragma once
#include <stdint.h>

enum { MGTraceIdle, MGTraceBoundary, MGTraceReady, MGTraceRecording, MGTraceComplete, MGTraceFailed };
typedef struct {
    int phase;
    uintptr_t queue;
    unsigned buffers, frames;
    double requested, started;
} MadeiraGPUTracePolicy;

static inline void madeira_trace_arm(MadeiraGPUTracePolicy *p, double now) {
    *p = (MadeiraGPUTracePolicy){.phase=MGTraceBoundary, .requested=now};
}
static inline int madeira_trace_startable(MadeiraGPUTracePolicy *p, uintptr_t queue) {
    return p->phase == MGTraceReady && p->queue == queue;
}
static inline void madeira_trace_started(MadeiraGPUTracePolicy *p, double now) {
    p->phase = MGTraceRecording; p->started = now;
}
static inline int madeira_trace_created(MadeiraGPUTracePolicy *p, uintptr_t queue) {
    if (p->phase != MGTraceRecording || p->queue != queue) return 0;
    if (++p->buffers > 32) { p->phase = MGTraceFailed; return -1; }
    return 1;
}
/* Only a buffer created inside this capture can complete a captured frame. */
static inline int madeira_trace_committed(MadeiraGPUTracePolicy *p, uintptr_t queue,
                                         int captured, int presents) {
    if (p->phase == MGTraceBoundary && presents) {
        p->queue = queue; p->phase = MGTraceReady; return 0;
    }
    if (p->phase != MGTraceRecording || p->queue != queue || !captured || !presents) return 0;
    if (++p->frames == 2) { p->phase = MGTraceComplete; return 1; }
    return 0;
}
static inline int madeira_trace_expired(const MadeiraGPUTracePolicy *p, double now) {
    return (p->phase == MGTraceBoundary || p->phase == MGTraceReady || p->phase == MGTraceRecording) &&
        now - (p->phase == MGTraceRecording ? p->started : p->requested) > 15;
}
