#pragma once

// §6.3: Four concrete ObservationOperator policies.
//   TruthfulMeasurement     — returns ground truth; debugging/baseline retrodiction.
//   HedonicHallucination    — CPI distorted by own and inbound investments; v5 default.
//   PhysicalTelemetryOnly   — only MWh, rail ton-miles, crude extracted; v5 §7.1 prescription.
//   AsymmetricTelemetry     — per-principal measurement access via TelemetryAccess; realistic retrodiction.

#include "rcsim/observer/asymmetric_telemetry.hpp"
#include "rcsim/observer/measurement.hpp"

namespace rc::sim::observer {

// §6.3: TruthfulMeasurement.
class TruthfulMeasurement : public ObservationOperator {
public:
    // TODO(phase 3, §6.3): return ground truth as Observation.
    Observation observe(
        const state::WorldState& truth,
        state::PrincipalId observer,
        const std::vector<DistortionInvestment>& incoming_distortions,
        const TelemetryAccess& access
    ) const override;
};

// §6.3: HedonicHallucination — v5 default.
class HedonicHallucination : public ObservationOperator {
public:
    // TODO(phase 3, §6.3): CPI distorted by own + inbound investments per DESIGN_v1.3.md §6.3
    Observation observe(
        const state::WorldState& truth,
        state::PrincipalId observer,
        const std::vector<DistortionInvestment>& incoming_distortions,
        const TelemetryAccess& access
    ) const override;
};

// §6.3: PhysicalTelemetryOnly — v5 §7.1 prescription.
class PhysicalTelemetryOnly : public ObservationOperator {
public:
    // TODO(phase 3, §6.3): return only MWh_consumed, rail_ton_miles, crude_extracted.
    Observation observe(
        const state::WorldState& truth,
        state::PrincipalId observer,
        const std::vector<DistortionInvestment>& incoming_distortions,
        const TelemetryAccess& access
    ) const override;
};

// §6.3: AsymmetricTelemetry — realistic retrodiction via per-principal TelemetryAccess.
class AsymmetricTelemetryPolicy : public ObservationOperator {
public:
    // SPEC_AMBIGUOUS(§6.3): spec uses "AsymmetricTelemetry" as both a policy type name and
    //   a per-principal access record type (TelemetryAccess). Disambiguated here by
    //   suffixing the policy with "Policy" — semantics unchanged.
    // TODO(phase 3, §6.3): apply TelemetryAccess mask + HedonicHallucination for CPI.
    Observation observe(
        const state::WorldState& truth,
        state::PrincipalId observer,
        const std::vector<DistortionInvestment>& incoming_distortions,
        const TelemetryAccess& access
    ) const override;
};

}  // namespace rc::sim::observer
