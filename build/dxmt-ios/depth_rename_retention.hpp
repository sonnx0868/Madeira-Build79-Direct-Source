#pragma once

#include <cstddef>
#include <cstdint>

namespace madeira {

// Keep the two common backing slots warm. A larger burst may still use all
// slots; only an unused, completed tail allocation loses the pool's reference.
// This is a retirement policy, not a smaller active rename capacity.
inline constexpr std::size_t depth_rename_warm_slots = 2;
inline constexpr std::uint64_t depth_rename_idle_submissions = 300;

inline bool retire_depth_rename_slot(std::size_t slot, std::uint64_t last_gpu_use,
                                     std::uint64_t coherent_submission) {
  return slot >= depth_rename_warm_slots &&
         coherent_submission >= last_gpu_use &&
         coherent_submission - last_gpu_use >= depth_rename_idle_submissions;
}

} // namespace madeira
