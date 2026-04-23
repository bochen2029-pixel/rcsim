#include "rcsim/state/world_state.hpp"

#include "rcsim/core/hash.hpp"

// §3.5: state_hash delegates to core::canonical_hash.

namespace rc::sim::state {

core::StateHash state_hash(const WorldState& s) noexcept {
    return core::canonical_hash(s);
}

}  // namespace rc::sim::state
