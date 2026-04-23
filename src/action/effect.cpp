#include "rcsim/action/effect.hpp"

// §5.2: action/effect.hpp re-exports state::PendingEffect types; no implementation needed.
// Effect maturation + retirement logic lives in the Advance loop (phase 1 + phase 8).

namespace rc::sim::action {
// TODO(phase 2, §5.2, §9.1, §9.8): mature_pending / retire_expired helpers.
}  // namespace rc::sim::action
