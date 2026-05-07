#include "rcsim/core/fixed_point.hpp"

#include <cmath>
#include <cstdint>
#include <limits>

// §2.2: Per-field Q-format quantize/dequantize.
//
// Determinism note: under /fp:strict (MSVC) and -ffp-contract=off -fno-fast-math
// -fno-associative-math (GCC/Clang), the multiply-by-scale and the cast-to-int64
// are bit-identical across compilers. std::nearbyint honors current rounding
// mode; default is FE_TONEAREST (round-to-nearest-ties-to-even), which is
// the IEEE-754 default and what we require.

namespace rc::sim::core {

namespace {

constexpr double kScale_Q32_32 = static_cast<double>(1ULL << 32);          // 2^32
constexpr double kScale_Q48_16 = static_cast<double>(1ULL << 16);          // 2^16
constexpr double kScale_Q56_8  = static_cast<double>(1ULL << 8);           // 2^8

// §2.2 ranges (max representable, signed int64):
//   Q32.32 max ≈ 2^31 in integer-units after scale → 2^31 / 2^32 = 0.5 * 2^32 / 2^32
//   Actually: int64 max / 2^32 ≈ 2.147e9
constexpr double kMax_Q32_32 = static_cast<double>(std::numeric_limits<int64_t>::max()) / kScale_Q32_32;
constexpr double kMax_Q48_16 = static_cast<double>(std::numeric_limits<int64_t>::max()) / kScale_Q48_16;
constexpr double kMax_Q56_8  = static_cast<double>(std::numeric_limits<int64_t>::max()) / kScale_Q56_8;

inline int64_t quantize_with_scale(double v, double scale) noexcept {
    // Round-to-nearest-ties-to-even per IEEE-754 default; deterministic under strict FP.
    double scaled = v * scale;
    double rounded = std::nearbyint(scaled);
    return static_cast<int64_t>(rounded);
}

}  // namespace

Q32_32 quantize_Q32_32(double v) noexcept {
    return quantize_with_scale(v, kScale_Q32_32);
}

Q48_16 quantize_Q48_16(double v) noexcept {
    return quantize_with_scale(v, kScale_Q48_16);
}

Q56_8 quantize_Q56_8(double v) noexcept {
    return quantize_with_scale(v, kScale_Q56_8);
}

double dequantize_Q32_32(Q32_32 q) noexcept {
    return static_cast<double>(q) / kScale_Q32_32;
}

double dequantize_Q48_16(Q48_16 q) noexcept {
    return static_cast<double>(q) / kScale_Q48_16;
}

double dequantize_Q56_8(Q56_8 q) noexcept {
    return static_cast<double>(q) / kScale_Q56_8;
}

double saturation_ratio_Q32_32(double v) noexcept {
    double abs_v = v < 0.0 ? -v : v;
    return abs_v / kMax_Q32_32;
}

double saturation_ratio_Q48_16(double v) noexcept {
    double abs_v = v < 0.0 ? -v : v;
    return abs_v / kMax_Q48_16;
}

double saturation_ratio_Q56_8(double v) noexcept {
    double abs_v = v < 0.0 ? -v : v;
    return abs_v / kMax_Q56_8;
}

}  // namespace rc::sim::core
