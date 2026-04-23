#pragma once

// §7.2: PoliticalClock — per-principal response-latency state.
// DESIGN_v1.3.md §7.2: tau_modal + tau_variance from gamma-fit to historical response data.

#include "rcsim/core/tick.hpp"
#include "rcsim/political/regime.hpp"

namespace rc::sim::political {

// §7.2: PoliticalClock.
struct PoliticalClock {
    PoliticalRegime regime;
    core::Tick next_election_or_succession;

    double regime_survival_pressure;
    double incumbent_tenure_years;

    // §7.2: REQUIRED from gamma-fit to historical response data (phase 7b).
    double tau_modal;
    double tau_variance;
    double crisis_threshold;

    // §7.5: endogenous-transition thresholds.
    double q_collapse_threshold;
    core::Tick collapse_pressure_duration;
    double survival_pressure_limit;
};

}  // namespace rc::sim::political
