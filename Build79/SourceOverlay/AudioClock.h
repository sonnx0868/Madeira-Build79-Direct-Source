#ifndef MADEIRA_AUDIO_CLOCK_H
#define MADEIRA_AUDIO_CLOCK_H
#include <stdint.h>
#include <stdatomic.h>
typedef struct { _Atomic uint64_t frames; } MadeiraHardwareClock;
static inline void MadeiraClockAdvance(MadeiraHardwareClock *c,uint32_t requested) { atomic_fetch_add_explicit(&c->frames,requested,memory_order_relaxed); }
static inline uint64_t MadeiraClockRead(MadeiraHardwareClock *c) { return atomic_load_explicit(&c->frames,memory_order_relaxed); }
static inline void MadeiraClockReset(MadeiraHardwareClock *c) { atomic_store_explicit(&c->frames,0,memory_order_relaxed); }
#endif
