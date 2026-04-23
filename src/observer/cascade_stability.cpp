#include "rcsim/observer/cascade_stability.hpp"

#include "rcsim/state/principal.hpp"

// §6.4.1: legitimacy_S bounds; distortion lockout at 0; optional recovery per-regime.

namespace rc::sim::observer {

void clamp_legitimacy(state::PrincipalState& /*p*/) noexcept {
    // TODO(phase 3, §6.4.1): clamp to [0, initial_S]
}

bool distortion_locked_out(const state::PrincipalState& /*p*/) noexcept {
    // TODO(phase 3, §6.4.1): legitimacy_S <= 0
    return false;
}

void apply_legitimacy_recovery(state::PrincipalState& /*p*/, double /*dt_years*/) noexcept {
    // TODO(phase 3, §6.4.1): per-regime rate; default 0 (permanent).
}

void debit_legitimacy_for_attribution(
    state::PrincipalState& /*distorter*/,
    double /*attribution_magnitude*/
) noexcept {
    // TODO(phase 3, §6.4.1): floored at 0.
}

}  // namespace rc::sim::observer
