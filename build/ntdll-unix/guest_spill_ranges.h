#ifndef MADEIRA_GUEST_SPILL_RANGES_H
#define MADEIRA_GUEST_SPILL_RANGES_H
#include <stdint.h>
struct ios_spill_range { uint64_t lo, hi; };
/* Half-open allocation windows. Generic guest reserves may not take any
 * slice of the published host arena; FEX's bounded requests use its arena
 * through NtAllocateVirtualMemoryEx instead of this generic spill path. */
static unsigned ios_guest_spill_ranges(uint64_t lo, uint64_t hi,
                                      uint64_t host_lo, uint64_t host_hi,
                                      struct ios_spill_range out[2])
{
    unsigned count = 0;
    if (hi <= lo) return 0;
    if (!host_lo || host_hi <= host_lo || host_hi <= lo || host_lo >= hi)
    { out[0] = (struct ios_spill_range){lo, hi}; return 1; }
    if (lo < host_lo) out[count++] = (struct ios_spill_range){lo, host_lo < hi ? host_lo : hi};
    if (host_hi < hi) out[count++] = (struct ios_spill_range){host_hi > lo ? host_hi : lo, hi};
    return count;
}
#endif
