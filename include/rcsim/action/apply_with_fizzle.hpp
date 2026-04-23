#pragma once

// §5.4: Ground-truth application with Fizzle semantics.
// DESIGN_v1.3.md §5.4: after validate_epistemic passes, resources are deducted
// unconditionally; apply() may return Fizzled when ground truth makes the action
// impossible (e.g., target already destroyed). Fizzle is information-bearing
// (actor notices resources gone, no effect) but does NOT leak specific ground-truth state.

#include <string>

#include "rcsim/action/action.hpp"
#include "rcsim/state/territory.hpp"
#include "rcsim/state/world_state.hpp"

namespace rc::sim::action {

// §5.4: ApplyResult — outcome of ground-truth apply.
struct ApplyResult {
    state::WorldState state;   // mutated state
    bool applied = false;      // physical effect took hold
    bool fizzled = false;      // ground truth made action impossible
    // SPEC_AMBIGUOUS(§5.4): reason enum not enumerated; string stub per validate_epistemic.
    std::string reason;
};

// §5.4, §9.2b: deduct_resources — unconditional on ground truth.
// Called after validate_epistemic passes, before apply().
// TODO(phase 2, §5.4, §9.2b): implement per DESIGN_v1.3.md §5.4
void deduct_resources(state::PrincipalState& p, const Action& a) noexcept;

// §5.4, §9.2c: apply — ground-truth application. May Fizzle.
// NOTE on signature: spec shows `apply(WorldState s, PrincipalId p, const Action& a)` returning
// ApplyResult by-value. To preserve move-only state, ApplyResult embeds WorldState.
// TODO(phase 2, §5.4, §9.2c): implement per DESIGN_v1.3.md §5.4
[[nodiscard]] ApplyResult apply(
    state::WorldState s,
    state::PrincipalId actor,
    const Action& a
) noexcept;

}  // namespace rc::sim::action
