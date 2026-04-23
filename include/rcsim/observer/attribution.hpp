#pragma once

// §6.4: Probabilistic attribution — closes Bang-Bang Exploit (v1.3 critical fix).
// DESIGN_v1.3.md §6.4: deterministic threshold → RL binary-search produces bang-bang policies.
// Fix: probabilistic evaluation via seeded rng_observer + logistic smoothing, k_detect ~ 3.0.
// Maintains bit-determinism; smooths game-theoretic gradient.

#include "rcsim/core/rng.hpp"
#include "rcsim/core/tick.hpp"
#include "rcsim/observer/distortion.hpp"
#include "rcsim/observer/measurement.hpp"
#include "rcsim/state/territory.hpp"

// Forward declaration — avoid circular include with principal.hpp.
namespace rc::sim::state { struct PrincipalState; }

namespace rc::sim::observer {

// §6.4: AttributionInvestment — tracks a pending attribution attempt over its duration.
struct AttributionInvestment {
    state::PrincipalId attributer;
    state::PrincipalId accused;
    ObservationField field;
    double investment_M;
    core::Tick activates_at;
    core::Tick expires_at;
};

// §6.4: AttributeDistortion action-level struct is declared in action/action.hpp.
// Here we declare the evaluation path used during apply.

// §6.4: detection_efficacy — per-attributer multiplier on signal.
// TODO(phase 3, §6.4): implement per DESIGN_v1.3.md §6.4
double detection_efficacy(const state::PrincipalState& attributer) noexcept;

// §6.4: evaluate_attribution — probabilistic, RNG-seeded.
//   signal = attr.investment_M * detection_efficacy(attributer)
//          - target_distortion.magnitude * concealment_factor(target_distortion);
//   p_detect = 1.0 / (1.0 + exp_canonical(-k_detect * signal));
//   draw = rng_observer.next_double();
//   return draw < p_detect;
// TODO(phase 3, §6.4): implement per DESIGN_v1.3.md §6.4
bool evaluate_attribution(
    const struct AttributeDistortionRef& attr,   // wraps action::AttributeDistortion
    const DistortionInvestment& target_distortion,
    const state::PrincipalState& attributer,
    core::Pcg64& rng_observer
) noexcept;

// Thin by-reference wrapper so attribution.hpp does not pull in action/action.hpp.
// SPEC_AMBIGUOUS(§6.4): spec shows evaluate_attribution taking action::AttributeDistortion
//   by const-ref; that creates a circular dependency (action → observer → action).
//   Break via this ref struct; fields match action::AttributeDistortion 1:1.
struct AttributeDistortionRef {
    state::PrincipalId accused;
    ObservationField field;
    double investment_M;
    core::Tick duration;
};

}  // namespace rc::sim::observer
