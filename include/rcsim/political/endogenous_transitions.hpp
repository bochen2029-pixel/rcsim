#pragma once

// §7.5: Two endogenous regime transitions (v1 limited).
// DESIGN_v1.3.md §7.5:
//   Democracy → FailedState: sustained Q→0 (energy collapse).
//   Autocracy → TransitionalState: sustained survival_pressure > 0.9.
// Fuller typology deferred to v1.1.

// Forward declare to avoid circular include; state::PrincipalState is authored in state/principal.hpp.
namespace rc::sim::state { struct PrincipalState; struct WorldState; }

namespace rc::sim::political {

// §7.5: evaluate Democracy → FailedState transition trigger.
//   Trigger: Q collapse sustained over clock.collapse_pressure_duration ticks.
// TODO(phase 3, §7.5): implement per DESIGN_v1.3.md §7.5
bool democracy_to_failed_state_triggered(
    const state::PrincipalState& p,
    const state::WorldState& s
) noexcept;

// §7.5: evaluate Autocracy → TransitionalState transition trigger.
//   Trigger: survival_pressure > survival_pressure_limit sustained over duration.
// TODO(phase 3, §7.5): implement per DESIGN_v1.3.md §7.5
bool autocracy_to_transitional_state_triggered(
    const state::PrincipalState& p,
    const state::WorldState& s
) noexcept;

// §7.5: apply regime transition — mutates PrincipalState.regime and clock fields.
//   Queues RegimeTransition PendingEffect per §5.2 for tick-boundary application.
// TODO(phase 3, §7.5, §5.2): implement per DESIGN_v1.3.md §7.5
void apply_regime_transition(
    state::PrincipalState& p,
    PoliticalRegime new_regime
) noexcept;

}  // namespace rc::sim::political
