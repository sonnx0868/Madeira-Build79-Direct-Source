// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>

namespace madeira {

enum class UploadBatchKind : uint8_t { Empty, Barrier, Compute, Copy };

// Move an upload into an earlier copy encoder only after checking every crossed
// pass. Cache classifications in a fixed ring: command chains are inspected once
// and each upload checks at most 64 live predecessors, including its merge target.
// The caller supplies the production allocation dependency test and preserves
// command order when appending the upload to that target.
template <class Pass, class Classify, class Dependent, class Merge>
unsigned batch_buffer_uploads(Pass **passes, unsigned count, Classify classify,
                             Dependent dependent, Merge merge) {
  constexpr unsigned lookback = 64;
  std::array<UploadBatchKind, lookback> kinds{};
  unsigned live = 0;
  for (unsigned read = 0; read < count; ++read) {
    auto *candidate = passes[read];
    const auto kind = classify(candidate);
    if (kind == UploadBatchKind::Empty)
      continue;
    bool merged = false;
    if (kind == UploadBatchKind::Copy) {
      const unsigned first = live > lookback ? live - lookback : 0;
      for (unsigned j = live; j > first;) {
        --j;
        const auto previous_kind = kinds[j % lookback];
        if (previous_kind != UploadBatchKind::Copy && previous_kind != UploadBatchKind::Compute)
          break;
        auto *previous = passes[j];
        if (dependent(candidate, previous))
          break;
        if (previous_kind == UploadBatchKind::Copy) {
          merge(previous, candidate);
          merged = true;
          break;
        }
      }
    }
    if (!merged) {
      kinds[live % lookback] = kind;
      passes[live++] = candidate;
    }
  }
  return live;
}

} // namespace madeira
