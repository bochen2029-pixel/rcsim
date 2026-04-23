#pragma once

// §6.4.1: Cascade stability — what happens when legitimacy_S = 0.
// DESIGN_v1.3.md §6.4.1:
//   - legitimacy_S clamped to [0, initial_S]
//   - At legitimacy_S == 0: distortion lockout; cannot launch new ManipulateObservation
//   - Existing distortion budgets run to expiration; no further re-attribution debit
//   - Optional legitimacy_recovery_rate per-regime; default 0 (permanent consequence).
// §16.2 property test: distortion_cascade_bounded.cpp.

// Forward declaration — state::PrincipalState is authored in state/principal.hpp.
namespace rc::sim::state { struct PrincipalState; }

namespace rc::sim::observer {

// §6.4.1: clamp legitimacy_S to [0, initial_S].
// TODO(phase 3, §6.4.1): implement per DESIGN_v1.3.md §6.4.1
void clamp_legitimacy(state::PrincipalState& p) noexcept;

// §6.4.1: distortion_locked_out — true when principal cannot launch new ManipulateObservation.
//   Condition: legitimacy_S <= 0 (with tolerance on post-quantization comparisons).
// TODO(phase 3, §6.4.1): implement per DESIGN_v1.3.md §6.4.1
bool distortion_locked_out(const state::PrincipalState& p) noexcept;

// §6.4.1: optional per-regime legitimacy_recovery_rate. Default 0.
// TODO(phase 3, §6.4.1): implement per DESIGN_v1.3.md §6.4.1
void apply_legitimacy_recovery(state::PrincipalState& p, double dt_years) noexcept;

// §6.4.1: debit legitimacy on successful attribution. Floors at 0.
// TODO(phase 3, §6.4.1): implement per DESIGN_v1.3.md §6.4.1
void debit_legitimacy_for_attribution(
    state::PrincipalState& distorter,
    double attribution_magnitude
) noexcept;

}  // namespace rc::sim::observer
