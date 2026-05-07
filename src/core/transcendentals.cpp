#include "rcsim/core/transcendentals.hpp"

#include <cmath>

#include "rcsim/core/transcendentals_polynomial.hpp"

// §4.5: exp_canonical — polynomial for [-50, 50]; Sleef_exp_u10 (pinned scalar) else.
// SIMD variants (Sleef_expd2_u10avx2, etc.) must NOT be linked — test 07 enforces.
//
// Sleef integration: the C symbol `Sleef_exp_u10` is exported by libsleef when
// linked statically. We declare it directly to avoid pulling sleef.h into this
// translation unit (which would risk SIMD-variant macro pollution).

extern "C" {
double Sleef_exp_u10(double x);
double Sleef_log_u10(double x);
}  // extern "C"

namespace rc::sim::core {

double exp_canonical(double x) noexcept {
    if (in_polynomial_range(x)) {
        return polynomial_exp_bitexact(x);
    }
    return Sleef_exp_u10(x);
}

double log_canonical(double x) noexcept {
    // Pinned sleef scalar variant (1ULP). No polynomial accelerator in v1 —
    // log is rarely on the hot path (RK4 + gates dominate; pow_canonical is
    // the only consumer, and it bottoms out in exp_canonical for the common case).
    return Sleef_log_u10(x);
}

double pow_canonical(double base, double exponent) noexcept {
    // Standard identity: a^b = exp(b * log(a)).
    // Edge cases (base == 0, base < 0, exponent == 0, exponent integer) handled
    // explicitly so the simulation hot path never sees a NaN from log(0) or log(-x).
    if (base == 0.0) {
        return exponent == 0.0 ? 1.0 : 0.0;
    }
    if (exponent == 0.0) {
        return 1.0;
    }
    if (base < 0.0) {
        // Negative base only valid for integer exponent. The ODE callers only
        // raise non-negative quantities (S, G, etc.) to non-integer powers,
        // so this branch is a guard against bugs upstream.
        double rounded = std::nearbyint(exponent);
        if (rounded == exponent) {
            double mag = pow_canonical(-base, exponent);
            int64_t k = static_cast<int64_t>(rounded);
            return (k & 1) ? -mag : mag;
        }
        // Non-integer exponent on negative base — return NaN to surface upstream.
        return std::nan("rcsim_pow_canonical_neg_base");
    }
    return exp_canonical(exponent * log_canonical(base));
}

}  // namespace rc::sim::core
