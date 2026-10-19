// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <cstdint>

namespace madeira {
// Submit independent texture uploads during loading, before the next Present.
// One oversized upload remains legal; the following upload retires that batch.
inline bool flush_upload_batch(uint64_t pending, uint64_t incoming) {
  constexpr uint64_t limit = 64ull << 20;
  return pending && (pending >= limit || incoming > limit - pending);
}

// Reserve the spare allocation BEFORE allocating. Independent immediate
// contexts share this counter; a check followed by fetch_add can overshoot it.
inline bool reserve_staging_spare(std::atomic<uint64_t> &bytes, uint64_t size, uint64_t budget) {
  uint64_t used = bytes.load(std::memory_order_relaxed);
  do {
    if (used > budget || size > budget - used) return false;
  } while (!bytes.compare_exchange_weak(used, used + size, std::memory_order_relaxed));
  return true;
}
}
