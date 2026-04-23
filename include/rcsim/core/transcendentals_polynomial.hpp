#pragma once

// §4.5: Polynomial alternative as primary for sigmoid arguments in [-50, 50].
// DESIGN_v1.3.md §4.5: Padé [6/6] polynomial approximation of exp; strictly bit-exact
// by construction under IEEE-754. Sleef is fallback for out-of-range values.
// This eliminates sleef from the hot path entirely for normal operation.

namespace rc::sim::core {

// §4.5: polynomial_exp_bitexact — Padé [6/6] approximation of exp(x).
// Valid for x in [-50, 50]. Strictly bit-exact across compilers under IEEE-754.
// SPEC_AMBIGUOUS(§4.5): Padé [6/6] coefficients not enumerated in spec.
//   They will be derived during phase 1 implementation and pinned.
// TODO(phase 1, §4.5): implement Padé [6/6] per DESIGN_v1.3.md §4.5
double polynomial_exp_bitexact(double x) noexcept;

// §4.5: Range check: returns true when polynomial path is valid.
inline constexpr double kPolynomialExpLowerBound = -50.0;
inline constexpr double kPolynomialExpUpperBound =  50.0;

inline constexpr bool in_polynomial_range(double x) noexcept {
    return x >= kPolynomialExpLowerBound && x <= kPolynomialExpUpperBound;
}

}  // namespace rc::sim::core
