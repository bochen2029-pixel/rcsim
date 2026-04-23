#pragma once

// §8: Integrator — fixed-step RK4, no bisection, gates held constant per tick.
// §8.1 IntegratorConfig: dt_years = 1/52 (1 tick = 1 week), substeps_per_tick = 4, max_saturation_ratio = 0.95.
// §8.2 CRITICAL: bisection abolished. Gates evaluated once at tick boundary; constant across substeps.
// §9.6 Phase 6: integrate_rk4_fixed_step(s, dt, substeps, gates_t0).

#include <cstdint>

#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/state/world_state.hpp"

namespace rc::sim::dynamics {

// §8.1: IntegratorConfig.
struct IntegratorConfig {
    double dt_years = 1.0 / 52.0;      // 1 tick = 1 week
    uint32_t substeps_per_tick = 4;    // RK4 stages
    double max_saturation_ratio = 0.95;
    // §8.2: NOTE — gate_bisection_tol and gate_bisection_max_iters REMOVED in v1.3.
    // Any field referencing bisection is a bug; see test 15_bisection_absent.
};

// §8: Integrator base — fixed-step RK4.
class Integrator {
public:
    virtual ~Integrator() = default;

    // §8.2, §9.6: step — runs fixed-step RK4 over one tick with gates held constant.
    // Returns number of substeps taken (always == cfg.substeps_per_tick; signature kept
    // for diagnostics + future compatibility).
    virtual uint32_t step(
        state::WorldState& s,
        double dt_years,
        const GateValues& gates_t0
    ) noexcept = 0;
};

// §8.2: Rk4Integrator — fixed-step; no bisection.
class Rk4Integrator : public Integrator {
public:
    Rk4Integrator(ODESystem* ode, IntegratorConfig cfg) noexcept;

    // TODO(phase 1, §8.2): implement canonical RK4 with gates closed-over as constants.
    uint32_t step(
        state::WorldState& s,
        double dt_years,
        const GateValues& gates_t0
    ) noexcept override;

    // NOTE: there is no `bisect_to_gate_crossing` method. Its presence anywhere in
    // the codebase is a bug. Test 15_bisection_absent.cpp verifies absence.

private:
    ODESystem* ode_;
    IntegratorConfig cfg_;
};

// §8.2, §9.6: convenience wrapper matching the spec's `integrate_rk4_fixed_step` name.
// TODO(phase 1, §8.2, §9.6): implement per DESIGN_v1.3.md §9.6
void integrate_rk4_fixed_step(
    state::WorldState& s,
    double dt_years,
    uint32_t substeps_per_tick,
    const GateValues& gates_t0
) noexcept;

}  // namespace rc::sim::dynamics
