/* SPDX-License-Identifier: MIT */
#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
namespace dxmt {
// Same 64 completed-entry reserve as before. No new allocations or retention.
template<class Queue> size_t completedNames(const Queue &queue, uint64_t sequence, bool ordered) {
  if (!ordered) {
    return static_cast<size_t>(std::count_if(queue.begin(), queue.end(),
        [sequence](const auto &entry) { return entry.will_free_at <= sequence; }));
  }
  return static_cast<size_t>(std::upper_bound(queue.begin(), queue.end(), sequence,
      [](uint64_t seq, const auto &entry) { return seq < entry.will_free_at; }) - queue.begin());
}
} // namespace dxmt
