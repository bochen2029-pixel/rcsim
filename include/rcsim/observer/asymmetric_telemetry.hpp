#pragma once

// §6.3: AsymmetricTelemetry — per-principal measurement access policy.
// DESIGN_v1.3.md §6.3: Four operator policies; AsymmetricTelemetry is the fourth.
// Per-principal TelemetryAccess stored in PrincipalState.

#include <cstdint>
#include <unordered_map>

#include "rcsim/observer/measurement.hpp"
#include "rcsim/state/territory.hpp"

namespace rc::sim::observer {

// §6.3: TelemetryAccess — per-principal, per-territory, per-field access mask.
// Controls what each principal can measure; enables realistic retrodiction.
// SPEC_AMBIGUOUS(§6.3): spec describes AsymmetricTelemetry semantically but does not
// fully enumerate TelemetryAccess layout. Using nested map as stub; revisit during phase 3.
struct TelemetryAccess {
    // territory_id → set of visible fields. Bitmask over ObservationField enumerators.
    // TODO(phase 3, §6.3): consider replacing with deterministic sorted-key container
    //   (std::unordered_map iteration order is unspecified; sync-log write paths must not
    //   iterate this without sorting — see §2.4).
    std::unordered_map<state::TerritoryId, uint64_t> field_visibility;
};

}  // namespace rc::sim::observer
