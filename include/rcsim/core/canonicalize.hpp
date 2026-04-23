#pragma once

// §2.6: Signed-zero canonicalization + NaN/Inf trap.
// §2.7: Kahan-style quantization residual buffer.
// §9.7: Canonicalize state (Q-format quantization + Kahan residual buffer + signed-zero sweep).

#include "rcsim/core/fixed_point.hpp"

namespace rc::sim::state {
struct WorldState;
struct CanonicalizationState;
}

namespace rc::sim::core {

// §2.6: Canonicalization sweep applied immediately before hashing.
//   if (v == 0.0) v = 0.0;                     // strips sign bit on signed zeros
//   if (!std::isfinite(v)) trap_nonfinite(v);  // hard halt with diagnostic
//   return v;
// TODO(phase 1, §2.6): implement per DESIGN_v1.3.md §2.6
double canonicalize_for_hash(double v) noexcept;

// §2.6: NaN/Inf trap. Halts with field-identification for forensic analysis.
// [[noreturn]] by intent but not marked to let stub compile without call-site fanfare;
// actual implementation uses std::abort or platform-specific halt.
// TODO(phase 1, §2.6): implement per DESIGN_v1.3.md §2.6
void trap_nonfinite(double v) noexcept;

// §2.7: canonicalize_state_with_residual — full tick-boundary canonicalization pass.
//   For each field, per territory:
//     double augmented = unquantized + previous_residual;
//     double quantized = quantize_to_Q_format(augmented);
//     double new_residual = augmented - quantized;
//   quantized enters canonical state (hashed); new_residual stored in sidecar (not hashed).
// TODO(phase 1, §2.7): implement per DESIGN_v1.3.md §2.7
void canonicalize_state_with_residual(state::WorldState& s) noexcept;

}  // namespace rc::sim::core
