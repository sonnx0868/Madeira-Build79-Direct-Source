/* SPDX-License-Identifier: MIT
 * CPU time inside Present. GPU execution is measured separately; these
 * intervals must not be relabelled as GPU time or added to overlapping work.
 */
#pragma once
#include <array>
#include <chrono>
#include <cstddef>

namespace madeira {
enum class PresentWait : std::size_t { DeviceLock, Prepare, Submit, Layer, FrameFence, Commit, Queue, Count };

struct PresentWaits {
  std::array<double, static_cast<std::size_t>(PresentWait::Count)> milliseconds{};
  double get(PresentWait part) const { return milliseconds[static_cast<std::size_t>(part)]; }
  void clear() { milliseconds.fill(0); }
};

template<class Clock = std::chrono::steady_clock>
class PresentWaitScope {
  PresentWaits &waits_;
  PresentWait part_;
  typename Clock::time_point start_ = Clock::now();
public:
  PresentWaitScope(PresentWaits &waits, PresentWait part) : waits_(waits), part_(part) {}
  PresentWaitScope(const PresentWaitScope &) = delete;
  PresentWaitScope &operator=(const PresentWaitScope &) = delete;
  ~PresentWaitScope() {
    waits_.milliseconds[static_cast<std::size_t>(part_)] +=
        std::chrono::duration<double, std::milli>(Clock::now() - start_).count();
  }
};
}
