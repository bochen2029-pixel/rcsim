#pragma once

// §2.2: Per-field fixed-point scaling with EJ/yr units for energy.
// DESIGN_v1.3.md §2.2:
//   r      Q32.32  dimensionless    [1.0, ~10^9]
//   H      Q48.16  persons          [0, ~10^12]
//   G      Q48.16  HE-units         [0, ~10^12]
//   M      Q56.8   USD-eq           [0, ~10^17]
//   Npop   Q48.16  persons          [0, ~10^12]
//   S      Q32.32  dimensionless    [0, ~10^9]
//   Qelec  Q32.32  EJ/yr            [0, ~1000]
//   Qliq   Q32.32  EJ/yr            [0, ~1000]
//   D      Q48.16  accumulator      [0, ~10^12]
//   A      Q32.32  dimensionless    [1.0, ~10^9]

#include <cstdint>

namespace rc::sim::core {

// §2.2: Per-field Q-format integer types (64-bit signed, split integer.fraction).
using Q32_32 = int64_t;   // 32 integer bits, 32 fractional bits
using Q48_16 = int64_t;   // 48 integer bits, 16 fractional bits
using Q56_8  = int64_t;   // 56 integer bits,  8 fractional bits

// §2.2: Scale factors (shift amounts for fractional-bit count).
inline constexpr int kQ32_32_FractionalBits = 32;
inline constexpr int kQ48_16_FractionalBits = 16;
inline constexpr int kQ56_8_FractionalBits  = 8;

// §2.2: Per-field saturation thresholds (as described in §8.3).
inline constexpr double kSaturationWarning = 0.95;
inline constexpr double kSaturationHalt    = 1.00;

// §2.2: quantize() — double → Q-format integer at per-field scale.
// TODO(phase 1, §2.2): implement per DESIGN_v1.3.md §2.2
Q32_32 quantize_Q32_32(double v) noexcept;
Q48_16 quantize_Q48_16(double v) noexcept;
Q56_8  quantize_Q56_8(double v) noexcept;

// §2.2: dequantize() — Q-format integer → double.
// TODO(phase 1, §2.2): implement per DESIGN_v1.3.md §2.2
double dequantize_Q32_32(Q32_32 q) noexcept;
double dequantize_Q48_16(Q48_16 q) noexcept;
double dequantize_Q56_8 (Q56_8  q) noexcept;

// §8.3: Per-field saturation check (after integration, before canonicalization).
// Returns ratio of field-value to Q-format max.
// TODO(phase 1, §8.3): implement per DESIGN_v1.3.md §8.3
double saturation_ratio_Q32_32(double v) noexcept;
double saturation_ratio_Q48_16(double v) noexcept;
double saturation_ratio_Q56_8 (double v) noexcept;

}  // namespace rc::sim::core
