#pragma once

// §5.4: Split validation with Fizzle mechanic — closes Epistemic Oracle Leak (v1.3 critical fix).
// DESIGN_v1.3.md §5.4: validate_epistemic() returns {ok, reason} — ok means principal
// BELIEVES action valid. Uses observation (fog-of-war), NOT ground truth.
// §16.2 property test: epistemic_validation_no_leak.cpp + fizzle_no_ground_truth_leak.cpp.

#include <cstdint>

#include "rcsim/action/action.hpp"
#include "rcsim/state/territory.hpp"

// Forward-declare Observation to avoid heavy include chain.
namespace rc::sim::observer {
struct Observation;
}

namespace rc::sim::action {

// R-13: ValidationReason — closed enum frozen at schema_major (additive only
// within schema_minor). Stored in canonical wire format as uint32 LE per §10.1a.
enum class ValidationReason : uint32_t {
    Ok                     = 0,
    InsufficientTreasury   = 1,
    InsufficientLegitimacy = 2,
    NotOwnedByActor        = 3,
    PoliticalClockGated    = 4,
    DistortionLockout      = 5,
    UnknownTarget          = 6,
    InvalidParameters      = 7,
    DurationOutOfRange     = 8,
    PrincipalLockedOut     = 9
};

// §5.4: ValidationResult.
struct ValidationResult {
    bool ok;
    ValidationReason reason;
};

// §5.4, §9.2a: validate_epistemic — checks action against principal's last_seen observation.
//   ok == true means principal BELIEVES action is valid (may still Fizzle at apply).
[[nodiscard]] ValidationResult validate_epistemic(
    const observer::Observation& obs,
    state::PrincipalId actor,
    const Action& a
) noexcept;

}  // namespace rc::sim::action
