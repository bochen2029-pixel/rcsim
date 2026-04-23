#include "rcsim/action/queue.hpp"

// §5.3: Action ordering — sort by (tick, principal_id, action_seq).

namespace rc::sim::action {

bool ActionCompare::operator()(const PendingAction& /*a*/, const PendingAction& /*b*/) const noexcept {
    // TODO(phase 2, §5.3): lex-compare (tick, principal_id, action_seq) ascending.
    return false;
}

void resolve_conflicts(std::vector<PendingAction>& /*queue*/) noexcept {
    // TODO(phase 2, §5.3): priority desc then principal_id asc; loser → Pass with reason.
}

}  // namespace rc::sim::action
