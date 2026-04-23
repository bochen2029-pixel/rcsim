#pragma once

// §2.6: Signed-zero canonicalization + NaN/Inf trap.
// §3.5: state_hash(const WorldState&) noexcept → uses canonicalize_for_hash + tree_reduce.
// §9.9: Hash state via deterministic_tree_reduce over canonicalized fields.

#include <array>
#include <cstdint>

namespace rc::sim::state {
struct WorldState;
}

namespace rc::sim::core {

// §2.6, §9.9: State hash is Blake3-256, 32 bytes.
struct StateHash {
    std::array<uint8_t, 32> bytes{};

    // TODO(phase 1): implement equality for hash-chain verification.
    bool operator==(const StateHash& other) const noexcept;
    bool operator!=(const StateHash& other) const noexcept;
};

// §3.5, §9.9: canonical_hash over WorldState.
// Uses canonicalize_for_hash (§2.6) on every field; uses deterministic_tree_reduce (§2.4) for reductions.
// TODO(phase 1, §2.6, §3.5, §9.9): implement per DESIGN_v1.3.md
StateHash canonical_hash(const state::WorldState& s) noexcept;

// §2.6: Signed-zero canonicalization sweep. Declared here and in canonicalize.hpp.
// Forwarded for ergonomics when hashing. Authoritative definition in canonicalize.hpp.
double canonicalize_for_hash(double v) noexcept;

// Hash-chain link: Blake3-256(prev_hash || canonical_hash(s)).
// §10.1: sync log binary format. Chain hash per tick.
// TODO(phase 4, §10.1): implement hash-chain for sync log.
StateHash hash_chain_link(const StateHash& prev, const StateHash& current) noexcept;

}  // namespace rc::sim::core
