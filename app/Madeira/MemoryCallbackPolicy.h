#ifndef MADEIRA_MEMORY_CALLBACK_POLICY_H
#define MADEIRA_MEMORY_CALLBACK_POLICY_H

#include <stdlib.h>

/* The old FEX interval mutex is not recursive. Wine already knows how to
 * suppress allocator notifications while its loader calls NotifyImageMap.
 * Enable that existing guard before any PE module initializes, while keeping
 * an explicit cfg/Dock value (including 0) authoritative. */
static inline void madeira_prepare_memory_callbacks(void)
{
    if (!getenv("MADEIRA_IMAGE_MAP_GUARD"))
        setenv("MADEIRA_IMAGE_MAP_GUARD", "1", 0);
}

#endif
