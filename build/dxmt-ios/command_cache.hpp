/* SPDX-License-Identifier: MIT
 * Bounded adaptation of willfaust/dxmt#42. Disabled until paired device tests.
 */
#pragma once
#include <atomic>
#include <cstdlib>
#include <cstring>

namespace dxmt {
inline std::atomic<unsigned> command_cache_slots{0};
class CommandCacheLease {
  bool owned_ = false;
public:
  CommandCacheLease() = default;
  CommandCacheLease(const CommandCacheLease &) = delete;
  CommandCacheLease &operator=(const CommandCacheLease &) = delete;
  ~CommandCacheLease() { release(); }
  bool acquire() {
    if (owned_) return true;
    unsigned used = command_cache_slots.load(std::memory_order_relaxed);
    while (used < 4) {
      if (command_cache_slots.compare_exchange_weak(used, used + 1, std::memory_order_relaxed))
        return owned_ = true;
    }
    return false;
  }
  void release() {
    if (owned_) {
      owned_ = false;
      command_cache_slots.fetch_sub(1, std::memory_order_relaxed);
    }
  }
};
inline bool commandCacheEnabled() {
  static const bool on = [] {
    const char *v = std::getenv("DXMT_CMDLIST_KEEP_BLOCK");
    return v && std::strcmp(v, "1") == 0;
  }();
  return on;
}
} // namespace dxmt
