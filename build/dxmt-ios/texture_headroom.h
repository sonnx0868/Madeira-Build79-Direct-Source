#ifndef MADEIRA_TEXTURE_HEADROOM_H
#define MADEIRA_TEXTURE_HEADROOM_H

#include <stdint.h>

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
