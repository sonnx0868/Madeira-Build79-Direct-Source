#pragma once
#include <math.h>
#include <stdint.h>

typedef struct {
    uint64_t cpu_ticks, cycles, instructions;
} MadeiraWorkCounters;

typedef struct {
    int valid, counter_valid;
    double cpu_ms, cpu_cores, cycles_per_cpu_ns, instructions_per_cycle;
    double cpu_ms_per_frame, instructions_per_frame;
    uint64_t cycles, instructions;
} MadeiraWorkSample;

/* CPU ticks use the Mach timebase. A cycle rate includes all sampled cores;
 * migration and IPC changes prevent treating it as a physical clock reading.
 * Unsupported/rolled-back counters must not look like a frequency collapse. */
static inline MadeiraWorkSample madeira_work_sample(MadeiraWorkCounters before,
                                                     MadeiraWorkCounters after,
                                                     uint32_t numer, uint32_t denom,
                                                     double seconds, uint64_t frames) {
    MadeiraWorkSample s = {0};
    s.cpu_ms_per_frame = s.instructions_per_frame = -1;
    if (!numer || !denom || !isfinite(seconds) || seconds <= 0 || after.cpu_ticks < before.cpu_ticks)
        return s;
    const double cpu_ns = (double)(after.cpu_ticks - before.cpu_ticks) * numer / denom;
    if (!isfinite(cpu_ns)) return s;
    s.valid = 1;
    s.cpu_ms = cpu_ns / 1e6;
    s.cpu_cores = cpu_ns / (seconds * 1e9);
    if (frames) s.cpu_ms_per_frame = s.cpu_ms / frames;
    if (after.cycles < before.cycles || after.instructions < before.instructions) return s;
    s.cycles = after.cycles - before.cycles;
    s.instructions = after.instructions - before.instructions;
    if (cpu_ns <= 0 || !s.cycles || !s.instructions) return s;
    s.counter_valid = 1;
    s.cycles_per_cpu_ns = s.cycles / cpu_ns;
    s.instructions_per_cycle = (double)s.instructions / s.cycles;
    if (frames) s.instructions_per_frame = (double)s.instructions / frames;
    return s;
}
