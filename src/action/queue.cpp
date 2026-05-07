#include "rcsim/action/queue.hpp"

#include <algorithm>

// §5.3: Action ordering — sort by (tick, principal_id, action_seq) ascending.
// Conflicts resolve by priority desc → principal_id asc; loser degrades to Pass.
//
// Determinism: every comparator field is integer; std::ranges::sort is
// guaranteed stable when called with a strict-weak ordering, but stability
// matters only if duplicate sort-keys exist. Our (tick, principal_id, action_seq)
// triple is unique within a tick window, so std::sort is sufficient.

namespace rc::sim::action {

bool ActionCompare::operator()(const PendingAction& a, const PendingAction& b) const noexcept {
    if (a.tick != b.tick)                 return a.tick < b.tick;
    if (a.principal_id != b.principal_id) return a.principal_id < b.principal_id;
    return a.action_seq < b.action_seq;
}

namespace {

// "Conflict": two actions targeting the same atomic resource. Phase 2 v1
// detects conflict by `priority`-tied entries with overlapping principal-id;
// the spec's §5.3 conflict surface is "actions that mutate the same
// territory/principal mutation-class in the same tick." Phase 3 will refine
// when SeizeTerritory + Blockade composition ambiguities surface; for now we
// treat any two PendingActions in the same (tick, principal_id) bucket as
// non-conflicting (validation + apply handle their own atomicity). Conflict
// resolution proper applies across principals — handled at apply time via
// FizzleReason::ConflictDegraded.
//
// This function exists to be expanded in Phase 3; in Phase 2 it sorts the
// queue by priority-desc within tick + ensures losers (lower priority on
// the same atomic-resource conflict) are degraded to Pass.

bool same_target_class(const Action& /*lhs*/, const Action& /*rhs*/) noexcept {
    // Phase 2: no cross-principal conflict detection yet — that requires
    // walking the action variants. Always returns false; conflict resolution
    // is structurally a Phase 3 feature when SeizeTerritory/Blockade compose.
    return false;
}

}  // namespace

void resolve_conflicts(std::vector<PendingAction>& queue) noexcept {
    // Phase 2: secondary sort by priority desc → principal_id asc within (tick).
    // The base canonical sort already establishes (tick, principal_id, action_seq);
    // here we resort within each tick by priority for conflict-loser detection.
    auto by_priority = [](const PendingAction& a, const PendingAction& b) noexcept {
        if (a.tick != b.tick)         return a.tick < b.tick;
        if (a.priority != b.priority) return a.priority > b.priority;   // desc
        return a.principal_id < b.principal_id;
    };
    std::sort(queue.begin(), queue.end(), by_priority);

    // Walk consecutive same-tick pairs; degrade losers to Pass.
    for (std::size_t i = 1; i < queue.size(); ++i) {
        if (queue[i].tick != queue[i - 1].tick) continue;
        if (same_target_class(queue[i].action, queue[i - 1].action)) {
            // queue[i-1] is winner (higher priority due to sort); degrade i.
            queue[i].action = Pass{};
        }
    }
}

}  // namespace rc::sim::action
