#pragma once

// §6.1: Principals do not see WorldState.
// DESIGN_v1.3.md §6.1: Observation struct; ObservationOperator abstract class.
// Per-principal observation produced in phase 3 of Advance (post-action, pre-integration).

#include <cstdint>
#include <vector>

#include "rcsim/core/tick.hpp"
#include "rcsim/state/territory.hpp"

// Forward declarations — state/principal types and observer subtypes.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::observer {

// §6.2: ObservationField — which observable is being distorted / attributed.
// SPEC_AMBIGUOUS(§6.2): spec names observation fields but does not enumerate a closed
// set. Enum values frozen at schema_major per §10.1a enum rules.
enum class ObservationField : uint32_t {
    CPI             = 0,
    RealGDP         = 1,
    Unemployment    = 2,
    S_AI_index      = 3,
    // Physical telemetry (undistortable):
    MWhConsumed     = 4,
    RailTonMiles    = 5,
    CrudeExtracted  = 6
    // Values frozen at schema_major; additive-only within schema_minor.
};

// §6.1: Per-territory observation projection.
struct TerritoryObservation {
    state::TerritoryId id;
    // TODO(phase 3, §6.1): per-field visibility according to TelemetryAccess
    double r_reported;
    double G_reported;
    double Qelec_reported;
    double Qliq_reported;
    // SPEC_AMBIGUOUS(§6.1): exact field list not enumerated in spec; this is a starting set.
};

// §6.4: DistortionDetection — populated when attribution succeeds.
struct DistortionDetection {
    state::PrincipalId detected_actor;
    ObservationField field;
    core::Tick detected_at;
    // TODO(phase 3, §6.4): magnitude + confidence fields
};

// §6.1: Observation — what a principal sees each tick.
struct Observation {
    core::Tick as_of;
    double CPI_reported;
    double real_GDP_reported;
    double unemployment_reported;
    double S_AI_index_reported;

    // §6.3 "PhysicalTelemetryOnly" / undistortable physical telemetry.
    double MWh_consumed_local;
    double rail_ton_miles_local;
    double crude_extracted_local;

    std::vector<TerritoryObservation> territories_visible;
    std::vector<DistortionDetection> attributions_made;
};

// §3.4: ObservationCache — PrincipalState.last_seen holds one of these.
// Currently an alias; promotable to a richer cache (history) if phase 3 needs it.
using ObservationCache = Observation;

// Forward-declare DistortionInvestment + TelemetryAccess (both authored in companion headers).
struct DistortionInvestment;
struct TelemetryAccess;

// §6.1: ObservationOperator — abstract; policy selection per §6.3.
class ObservationOperator {
public:
    virtual ~ObservationOperator() = default;

    // §6.1: per-principal observation call.
    virtual Observation observe(
        const state::WorldState& truth,
        state::PrincipalId observer,
        const std::vector<DistortionInvestment>& incoming_distortions,
        const TelemetryAccess& access
    ) const = 0;
};

}  // namespace rc::sim::observer
