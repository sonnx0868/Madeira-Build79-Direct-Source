#pragma once
#include <stdint.h>
#include <math.h>

typedef struct { uint64_t ticks, cycles, instructions, energy_nj; } MadeiraPerfCounters;
typedef struct {
    int valid, rate_valid;
    double cpu_ms, ghz, ipc, million_instructions, energy_mj;
} MadeiraPerfDelta;

/* Times are Mach ticks. GHz here is cycles / running time within ONE
 * performance level, averaged over the window, not an instantaneous clock. */
static inline MadeiraPerfDelta madeira_perf_delta(MadeiraPerfCounters a, MadeiraPerfCounters b,
                                                  uint32_t numer, uint32_t denom) {
    MadeiraPerfDelta d = { .ghz = -1, .ipc = -1, .energy_mj = -1 };
    if (!numer || !denom || b.ticks < a.ticks || b.cycles < a.cycles ||
        b.instructions < a.instructions) return d;
    double ns = (double)(b.ticks-a.ticks)*numer/denom;
    d.valid = 1; d.cpu_ms = ns/1e6;
    d.million_instructions = (double)(b.instructions-a.instructions)/1e6;
    if (b.energy_nj >= a.energy_nj) d.energy_mj = (double)(b.energy_nj-a.energy_nj)/1e6;
    if (ns > 0 && b.cycles > a.cycles && b.instructions > a.instructions) {
        d.rate_valid = 1;
        d.ghz = (double)(b.cycles-a.cycles)/ns;
        d.ipc = (double)(b.instructions-a.instructions)/(b.cycles-a.cycles);
    }
    return d;
}

static inline int madeira_perf_subtract(MadeiraPerfCounters total, MadeiraPerfCounters part,
                                        MadeiraPerfCounters *rest) {
    if (part.ticks > total.ticks || part.cycles > total.cycles ||
        part.instructions > total.instructions || part.energy_nj > total.energy_nj) return 0;
    *rest = (MadeiraPerfCounters){total.ticks-part.ticks, total.cycles-part.cycles,
        total.instructions-part.instructions, total.energy_nj-part.energy_nj};
    return 1;
}
