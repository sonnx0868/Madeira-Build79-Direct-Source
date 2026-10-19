/* SPDX-License-Identifier: LGPL-2.1-or-later
 * Copyright 2026 Evan Bolton
 * Short handoffs stay yields; only a sustained polling loop may sleep.
 * Based on NutsEmu Wine patch 0122 (LGPL-2.1-or-later, Evan Bolton, 2026).
 * The pure state machine is shared by the runtime and host verification.
 */
#ifndef MADEIRA_IOS_YIELD_POLICY_H
#define MADEIRA_IOS_YIELD_POLICY_H

#include <stdint.h>
#include <limits.h>

#ifndef IOS_YIELD_BUDGET_NS
#define IOS_YIELD_BUDGET_NS UINT64_C(32000)
#endif
#ifndef IOS_YIELD_ADAPTIVE
#define IOS_YIELD_ADAPTIVE 1
#endif
#define IOS_YIELD_GAP_NS UINT64_C(100000)

struct ios_yield_state
{
    uint64_t last_ns, start_ns;
    unsigned int calls, idle_parks;
    int initialized;
};

static inline int ios_yield_should_park( struct ios_yield_state *s, uint64_t now,
                                         unsigned int minimum_calls )
{
    if (!s->initialized || now - s->last_ns >= IOS_YIELD_GAP_NS)
    {
        s->start_ns = now;
        s->calls = 0;
        s->idle_parks = 0;
        s->initialized = 1;
    }
    s->last_ns = now;
    if (s->calls < UINT_MAX) ++s->calls;
    uint64_t budget = IOS_YIELD_BUDGET_NS;
#if defined(IOS_YIELD_ADAPTIVE) && IOS_YIELD_ADAPTIVE
#ifndef IOS_YIELD_MIN_BUDGET_NS
#define IOS_YIELD_MIN_BUDGET_NS UINT64_C(4000)
#endif
    if (s->idle_parks >= 4) budget = IOS_YIELD_MIN_BUDGET_NS;
    else if (s->idle_parks >= 2) budget = UINT64_C(8000);
#endif
    return now - s->start_ns >= budget && s->calls >= minimum_calls;
}

static inline unsigned int ios_yield_park_us( const struct ios_yield_state *s,
                                             unsigned int maximum_us )
{
#if defined(IOS_YIELD_ADAPTIVE) && IOS_YIELD_ADAPTIVE
    unsigned int short_us = s->idle_parks < 2 ? 50 : s->idle_parks < 4 ? 75 : maximum_us;
    return short_us < maximum_us ? short_us : maximum_us;
#else
    (void)s;
    return maximum_us;
#endif
}

/* The sleep is intentional idle time. Compare the next call with its end,
 * so a delayed timer wake doesn't turn a long poll back into a busy loop. */
static inline void ios_yield_did_park( struct ios_yield_state *s, uint64_t now )
{
    s->last_ns = now;
#if (defined(IOS_YIELD_REARM_AFTER_PARK) && IOS_YIELD_REARM_AFTER_PARK) || \
    (defined(IOS_YIELD_ADAPTIVE) && IOS_YIELD_ADAPTIVE)
    s->start_ns = now;
    s->calls = 0;
#if defined(IOS_YIELD_ADAPTIVE) && IOS_YIELD_ADAPTIVE
    if (s->idle_parks < 4) ++s->idle_parks;
#endif
#endif
}

#endif
