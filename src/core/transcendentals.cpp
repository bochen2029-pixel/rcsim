#include "rcsim/core/transcendentals.hpp"

#include "rcsim/core/transcendentals_polynomial.hpp"

// §4.5: exp_canonical — polynomial for [-50, 50]; Sleef_exp_u10 (pinned scalar) else.
// TODO(phase 1, §4.5): link against pinned sleef variant via CMake target.
// SIMD variants (Sleef_expd2_u10avx2, etc.) must NOT be linked — test 07 enforces.

namespace rc::sim::core {

double exp_canonical(double x) noexcept {
    if (in_polynomial_range(x)) {
        return polynomial_exp_bitexact(x);
    }
    // TODO(phase 1, §4.5): return Sleef_exp_u10(x);
    return 0.0;
}

double pow_canonical(double /*base*/, double /*exponent*/) noexcept {
    // TODO(phase 1, §4.5): pow via exp_canonical(exponent * log_canonical(base))
    return 0.0;
}

double log_canonical(double /*x*/) noexcept {
    // TODO(phase 1, §4.5): pinned sleef scalar log + polynomial accelerator.
    return 0.0;
}

}  // namespace rc::sim::core
