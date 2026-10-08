/* Fresh allocations only: never move or discard already committed game data. */
#ifndef MADEIRA_IOS_SWAP_PRESSURE_H
#define MADEIRA_IOS_SWAP_PRESSURE_H
#include <stdint.h>
#include <stddef.h>

static inline int ios_swap_pressure_needs_backing(int enabled, uint64_t available, size_t allocation)
{
    /* Unknown headroom retains the existing backing policy. Account for the
     * incoming commit and keep 2 GB for graphics/Mono/other native allocations.
     * Subtraction avoids overflow for oversized reservation requests. */
    if (!enabled || !available) return 1;
    return (uint64_t)allocation >= available || available - (uint64_t)allocation <= (2ull << 30);
}

static inline int ios_swap_pressure_back_commit(int enabled, uint64_t *available, size_t allocation)
{
    uint64_t current = __atomic_load_n(available, __ATOMIC_RELAXED);
    if (!enabled) return 1;
    do
    {
        if (ios_swap_pressure_needs_backing(enabled, current, allocation)) return 1;
        /* Charge fresh anonymous commits between monitor samples. Otherwise a
         * rapid burst could repeatedly spend the same cached memory headroom. */
    } while (!__atomic_compare_exchange_n(available, &current, current - (uint64_t)allocation,
                                          0, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return 0;
}

void ios_swap_pressure_update(uint64_t available);
#endif
