#include "rcsim/core/canonicalize.hpp"

#include "rcsim/state/world_state.hpp"

// §2.6: signed-zero canonicalization + NaN/Inf trap.
// §2.7: Kahan-style quantization residual buffer at tick boundary.

namespace rc::sim::core {

double canonicalize_for_hash(double v) noexcept {
    // TODO(phase 1, §2.6):
    //   if (v == 0.0) v = 0.0;                   // strips sign bit
    //   if (!std::isfinite(v)) trap_nonfinite(v);
    //   return v;
    return v;
}

void trap_nonfinite(double /*v*/) noexcept {
    // TODO(phase 1, §2.6): halt with field-identification for forensic analysis.
    //   Implementation: std::abort or platform halt after emitting diagnostic.
}

void canonicalize_state_with_residual(state::WorldState& /*s*/) noexcept {
    // TODO(phase 1, §2.7): per-field, per-territory Kahan residual quantization.
    //   double augmented = unquantized + previous_residual;
    //   double quantized = quantize_to_Q_format(augmented);
    //   double new_residual = augmented - quantized;
    //   state := quantized, sidecar := new_residual.
}

}  // namespace rc::sim::core
