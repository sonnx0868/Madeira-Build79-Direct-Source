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
#define IOS_YIELD_BUDGET_NS UINT64_C(2000000)
#endif
#define IOS_YIELD_GAP_NS UINT64_C(100000)

struct ios_yield_state
{
    uint64_t last_ns, start_ns;
    unsigned int calls;
    int initialized;
};

static inline int ios_yield_should_park( struct ios_yield_state *s, uint64_t now,
                                         unsigned int minimum_calls )
{
    if (!s->initialized || now - s->last_ns >= IOS_YIELD_GAP_NS)
    {
        s->start_ns = now;
        s->calls = 0;
        s->initialized = 1;
    }
    s->last_ns = now;
    if (s->calls < UINT_MAX) ++s->calls;
    return now - s->start_ns >= IOS_YIELD_BUDGET_NS && s->calls >= minimum_calls;
}

/* The sleep is intentional idle time. Compare the next call with its end,
 * so a delayed timer wake doesn't turn a long poll back into a busy loop. */
static inline void ios_yield_did_park( struct ios_yield_state *s, uint64_t now )
{
    s->last_ns = now;
#if defined(IOS_YIELD_REARM_AFTER_PARK) && IOS_YIELD_REARM_AFTER_PARK
    s->start_ns = now;
    s->calls = 0;
#endif
}

#endif
