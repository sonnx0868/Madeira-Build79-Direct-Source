#ifndef MADEIRA_TEXTURE_HEADROOM_H
#define MADEIRA_TEXTURE_HEADROOM_H

#include <stdint.h>

/* A zero cap retains the caller's process-only policy. Use saturating
 * arithmetic so an unavailable/oversized reading cannot wrap into a tiny
 * budget. The native wrapper obtains cap from physical RAM minus the
 * configured iPadOS reserve, as in the reviewed BCD memory fix. */
static inline uint64_t madeira_memory_limit(uint64_t available, uint64_t footprint, uint64_t cap) {
    uint64_t limit = available > UINT64_MAX - footprint ? UINT64_MAX : available + footprint;
    return cap && cap < limit ? cap : limit;
}

static inline uint64_t madeira_memory_available(uint64_t available, uint64_t footprint, uint64_t cap) {
    if (!available || !cap)
        return available;
    /* Zero means the OS could not supply a limit to older query callers.
     * A valid exhausted cap therefore reports at most 1 MB, which is well
     * below the existing pressure threshold and preserves that protocol. */
    uint64_t remaining = cap > footprint ? cap - footprint : UINT64_C(1048576);
    return remaining < available ? remaining : available;
}

/* Allocation-time texture pressure on a unified-memory GPU must respect
 * both the process's remaining memory and Metal's performance working set.
 * Neither number is a command to discard live GPU resources. Unknown GPU
 * guidance and discrete GPUs preserve the process-only policy. */
static inline uint64_t madeira_texture_headroom(uint64_t process_available,
                                                uint64_t metal_recommended,
                                                uint64_t metal_allocated,
                                                int unified_memory) {
    if (!unified_memory || !metal_recommended)
        return process_available;
    uint64_t gpu_available = metal_allocated < metal_recommended
                                 ? metal_recommended - metal_allocated : 0;
    return gpu_available < process_available ? gpu_available : process_available;
}

#endif
