/* SPDX-License-Identifier: MIT
 * Producer deadlines. No vblank rounding, spin loop or catch-up frame burst.
 * Time units are the caller's monotonic clock ticks. One state per producer. */
#pragma once
#include <stdint.h>

typedef struct {
    uint64_t next, period, generation;
    uintptr_t surface;
} MadeiraPresentPacer;

/* A producer can service several windows. Keep independent deadlines without
 * allocating or serializing unrelated layers behind one shared clock. */
static inline MadeiraPresentPacer *madeira_present_surface(MadeiraPresentPacer states[8], uintptr_t surface) {
    unsigned oldest = 0;
    for (unsigned i = 0; i < 8; ++i) {
        if (states[i].surface == surface || !states[i].next) return &states[i];
        if (states[i].next < states[oldest].next) oldest = i;
    }
    states[oldest] = (MadeiraPresentPacer){0};
    return &states[oldest];
}

static inline uint64_t madeira_present_deadline(MadeiraPresentPacer *p,
    uint64_t now, uint64_t period, uintptr_t surface, uint64_t generation) {
    if (!period || now > UINT64_MAX - period) {
        *p = (MadeiraPresentPacer){0};
        return now;
    }
    if (!p->next || p->period != period || p->surface != surface || p->generation != generation) {
        *p = (MadeiraPresentPacer){now + period, period, generation, surface};
        return now;
    }
    uint64_t deadline = p->next;
    if (now >= deadline) {
        /* A late frame runs immediately. A stall cannot bank catch-up work. */
        p->next = now - deadline >= period ? now + period : deadline + period;
        return now;
    }
    p->next = deadline > UINT64_MAX - period ? now + period : deadline + period;
    return deadline;
}

static inline unsigned madeira_present_rate(int mode, int panel) {
    if (panel <= 0 || panel > 1000) panel = 60;
    unsigned target = mode == 3 ? 30 : mode == 1 ? 60 : (unsigned)panel;
    return target > (unsigned)panel ? (unsigned)panel : target;
}
