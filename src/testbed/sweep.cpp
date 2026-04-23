#include "rcsim/testbed/sweep.hpp"

// §11.4: Blake3 seed derivation per (seed_root, param_idx, sample_idx).

namespace rc::sim::testbed {

uint64_t derive_sample_seed(uint64_t /*seed_root*/, uint32_t /*param_idx*/, uint32_t /*sample_idx*/) noexcept {
    // TODO(phase 5, §11.4): Blake3-256 truncation.
    return 0;
}

}  // namespace rc::sim::testbed
