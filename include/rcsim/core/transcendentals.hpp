#pragma once

// §4.5: Gate functions — pinned sleef variant + differentiated sharpness.
// DESIGN_v1.3.md §4.5:
//   - Pinned: Sleef_exp_u10 (scalar, double-precision, 1ULP).
//   - SIMD variants (Sleef_expd2_u10avx2, Sleef_expd4_u10avx512f, etc.) explicitly NOT used.
//   - Polynomial alternative as primary for inputs in [-50, 50]; sleef as fallback.
//   - CI test 07 verifies only pinned scalar variant is called.

namespace rc::sim::core {

// §4.5: exp_canonical — polynomial for [-50, 50]; pinned Sleef_exp_u10 else.
// TODO(phase 1, §4.5): implement per DESIGN_v1.3.md §4.5
//   if (x >= -50.0 && x <= 50.0) return polynomial_exp_bitexact(x);
//   return Sleef_exp_u10(x);
double exp_canonical(double x) noexcept;

// §4.5: pow_canonical — (typical usage: pow(S, n) for Hill function).
// TODO(phase 1, §4.5): implement via exp_canonical + log_canonical composition to avoid libm pow.
double pow_canonical(double base, double exponent) noexcept;

// §4.5: log_canonical — companion to exp_canonical.
// TODO(phase 1, §4.5): implement via pinned sleef scalar variant + polynomial for common ranges.
double log_canonical(double x) noexcept;

}  // namespace rc::sim::core
