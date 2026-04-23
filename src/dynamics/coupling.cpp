#include "rcsim/dynamics/coupling.hpp"

// §4.3: Q_elec ↔ Q_liq conversion.
// §4.6, §9.4: integer_network_flow_solve runs Phase 4 of Advance.

namespace rc::sim::dynamics {

ConversionFlows compute_conversion_flows(
    const state::TerritoryState& /*t*/,
    double /*u_liq_to_elec*/,
    double /*u_elec_to_liq*/
) noexcept {
    // TODO(phase 2, §4.3): convert = η · u, η_l→e ≈ 0.35, η_e→l ≈ 0.6.
    return {};
}

void integer_network_flow_solve(state::WorldState& /*s*/) noexcept {
    // TODO(phase 2, §4.6, §9.4)
}

double blockade(const state::TerritoryState& /*t*/, const state::WorldState& /*s*/) noexcept {
    // TODO(phase 2, §5.1 Blockade + §4.1 RC-8)
    return 0.0;
}

double migration_flux(std::size_t /*territory_index*/, const state::WorldState& /*s*/) noexcept {
    // TODO(phase 2, §4.1, §17.2)
    return 0.0;
}

}  // namespace rc::sim::dynamics
