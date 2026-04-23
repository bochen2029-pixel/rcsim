#pragma once

// §4.5: Gate functions — Theta_soft (k=50), Pi_hard (k=200).
// §4.2, §4.5: rho (Hill function), S_AI aggregation helpers.
// §6.4: k_detect = 3.0 for attribution logistic.

#include "rcsim/core/transcendentals.hpp"

namespace rc::sim::dynamics {

// §4.5: sharpness constants.
inline constexpr double kTheta_soft_k = 50.0;    // Q_elec availability, transition width ~0.04
inline constexpr double kPi_hard_k    = 200.0;   // Q_crit threshold, transition width ~0.01 (indicator-like)
inline constexpr double kDetect_k     = 3.0;     // §6.4 logistic sharpness for attribution

// §4.5: Theta_soft — soft sigmoid for Q_elec availability gates.
// TODO(phase 1, §4.5): implement via core::exp_canonical
//   return 1.0 / (1.0 + exp_canonical(-50.0 * x));
inline double Theta_soft(double x) noexcept {
    return 1.0 / (1.0 + core::exp_canonical(-kTheta_soft_k * x));
}

// §4.5: Pi_hard — indicator-like sigmoid for Q_crit thresholds.
// TODO(phase 1, §4.5): implement via core::exp_canonical
//   return 1.0 / (1.0 + exp_canonical(-200.0 * x));
inline double Pi_hard(double x) noexcept {
    return 1.0 / (1.0 + core::exp_canonical(-kPi_hard_k * x));
}

// §4.2, §4.5: rho — Hill function for S_AI-driven build rate.
// TODO(phase 1, §4.5): implement via core::exp_canonical + core::pow_canonical
double rho(double S, double S_half, double n, double rho_max) noexcept;

// §4.2: eta — derived quantity for TerritoryState.eta recomputation.
// SPEC_AMBIGUOUS(§4.2): the prose names eta and phi as "derived (rebuilt pre-integration)"
//   but does not pin a formula for them. Resolution deferred to phase 1.
// TODO(phase 1, §4.2): implement per DESIGN_v1.3.md once formula is pinned.
double eta(double r, double H, double G) noexcept;

// §4.2: phi — derived quantity for TerritoryState.phi recomputation.
// SPEC_AMBIGUOUS(§4.2): see eta above.
// TODO(phase 1, §4.2): implement per DESIGN_v1.3.md once formula is pinned.
double phi(double r, double H, double G) noexcept;

// §4.1 RC-4: e_U(nu) — residual employability multiplier in dM equation.
// SPEC_AMBIGUOUS(§4.1): e_U function form not pinned in spec (spec writes `e_U(\nu)`).
// TODO(phase 1, §4.1): implement per DESIGN_v1.3.md once formula is pinned.
double e_U(double nu) noexcept;

// §4.1 RC-1: xi_of_D — effective discovery-function for dr equation.
// SPEC_AMBIGUOUS(§4.1): xi function form not pinned in spec.
// TODO(phase 1, §4.1): implement per DESIGN_v1.3.md once formula is pinned.
double xi_of_D(double D_global) noexcept;

}  // namespace rc::sim::dynamics
