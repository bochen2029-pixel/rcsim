#pragma once

// §5.3: Action ordering. Actions sort by (tick, principal_id, action_seq).
//   Conflicts resolve by priority desc then principal_id asc; loser degrades to Pass.
// §9.2: Drain inbound actions in canonical order.

#include <cstdint>
#include <vector>

#include "rcsim/action/action.hpp"
#include "rcsim/core/tick.hpp"
#include "rcsim/state/territory.hpp"

namespace rc::sim::action {

// §5.3: PendingAction — queued action with sort keys.
struct PendingAction {
    core::Tick tick;                 // primary sort key
    state::PrincipalId principal_id; // secondary sort key
    uint32_t action_seq;             // tertiary sort key (per-principal issuance order)
    int32_t priority;                // §5.3: conflict resolution
    Action action;
};

// §5.3: Canonical comparator — (tick, principal_id, action_seq) ascending.
struct ActionCompare {
    // TODO(phase 2, §5.3): implement per DESIGN_v1.3.md §5.3
    bool operator()(const PendingAction& a, const PendingAction& b) const noexcept;
};

// §5.3: Conflict resolution — priority desc then principal_id asc; loser → Pass.
// TODO(phase 2, §5.3): implement per DESIGN_v1.3.md §5.3
void resolve_conflicts(std::vector<PendingAction>& queue) noexcept;

// §9.2: ActionQueue — the per-tick inbound queue.
struct ActionQueue {
    std::vector<PendingAction> actions;
};

}  // namespace rc::sim::action
