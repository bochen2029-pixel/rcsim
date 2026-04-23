#include "rcsim/action/apply_with_fizzle.hpp"

// §5.4: Ground-truth application with Fizzle mechanic.
// §9.2b: deduct_resources unconditional on ground truth.
// §9.2c: apply may return Fizzled when ground truth makes action impossible.

namespace rc::sim::action {

void deduct_resources(state::PrincipalState& /*p*/, const Action& /*a*/) noexcept {
    // TODO(phase 2, §5.4, §9.2b)
}

ApplyResult apply(state::WorldState s, state::PrincipalId /*actor*/, const Action& /*a*/) noexcept {
    // TODO(phase 2, §5.4, §9.2c)
    ApplyResult r;
    r.state = std::move(s);
    return r;
}

}  // namespace rc::sim::action
