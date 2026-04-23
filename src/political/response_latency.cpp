#include "rcsim/political/response_latency.hpp"

// §7.3: Per-regime default response latencies; action-validation gate.

namespace rc::sim::political {

double default_tau_modal(PoliticalRegime /*r*/) noexcept {
    // TODO(phase 3, §7.3): SPEC_AMBIGUOUS(§7.3) - defaults from v1.2 inheritance
    return 0.0;
}

double default_tau_variance(PoliticalRegime /*r*/) noexcept {
    // TODO(phase 3, §7.3): populated from phase 7b gamma-fits.
    return 0.0;
}

bool regime_gate_allows_action(
    const PoliticalClock& /*clock*/,
    double /*crisis_signal*/
) noexcept {
    // TODO(phase 3, §7.3, §11.3): regime-typed gate per scripted heuristic.
    return true;
}

}  // namespace rc::sim::political
