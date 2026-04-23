#include "rcsim/core/fixed_point.hpp"

// TODO(phase 1, §2.2): implement per-field Q-format quantize/dequantize per DESIGN_v1.3.md §2.2.

namespace rc::sim::core {

Q32_32 quantize_Q32_32(double /*v*/) noexcept {
    // TODO(phase 1, §2.2)
    return 0;
}

Q48_16 quantize_Q48_16(double /*v*/) noexcept {
    // TODO(phase 1, §2.2)
    return 0;
}

Q56_8 quantize_Q56_8(double /*v*/) noexcept {
    // TODO(phase 1, §2.2)
    return 0;
}

double dequantize_Q32_32(Q32_32 /*q*/) noexcept {
    // TODO(phase 1, §2.2)
    return 0.0;
}

double dequantize_Q48_16(Q48_16 /*q*/) noexcept {
    // TODO(phase 1, §2.2)
    return 0.0;
}

double dequantize_Q56_8(Q56_8 /*q*/) noexcept {
    // TODO(phase 1, §2.2)
    return 0.0;
}

double saturation_ratio_Q32_32(double /*v*/) noexcept {
    // TODO(phase 1, §8.3)
    return 0.0;
}

double saturation_ratio_Q48_16(double /*v*/) noexcept {
    // TODO(phase 1, §8.3)
    return 0.0;
}

double saturation_ratio_Q56_8(double /*v*/) noexcept {
    // TODO(phase 1, §8.3)
    return 0.0;
}

}  // namespace rc::sim::core
