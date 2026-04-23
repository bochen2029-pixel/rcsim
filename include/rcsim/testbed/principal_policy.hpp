#pragma once

// §11.3: PrincipalPolicy abstract class (AEGIS §2.7 — new v1.3).
// DESIGN_v1.3.md §11.3: Monte Carlo sweeps need principals to act autonomously between
// scripted actions. Abstract interface; v1 default is ScriptedHeuristicPolicy.

#include <vector>

#include "rcsim/action/action.hpp"
#include "rcsim/core/tick.hpp"
#include "rcsim/observer/measurement.hpp"
#include "rcsim/political/clock.hpp"

// Forward declare — state::PrincipalState authored in state/principal.hpp.
namespace rc::sim::state { struct PrincipalState; }

namespace rc::sim::testbed {

// §11.3: PrincipalPolicy — abstract agent interface.
class PrincipalPolicy {
public:
    virtual ~PrincipalPolicy() = default;

    // §11.3: Select actions for a principal based on its observation + self-state + clock.
    // TODO(phase 5, §11.3): implement concrete policies per DESIGN_v1.3.md §11.3
    virtual std::vector<action::Action> select_actions(
        const observer::Observation& own_last_seen,
        const state::PrincipalState& self,
        const political::PoliticalClock& clock,
        core::Tick current_tick
    ) = 0;
};

// §11.3: Future policy types deferred to v1.1 (comment only — no stub here):
//   - GreedyEVPolicy
//   - PolicyNetwork (learned from prior sweeps)

}  // namespace rc::sim::testbed
