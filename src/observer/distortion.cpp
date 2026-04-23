#include "rcsim/observer/distortion.hpp"

// §6.2: DistortionInvestment is pure-data; cost-application lives here.

namespace rc::sim::observer {

double concealment_factor(const DistortionInvestment& /*d*/) noexcept {
    // TODO(phase 3, §6.4)
    return 0.0;
}

void apply_distortion_costs(
    state::PrincipalId /*actor*/,
    double /*investment_M*/,
    double /*investment_S*/,
    double& /*treasury_M*/,
    double& /*legitimacy_S*/
) noexcept {
    // TODO(phase 3, §6.2): debit M; debit S floored at legitimacy_S; excess S truncated.
}

}  // namespace rc::sim::observer
