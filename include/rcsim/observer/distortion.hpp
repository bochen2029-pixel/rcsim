#pragma once

// §6.2: Distortion as first-class action — with M and S cost.
// DESIGN_v1.3.md §6.2: S-cost bounded by legitimacy_S; excess truncated at deduction time.
// Principal at legitimacy_S == 0 cannot launch new distortion (validated epistemically).

#include "rcsim/core/tick.hpp"
#include "rcsim/observer/measurement.hpp"
#include "rcsim/state/territory.hpp"

namespace rc::sim::observer {

// §6.2: DistortionInvestment — active distortion against a target observer.
//   Populated by ManipulateObservation apply(); stored in actor's outbound_distortion
//   and referenced per-target during observe().
struct DistortionInvestment {
    state::PrincipalId actor;          // who emitted the distortion
    state::PrincipalId target_observer;
    ObservationField field;
    double magnitude;
    double investment_M_remaining;     // decays / depletes by concealment
    double investment_S_committed;     // debited from legitimacy_S at deduction time
    core::Tick activates_at;
    core::Tick expires_at;
};

// §6.4: concealment_factor — scales distortion against attribution signal.
// TODO(phase 3, §6.4): implement per DESIGN_v1.3.md §6.4
double concealment_factor(const DistortionInvestment& d) noexcept;

// §6.2: distortion-apply helper — debits treasury_M + legitimacy_S with S-floor.
//   Excess investment_S is truncated to available legitimacy at deduction time.
// TODO(phase 3, §6.2): implement per DESIGN_v1.3.md §6.2
void apply_distortion_costs(
    state::PrincipalId actor,
    double investment_M,
    double investment_S,
    double& treasury_M,
    double& legitimacy_S
) noexcept;

}  // namespace rc::sim::observer
