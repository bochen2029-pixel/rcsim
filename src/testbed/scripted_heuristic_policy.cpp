#include "rcsim/testbed/scripted_heuristic_policy.hpp"

#include "rcsim/state/principal.hpp"

// §11.3: v1 default PrincipalPolicy — regime-typed scripted heuristic.

namespace rc::sim::testbed {

std::vector<action::Action> ScriptedHeuristicPolicy::select_actions(
    const observer::Observation& /*own_last_seen*/,
    const state::PrincipalState& /*self*/,
    const political::PoliticalClock& /*clock*/,
    core::Tick /*current_tick*/
) {
    // TODO(phase 5, §11.3)
    return {};
}

}  // namespace rc::sim::testbed
