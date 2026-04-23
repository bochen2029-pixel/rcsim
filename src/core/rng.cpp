#include "rcsim/core/rng.hpp"

// TODO(phase 1, §2.3): implement PCG64 wrapper + Philox4x32-10 + substream-seed derivation.

namespace rc::sim::core {

Pcg64::Pcg64(uint64_t /*seed*/) noexcept {
    // TODO(phase 1, §2.3)
}

Pcg64::Pcg64(uint64_t /*scenario_seed*/, RngSubstream /*substream*/) noexcept {
    // TODO(phase 1, §2.3)
}

uint64_t Pcg64::next_uint64() noexcept {
    // TODO(phase 1, §2.3)
    return 0;
}

double Pcg64::next_double() noexcept {
    // TODO(phase 1, §2.3)
    return 0.0;
}

std::array<uint32_t, 4> Philox4x32::generate(
    std::array<uint32_t, 4> /*key*/,
    std::array<uint32_t, 4> /*counter*/
) const noexcept {
    // TODO(phase 1, §2.3)
    return {};
}

std::array<uint32_t, 4> Philox4x32::make_counter(
    uint64_t /*tick*/, uint32_t /*territory_id*/, uint32_t /*purpose_tag*/
) noexcept {
    // TODO(phase 1, §2.3)
    return {};
}

double Philox4x32::to_double(std::array<uint32_t, 4> /*output*/) noexcept {
    // TODO(phase 1, §2.3)
    return 0.0;
}

uint64_t derive_substream_seed(uint64_t /*scenario_seed*/, RngSubstream /*substream*/) noexcept {
    // TODO(phase 1, §2.3): Blake3-256 truncation via core/hash.hpp
    return 0;
}

}  // namespace rc::sim::core
