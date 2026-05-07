#pragma once

// §5.4: Ground-truth application with Fizzle semantics.
// DESIGN_v1.3.md §5.4: after validate_epistemic passes, resources are deducted
// unconditionally; apply() may return Fizzled when ground truth makes the action
// impossible (e.g., target already destroyed). Fizzle is information-bearing
// (actor notices resources gone, no effect) but does NOT leak specific ground-truth state.

#include <cstdint>

#include "rcsim/action/action.hpp"
#include "rcsim/state/territory.hpp"
#include "rcsim/state/world_state.hpp"

namespace rc::sim::action {

// R-14: FizzleReason — closed enum frozen at schema_major (additive only within
// schema_minor). Distinct from ValidationReason: this surfaces post-validation
// ground-truth failure. Wire-format: uint32 LE per §10.1a.
enum class FizzleReason : uint32_t {
    None                    = 0,
    TargetAlreadyDestroyed  = 1,
    TargetOwnerChanged      = 2,
    SupplyExhausted         = 3,
    GeographyImpossible     = 4,
    GateClosedAtApply       = 5,
    ConflictDegraded        = 6
};

// §5.4: ApplyResult — outcome of ground-truth apply.
struct ApplyResult {
    state::WorldState state;
    bool applied = false;
    bool fizzled = false;
    FizzleReason reason = FizzleReason::None;
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
