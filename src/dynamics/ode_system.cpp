#include "rcsim/dynamics/ode_system.hpp"

// §4.1: Per-territory ODE RHS (RC-1 through RC-8).
// §8.2: Gates held constant across RK4 substages — no mid-step reevaluation.

namespace rc::sim::dynamics {

GateValues evaluate_gates(const state::WorldState& /*s*/) noexcept {
    // TODO(phase 1, §8.2, §9.5)
    return GateValues{};
}

ODESystem::Deriv ODESystem::derivative(
    const state::TerritoryState& /*t*/,
    const state::WorldState& /*s*/,
    const GateValues& /*gates*/,
    std::size_t /*territory_index*/
) const noexcept {
    // TODO(phase 1, §4.1): per DESIGN_v1.3.md §4.1 RC-1 .. RC-8
    return {};
}

void ODESystem::rhs(
    const state::WorldState& /*s*/,
    const GateValues& /*gates*/,
    std::vector<Deriv>& /*out_deriv*/
) const noexcept {
    // TODO(phase 1, §4.1, §2.4): per-territory loop; no reductions inside.
}

double ODESystem::gamma_kinetic(
    std::size_t /*territory_index*/,
    double /*time_years*/,
    const state::WorldState& /*s*/
) const noexcept {
    // TODO(phase 1, §4.4): Σ A_k · 1[in-window] · exp(-λ_k (t - t_start))
    return 0.0;
}

}  // namespace rc::sim::dynamics
