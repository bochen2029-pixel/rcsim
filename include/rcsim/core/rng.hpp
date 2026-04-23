#pragma once

// §2.3: One PCG64 RNG substream per concern, namespace-separated.
// DESIGN_v1.3.md §2.3: Seven substreams derived from (scenario_seed, substream_id) via Blake3-256 truncation.
// Per-territory RNG via Philox-4x32-10 counter-based call keyed on (substream_state, tick, territory_id, purpose_tag).

#include <cstdint>
#include <array>

namespace rc::sim::core {

// §2.3: Substream identifiers. Values frozen per scheme_major per §10.1a enum rules.
enum class RngSubstream : uint32_t {
    Shocks       = 0,
    Supply       = 1,
    Geopolitical = 2,
    Demographic  = 3,
    Discovery    = 4,
    Market       = 5,
    Observer     = 6
};

// §2.3: PCG64 generator wrapper. Wraps pcg_cpp pcg64 or equivalent.
class Pcg64 {
public:
    // TODO(phase 1, §2.3): implement seeded constructor from (scenario_seed, substream_id)
    Pcg64() noexcept = default;
    explicit Pcg64(uint64_t seed) noexcept;
    Pcg64(uint64_t scenario_seed, RngSubstream substream) noexcept;

    // TODO(phase 1, §2.3): return next 64-bit integer; advances state.
    uint64_t next_uint64() noexcept;

    // TODO(phase 1, §2.3): return next double in [0.0, 1.0); uses next_uint64 with canonical conversion.
    double next_double() noexcept;

    // Internal state (opaque; 128 bits for PCG64). Not serialized directly;
    // sync log emits (scenario_seed, substream_id) and reconstructs.
    // TODO(phase 1, §2.3): populate per pcg_cpp implementation
    uint64_t state_ = 0;
    uint64_t inc_   = 0;
};

// §2.3: Philox-4x32-10 counter-based RNG, keyed.
// Inputs: (substream_state, tick, territory_id, purpose_tag) → 128-bit counter.
class Philox4x32 {
public:
    // TODO(phase 1, §2.3): implement Philox-4x32-10 counter-based generation.
    // No state across calls — purely a function of (key, counter).
    std::array<uint32_t, 4> generate(
        std::array<uint32_t, 4> key,
        std::array<uint32_t, 4> counter
    ) const noexcept;

    // Helper: derive counter from (tick, territory_id, purpose_tag).
    // TODO(phase 1, §2.3): canonicalize counter layout for reproducibility.
    static std::array<uint32_t, 4> make_counter(
        uint64_t tick, uint32_t territory_id, uint32_t purpose_tag
    ) noexcept;

    // Helper: draw single double in [0, 1) from key+counter.
    // TODO(phase 1, §2.3): implement canonical double conversion.
    static double to_double(std::array<uint32_t, 4> output) noexcept;
};

// §2.3: Derive per-substream PCG64 seed from (scenario_seed, substream_id) via Blake3-256 truncation.
// TODO(phase 1, §2.3): implement using core/hash.hpp Blake3-256.
uint64_t derive_substream_seed(uint64_t scenario_seed, RngSubstream substream) noexcept;

}  // namespace rc::sim::core
