#include "rcsim/dynamics/integrator.hpp"

// §8: Fixed-step RK4. NO bisection. Gates held constant across substages.

namespace rc::sim::dynamics {

Rk4Integrator::Rk4Integrator(ODESystem* ode, IntegratorConfig cfg) noexcept
    : ode_(ode), cfg_(cfg) {}

uint32_t Rk4Integrator::step(
    state::WorldState& /*s*/,
    double /*dt_years*/,
    const GateValues& /*gates_t0*/
) noexcept {
    // TODO(phase 1, §8.2): canonical fixed-step RK4; gates are closed-over constant.
    return cfg_.substeps_per_tick;
}

void integrate_rk4_fixed_step(
    state::WorldState& /*s*/,
    double /*dt_years*/,
    uint32_t /*substeps_per_tick*/,
    const GateValues& /*gates_t0*/
) noexcept {
    // TODO(phase 1, §8.2, §9.6)
}

}  // namespace rc::sim::dynamics
