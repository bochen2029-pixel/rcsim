#pragma once

// §11.3: ScriptedHeuristicPolicy — v1 default PrincipalPolicy.
// DESIGN_v1.3.md §11.3: regime-typed scripted heuristic.
//   Democracy: wait for election-adjacent ticks; respond to CPI shocks within tau_modal
//   Autocracy: act on survival_pressure > crisis_threshold; purge-gated elsewise
//   TheocraticAutocracy: succession-event-gated; otherwise defensive
//   Petrostate: oil-revenue-shortfall-gated
//   FailedState: crisis-only, high variance

#include "rcsim/testbed/principal_policy.hpp"

namespace rc::sim::testbed {

// §11.3: ScriptedHeuristicPolicy.
class ScriptedHeuristicPolicy : public PrincipalPolicy {
public:
    // TODO(phase 5, §11.3): implement regime-typed scripted heuristic per DESIGN_v1.3.md §11.3
    std::vector<action::Action> select_actions(
        const observer::Observation& own_last_seen,
        const state::PrincipalState& self,
        const political::PoliticalClock& clock,
        core::Tick current_tick
    ) override;
};

}  // namespace rc::sim::testbed
