#include "rcsim/action/validate_epistemic.hpp"

#include "rcsim/observer/measurement.hpp"

// §5.4: validate_epistemic — checks against principal's observation, NOT ground truth.
// Closes Epistemic Oracle Leak: ground-truth validation is performed in apply_with_fizzle.

namespace rc::sim::action {

ValidationResult validate_epistemic(
    const observer::Observation& /*obs*/,
    state::PrincipalId /*actor*/,
    const Action& /*a*/
) noexcept {
    // TODO(phase 2, §5.4, §9.2a)
    return {true, {}};
}

}  // namespace rc::sim::action
