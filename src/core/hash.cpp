#include "rcsim/core/hash.hpp"

#include <algorithm>
#include <cstring>

#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/tree_reduce.hpp"
#include "rcsim/state/world_state.hpp"

// §2.4 + §2.6 + §3.5 + §9.9: Blake3-256 canonical hash with signed-zero
// canonicalization, Kahan-residual-aware fields, and deterministic_tree_reduce
// for cross-territory aggregation.

extern "C" {
struct blake3_hasher {
    uint8_t opaque[1912];
};
void blake3_hasher_init(blake3_hasher* self);
void blake3_hasher_update(blake3_hasher* self, const void* input, size_t input_len);
void blake3_hasher_finalize(const blake3_hasher* self, uint8_t* out, size_t out_len);
}  // extern "C"

namespace rc::sim::core {

bool StateHash::operator==(const StateHash& other) const noexcept {
    return bytes == other.bytes;
}

bool StateHash::operator!=(const StateHash& other) const noexcept {
    return !(*this == other);
}

namespace {

// Hash a canonicalized double into the blake3 stream as 8 raw bytes (little-endian).
inline void hash_double(blake3_hasher& h, double v) noexcept {
    double canon = canonicalize_for_hash(v);
    uint64_t bits;
    std::memcpy(&bits, &canon, sizeof(bits));
    uint8_t le[8];
    for (int i = 0; i < 8; ++i) {
        le[i] = static_cast<uint8_t>((bits >> (8 * i)) & 0xFFu);
    }
    blake3_hasher_update(&h, le, sizeof(le));
}

inline void hash_uint64(blake3_hasher& h, uint64_t v) noexcept {
    uint8_t le[8];
    for (int i = 0; i < 8; ++i) {
        le[i] = static_cast<uint8_t>((v >> (8 * i)) & 0xFFu);
    }
    blake3_hasher_update(&h, le, sizeof(le));
}

inline void hash_uint32(blake3_hasher& h, uint32_t v) noexcept {
    uint8_t le[4];
    for (int i = 0; i < 4; ++i) {
        le[i] = static_cast<uint8_t>((v >> (8 * i)) & 0xFFu);
    }
    blake3_hasher_update(&h, le, sizeof(le));
}

// Hash one TerritoryState into a blake3 stream and produce a 32-byte digest.
// The digest is then fed into deterministic_tree_reduce per §2.4.
StateHash territory_digest(const state::TerritoryState& t) noexcept {
    blake3_hasher h;
    blake3_hasher_init(&h);
    hash_double(h, t.r);
    hash_double(h, t.H);
    hash_double(h, t.G);
    hash_double(h, t.M);
    hash_double(h, t.Npop);
    hash_double(h, t.S);
    hash_double(h, t.Qelec);
    hash_double(h, t.Qliq);
    hash_double(h, t.D_local);
    hash_double(h, t.eta);
    hash_double(h, t.phi);
    hash_uint32(h, t.id);
    hash_uint32(h, t.owner);
    StateHash out;
    blake3_hasher_finalize(&h, out.bytes.data(), out.bytes.size());
    return out;
}

}  // namespace

// Combine two StateHash values into a parent. Used as the binary op in
// deterministic_tree_reduce. Pair-ordering is fixed by territory_id sort,
// so this is bit-deterministic. Lives at namespace scope (not anonymous) so
// IdentityFor<HashCombine> can be specialized.
struct HashCombine {
    StateHash operator()(const StateHash& a, const StateHash& b) const noexcept {
        blake3_hasher h;
        blake3_hasher_init(&h);
        blake3_hasher_update(&h, a.bytes.data(), a.bytes.size());
        blake3_hasher_update(&h, b.bytes.data(), b.bytes.size());
        StateHash out;
        blake3_hasher_finalize(&h, out.bytes.data(), out.bytes.size());
        return out;
    }
};

template <>
struct IdentityFor<HashCombine> {
    static const StateHash value;
};
inline const StateHash IdentityFor<HashCombine>::value = StateHash{};



StateHash canonical_hash(const state::WorldState& s) noexcept {
    // §3.5 + §9.9: hash WorldState in deterministic order.
    //   1. Per-territory digests in territory_id order (caller is expected to
    //      maintain TerritoryGrid in id-sorted order; integrator preserves it).
    //   2. Per-territory digests fed into deterministic_tree_reduce.
    //   3. Global scalars (D_global, A, S_AI_index) hashed at the end into a
    //      finalizer along with the tree-reduced grid root.

    const std::size_t N = s.grid.states.size();
    std::vector<StateHash> per_territory;
    per_territory.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        per_territory.push_back(territory_digest(s.grid.states[i]));
    }

    StateHash grid_root = deterministic_tree_reduce<StateHash, HashCombine>(
        std::span<const StateHash>(per_territory.data(), per_territory.size()),
        HashCombine{}
    );

    // Finalizer: combine grid_root with global scalars + tick.
    blake3_hasher h;
    blake3_hasher_init(&h);
    blake3_hasher_update(&h, grid_root.bytes.data(), grid_root.bytes.size());
    hash_uint64(h, s.tick);
    hash_double(h, s.time_years);
    hash_double(h, s.global.D_global);
    hash_double(h, s.global.A);
    hash_double(h, s.global.S_AI_index);
    StateHash final_hash;
    blake3_hasher_finalize(&h, final_hash.bytes.data(), final_hash.bytes.size());
    return final_hash;
}

StateHash hash_chain_link(const StateHash& prev, const StateHash& current) noexcept {
    // §10.1: hash-chain link Blake3-256(prev_hash || canonical_hash(s)).
    blake3_hasher h;
    blake3_hasher_init(&h);
    blake3_hasher_update(&h, prev.bytes.data(), prev.bytes.size());
    blake3_hasher_update(&h, current.bytes.data(), current.bytes.size());
    StateHash out;
    blake3_hasher_finalize(&h, out.bytes.data(), out.bytes.size());
    return out;
}

}  // namespace rc::sim::core
