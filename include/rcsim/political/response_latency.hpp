#pragma once

// §7.3: Default response latencies per regime.
// DESIGN_v1.3.md §7.3: Action validation gates on political clock; per-regime defaults.

#include "rcsim/political/clock.hpp"
#include "rcsim/political/regime.hpp"

namespace rc::sim::political {

// §7.3: Per-regime default response latency (tau_modal only; variance calibrated in phase 7b).
// SPEC_AMBIGUOUS(§7.3): spec says "§7.3-§7.5 Unchanged from v1.2" but the v1.3 document
//   does not reproduce the numeric defaults. Populate from v1.2 values during phase 3.
// TODO(phase 3, §7.3): implement per DESIGN_v1.3.md §7.3 + DESIGN_v1.2 inherited defaults.
double default_tau_modal(PoliticalRegime r) noexcept;
double default_tau_variance(PoliticalRegime r) noexcept;

// §7.3: Per-regime action-validation gate — true means action is allowed at current tick.
//   Democracy: wait for election-adjacent ticks; respond to CPI shocks within tau_modal
//   Autocracy: act on survival_pressure > crisis_threshold; purge-gated otherwise
//   TheocraticAutocracy: succession-event-gated; defensive otherwise
//   Petrostate: oil-revenue-shortfall-gated
//   FailedState: crisis-only, high variance
// TODO(phase 3, §7.3, §11.3): implement per DESIGN_v1.3.md §7.3 + §11.3 ScriptedHeuristicPolicy.
bool regime_gate_allows_action(
    const PoliticalClock& clock,
    double crisis_signal
) noexcept;

}  // namespace rc::sim::political
