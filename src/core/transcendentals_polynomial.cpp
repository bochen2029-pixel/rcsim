#include "rcsim/core/transcendentals_polynomial.hpp"

#include <cmath>

// §4.5: Padé [6/6] polynomial approximation of exp(x), bit-exact across compilers
// under strict IEEE-754. R-08 in docs/SPEC_AMBIGUOUS_RESOLUTIONS.md.
//
// Form:
//   even(r) = 1 + (5/44) r²  + (1/792) r⁴   + (1/665280) r⁶
//   odd(r)  =     (1/2)  r   + (1/66)  r³   + (1/15840)  r⁵
//   exp(r)  ≈ (even + odd) / (even - odd)
//
// Range reduction (for x in [-50, 50]):
//   k = nearbyint(x / ln(2));  r = x - k * ln(2);   |r| ≤ ln(2)/2 ≈ 0.347
//   exp(x) = ldexp(exp(r), k)
//
// Determinism: every operation is single-rounding under /fp:strict and
// -ffp-contract=off; multiply-add is NOT fused. Horner-on-r² and ldexp via
// integer-exponent manipulation are bit-identical across compilers.

namespace rc::sim::core {

namespace {

constexpr double kP_e_c2 = 5.0 / 44.0;
constexpr double kP_e_c4 = 1.0 / 792.0;
constexpr double kP_e_c6 = 1.0 / 665280.0;

constexpr double kP_o_c1 = 1.0 / 2.0;
constexpr double kP_o_c3 = 1.0 / 66.0;
constexpr double kP_o_c5 = 1.0 / 15840.0;

constexpr double kLn2    = 0.6931471805599453;     // 0x3FE62E42FEFA39EF
constexpr double kInvLn2 = 1.4426950408889634;     // 0x3FF71547652B82FE

}  // namespace

double polynomial_exp_bitexact(double x) noexcept {
    double k_d = std::nearbyint(x * kInvLn2);
    double r = x - k_d * kLn2;

    double r2 = r * r;
    double pe = ((kP_e_c6 * r2 + kP_e_c4) * r2 + kP_e_c2) * r2 + 1.0;
    double po = r * ((kP_o_c5 * r2 + kP_o_c3) * r2 + kP_o_c1);

    double exp_r = (pe + po) / (pe - po);
    return std::ldexp(exp_r, static_cast<int>(k_d));
}

}  // namespace rc::sim::core
