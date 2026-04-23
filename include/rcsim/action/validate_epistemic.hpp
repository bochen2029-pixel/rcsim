#pragma once

// §5.4: Split validation with Fizzle mechanic — closes Epistemic Oracle Leak (v1.3 critical fix).
// DESIGN_v1.3.md §5.4: validate_epistemic() returns {ok, reason} — ok means principal
// BELIEVES action valid. Uses observation (fog-of-war), NOT ground truth.
// §16.2 property test: epistemic_validation_no_leak.cpp + fizzle_no_ground_truth_leak.cpp.

#include <cstdint>
#include <string>

#include "rcsim/action/action.hpp"
#include "rcsim/state/territory.hpp"

// Forward-declare Observation to avoid heavy include chain.
namespace rc::sim::observer {
struct Observation;
}

namespace rc::sim::action {

// §5.4: ValidationResult.
struct ValidationResult {
    bool ok;
    // SPEC_AMBIGUOUS(§5.4): reason-code enum not enumerated in spec; using string stub.
    //   Phase 2 should replace with a stable enum for determinism.
    std::string reason;
};

// §5.4, §9.2a: validate_epistemic — checks action against principal's last_seen observation.
//   ok == true means principal BELIEVES action is valid (may still Fizzle at apply).
// TODO(phase 2, §5.4, §9.2a): implement per DESIGN_v1.3.md §5.4
[[nodiscard]] ValidationResult validate_epistemic(
    const observer::Observation& obs,
    state::PrincipalId actor,
    const Action& a
) noexcept;

}  // namespace rc::sim::action
