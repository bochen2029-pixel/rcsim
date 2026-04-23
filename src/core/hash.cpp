#include "rcsim/core/hash.hpp"

#include "rcsim/state/world_state.hpp"

// TODO(phase 1, §2.4, §2.6, §3.5, §9.9): implement Blake3-256 canonical hash
// via deterministic_tree_reduce (§2.4) + canonicalize_for_hash (§2.6).

namespace rc::sim::core {

bool StateHash::operator==(const StateHash& other) const noexcept {
    // TODO(phase 1): byte-compare; may delegate to std::equal.
    return bytes == other.bytes;
}

bool StateHash::operator!=(const StateHash& other) const noexcept {
    return !(*this == other);
}

StateHash canonical_hash(const state::WorldState& /*s*/) noexcept {
    // TODO(phase 1, §2.6, §3.5, §9.9)
    return StateHash{};
}

double canonicalize_for_hash(double v) noexcept {
    // TODO(phase 1, §2.6): delegate to canonicalize.cpp implementation.
    return v;
}

StateHash hash_chain_link(const StateHash& /*prev*/, const StateHash& /*current*/) noexcept {
    // TODO(phase 4, §10.1)
    return StateHash{};
}

}  // namespace rc::sim::core
