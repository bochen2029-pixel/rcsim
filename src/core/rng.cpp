#include "rcsim/core/rng.hpp"

// §2.3: PCG64 + Philox-4x32-10 + substream-seed derivation via Blake3-256.
//
// PCG64 is wrapped from pcg-cpp's pcg64. Bit-exactness across compilers is
// guaranteed by pcg-cpp's portable 128-bit arithmetic (no __uint128_t reliance
// — it falls back to a portable two-uint64 path on MSVC).
//
// Philox-4x32-10 is implemented inline (Salmon et al. 2011).
//
// Blake3 is supplied by FetchContent — declared via extern "C" to avoid pulling
// the C header here. CI test 03_cross_platform_hash pins the byte stream.

extern "C" {
struct blake3_hasher {
    uint8_t opaque[1912];   // upper bound; layout owned by blake3.h
};
void blake3_hasher_init(blake3_hasher* self);
void blake3_hasher_update(blake3_hasher* self, const void* input, size_t input_len);
void blake3_hasher_finalize(const blake3_hasher* self, uint8_t* out, size_t out_len);
}  // extern "C"

namespace rc::sim::core {

Pcg64::Pcg64(uint64_t seed) noexcept : engine_(seed) {}

Pcg64::Pcg64(uint64_t scenario_seed, RngSubstream substream) noexcept
    : engine_(derive_substream_seed(scenario_seed, substream)) {}

uint64_t Pcg64::next_uint64() noexcept {
    return engine_();
}

double Pcg64::next_double() noexcept {
    return (next_uint64() >> 11) * (1.0 / static_cast<double>(1ULL << 53));
}

namespace {
constexpr uint32_t kPhiloxW0 = 0x9E3779B9u;
constexpr uint32_t kPhiloxW1 = 0xBB67AE85u;
constexpr uint32_t kPhiloxM0 = 0xD2511F53u;
constexpr uint32_t kPhiloxM1 = 0xCD9E8D57u;

inline void philox_round(std::array<uint32_t, 4>& ctr, std::array<uint32_t, 2>& key) noexcept {
    uint64_t p0 = static_cast<uint64_t>(kPhiloxM0) * ctr[0];
    uint64_t p1 = static_cast<uint64_t>(kPhiloxM1) * ctr[2];
    uint32_t hi0 = static_cast<uint32_t>(p0 >> 32);
    uint32_t lo0 = static_cast<uint32_t>(p0);
    uint32_t hi1 = static_cast<uint32_t>(p1 >> 32);
    uint32_t lo1 = static_cast<uint32_t>(p1);
    std::array<uint32_t, 4> out = {
        hi1 ^ ctr[1] ^ key[0],
        lo1,
        hi0 ^ ctr[3] ^ key[1],
        lo0
    };
    ctr = out;
    key[0] += kPhiloxW0;
    key[1] += kPhiloxW1;
}
}  // namespace

std::array<uint32_t, 4> Philox4x32::generate(
    std::array<uint32_t, 4> key_in,
    std::array<uint32_t, 4> counter
) const noexcept {
    std::array<uint32_t, 2> key = {key_in[0] ^ key_in[2], key_in[1] ^ key_in[3]};
    for (int round = 0; round < 10; ++round) {
        philox_round(counter, key);
    }
    return counter;
}

std::array<uint32_t, 4> Philox4x32::make_counter(
    uint64_t tick, uint32_t territory_id, uint32_t purpose_tag
) noexcept {
    return {
        static_cast<uint32_t>(tick & 0xFFFFFFFFu),
        static_cast<uint32_t>(tick >> 32),
        territory_id,
        purpose_tag
    };
}

double Philox4x32::to_double(std::array<uint32_t, 4> output) noexcept {
    uint64_t u = (static_cast<uint64_t>(output[0]) << 32) | output[1];
    return (u >> 11) * (1.0 / static_cast<double>(1ULL << 53));
}

uint64_t derive_substream_seed(uint64_t scenario_seed, RngSubstream substream) noexcept {
    // §2.3: Blake3-256(seed_le || substream_id_le), truncated to 64 bits.
    blake3_hasher h;
    blake3_hasher_init(&h);
    uint8_t seed_le[8];
    for (int i = 0; i < 8; ++i) {
        seed_le[i] = static_cast<uint8_t>((scenario_seed >> (8 * i)) & 0xFFu);
    }
    blake3_hasher_update(&h, seed_le, sizeof(seed_le));
    uint32_t sub = static_cast<uint32_t>(substream);
    uint8_t sub_le[4];
    for (int i = 0; i < 4; ++i) {
        sub_le[i] = static_cast<uint8_t>((sub >> (8 * i)) & 0xFFu);
    }
    blake3_hasher_update(&h, sub_le, sizeof(sub_le));
    uint8_t out[32];
    blake3_hasher_finalize(&h, out, sizeof(out));
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= static_cast<uint64_t>(out[i]) << (8 * i);
    }
    return result;
}

}  // namespace rc::sim::core
