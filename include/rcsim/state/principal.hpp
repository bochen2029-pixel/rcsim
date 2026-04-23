#pragma once

// §3.4: Principal state. Nation-state actor; owns territories; acts via typed Action variant.
// §6.2: ManipulateObservation debits treasury_M and legitimacy_S.
// §6.3: TelemetryAccess per-principal measurement.
// §6.4.1: legitimacy_S bounded [0, initial_S] with distortion lockout at 0.
// §7: PoliticalRegime + PoliticalClock gate action validation.

#include <string>
#include <vector>

#include "rcsim/state/territory.hpp"
#include "rcsim/political/regime.hpp"
#include "rcsim/political/clock.hpp"
#include "rcsim/observer/distortion.hpp"
#include "rcsim/observer/attribution.hpp"
#include "rcsim/observer/measurement.hpp"
#include "rcsim/observer/asymmetric_telemetry.hpp"
#include "rcsim/action/action.hpp"

namespace rc::sim::state {

// §3.4: PrincipalState — value storage per spec.
struct PrincipalState {
    PrincipalId id;
    std::string name;
    std::vector<TerritoryId> owned;

    // §3.4, §6.4.1: economic + legitimacy reserves.
    double treasury_M;            // USD-eq
    double legitimacy_S;          // S-reserve; depleted by ManipulateObservation; floored at 0
    double deployment_fraction_A; // A split share
    double observation_budget;    // forensics / observation capacity

    // §7: regime + clock.
    political::PoliticalRegime regime;
    political::PoliticalClock clock;

    // §6.2: outbound distortion + attribution investments.
    std::vector<observer::DistortionInvestment> outbound_distortion;
    std::vector<observer::AttributionInvestment> outbound_attribution;

    // §6.1, §6.3: last observation + telemetry access.
    observer::ObservationCache last_seen;
    observer::TelemetryAccess telemetry_access;

    // §5.1, §9.2: pending actions queued for next Advance().
    std::vector<action::Action> pending_actions;
};

}  // namespace rc::sim::state
