#include "rcsim/observer/operator_policies.hpp"

// §6.3: Four concrete observer policies.

namespace rc::sim::observer {

Observation TruthfulMeasurement::observe(
    const state::WorldState& /*truth*/,
    state::PrincipalId /*observer*/,
    const std::vector<DistortionInvestment>& /*incoming_distortions*/,
    const TelemetryAccess& /*access*/
) const {
    // TODO(phase 3, §6.3): return ground truth as Observation.
    return {};
}

Observation HedonicHallucination::observe(
    const state::WorldState& /*truth*/,
    state::PrincipalId /*observer*/,
    const std::vector<DistortionInvestment>& /*incoming_distortions*/,
    const TelemetryAccess& /*access*/
) const {
    // TODO(phase 3, §6.3): CPI distorted by own + inbound investments.
    return {};
}

Observation PhysicalTelemetryOnly::observe(
    const state::WorldState& /*truth*/,
    state::PrincipalId /*observer*/,
    const std::vector<DistortionInvestment>& /*incoming_distortions*/,
    const TelemetryAccess& /*access*/
) const {
    // TODO(phase 3, §6.3): MWh, rail ton-miles, crude extracted only.
    return {};
}

Observation AsymmetricTelemetryPolicy::observe(
    const state::WorldState& /*truth*/,
    state::PrincipalId /*observer*/,
    const std::vector<DistortionInvestment>& /*incoming_distortions*/,
    const TelemetryAccess& /*access*/
) const {
    // TODO(phase 3, §6.3): TelemetryAccess mask + HedonicHallucination CPI.
    return {};
}

}  // namespace rc::sim::observer
