/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef MADEIRA_GAME_MEMORY_PROFILE_H
#define MADEIRA_GAME_MEMORY_PROFILE_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline int madeira_game_swap_override(long *capacity)
{
    const char *value = getenv("_MADEIRA_GAME_SWAP_MB");
    static const long choices[] = {0, 1024, 2048, 3072, 4096};
    if (!value) return 0;
    for (unsigned i = 0; i < sizeof(choices) / sizeof(choices[0]); ++i)
    {
        char expected[16];
        snprintf(expected, sizeof(expected), "%ld", choices[i]);
        if (!strcmp(value, expected)) { *capacity = choices[i]; return 1; }
    }
    return 0;
}

static inline long madeira_game_swap_capacity(long shared)
{
    long capacity;
    return madeira_game_swap_override(&capacity) ? capacity : shared;
}

/* Global env.* exports run after the profile. Reapply just the two explicit
 * choices so they cannot overwrite it or revive an explicitly disabled tier. */
static inline void madeira_apply_game_memory_overrides(void)
{
    const char *suspend = getenv("_MADEIRA_GAME_REAL_SUSPEND");
    long capacity;
    if (suspend && (!strcmp(suspend, "0") || !strcmp(suspend, "1")))
        setenv("MADEIRA_REAL_SUSPEND", suspend, 1);
    if (madeira_game_swap_override(&capacity))
    {
        char value[16]; snprintf(value, sizeof(value), "%ld", capacity);
        setenv("MADEIRA_SWAP_MB", value, 1);
        if (!capacity) unsetenv("MADEIRA_SWAP_FILE");
    }
}
#endif
