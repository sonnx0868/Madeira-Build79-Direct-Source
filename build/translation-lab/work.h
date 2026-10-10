/* SPDX-License-Identifier: MIT
 * Synthetic integer work for measuring x64/ARM64EC transition granularity.
 * This is a CPU experiment, not a game workload or an FPS prediction.
 */
#pragma once
#include <stdint.h>

struct translation_packet { uint64_t a, b, c, d; };
static inline uint64_t translation_step(const struct translation_packet *p, uint64_t state)
{
    state ^= p->a + UINT64_C(0x9e3779b97f4a7c15);
    state = (state << 13) | (state >> 51);
    state += p->b ^ p->c;
    state *= p->d | 1;
    return state ^ (state >> 29);
}
static inline uint64_t translation_run(const struct translation_packet *p, uint32_t count,
                                       uint32_t passes, uint64_t state)
{
    for (uint32_t pass = 0; pass < passes; pass++)
        for (uint32_t i = 0; i < count; i++) state = translation_step(p + i, state);
    return state;
}
