#include "rcsim/dynamics/integrator.hpp"

#include <vector>

// §8: Fixed-step RK4. NO bisection. Gates held constant across substages.
//
// §8.2: gate_bisection_tol and gate_bisection_max_iters are NOT in IntegratorConfig.
// This file must contain no `bisect`/`bisection` symbols; CI test 15 enforces.

namespace rc::sim::dynamics {

namespace {

// Apply derivative to a TerritoryState in place: out = state + dt * deriv.
inline void apply_step(
    state::TerritoryState& out,
    const state::TerritoryState& base,
    const ODESystem::Deriv& d,
    double dt
) noexcept {
    out.r       = base.r       + dt * d.dr;
    out.H       = base.H       + dt * d.dH;
    out.G       = base.G       + dt * d.dG;
    out.M       = base.M       + dt * d.dM;
    out.Npop    = base.Npop    + dt * d.dNpop;
    out.S       = base.S       + dt * d.dS;
    out.Qelec   = base.Qelec   + dt * d.dQelec;
    out.Qliq    = base.Qliq    + dt * d.dQliq;
    out.D_local = base.D_local + dt * d.dD_local;
    // Carry-through, including derived eta/phi (rebuilt pre-integration each tick).
    out.eta   = base.eta;
    out.phi   = base.phi;
    out.id    = base.id;
    out.owner = base.owner;
}

}  // namespace

Rk4Integrator::Rk4Integrator(ODESystem* ode, IntegratorConfig cfg) noexcept
    : ode_(ode), cfg_(cfg) {}

uint32_t Rk4Integrator::step(
    state::WorldState& s,
    double dt_years,
    const GateValues& gates_t0
) noexcept {
    if (ode_ == nullptr || s.grid.states.empty()) {
        return 0;
    }
    const std::size_t N = s.grid.states.size();
    const uint32_t substeps = cfg_.substeps_per_tick;
    const double h = dt_years / static_cast<double>(substeps);

    // Working buffers — k1..k4 derivatives + intermediate state copies.
    std::vector<ODESystem::Deriv> k1(N), k2(N), k3(N), k4(N);
    std::vector<state::TerritoryState> tmp_grid(N);

    // Snapshot WorldState that derivative() reads (we mutate s.grid in-place
    // between substages but must NOT mutate s.global mid-tick — gates_t0 is the
    // closed-over constant per §8.2).
    state::WorldState scratch = s;

    for (uint32_t step_i = 0; step_i < substeps; ++step_i) {
        // k1 = f(y_n)
        ode_->rhs(scratch, gates_t0, k1);

        // k2 = f(y_n + h/2 · k1)
        for (std::size_t i = 0; i < N; ++i) {
            apply_step(tmp_grid[i], scratch.grid.states[i], k1[i], h * 0.5);
        }
        scratch.grid.states.swap(tmp_grid);
        ode_->rhs(scratch, gates_t0, k2);
        scratch.grid.states.swap(tmp_grid);   // restore base

        // k3 = f(y_n + h/2 · k2)
        for (std::size_t i = 0; i < N; ++i) {
            apply_step(tmp_grid[i], scratch.grid.states[i], k2[i], h * 0.5);
        }
        scratch.grid.states.swap(tmp_grid);
        ode_->rhs(scratch, gates_t0, k3);
        scratch.grid.states.swap(tmp_grid);

        // k4 = f(y_n + h · k3)
        for (std::size_t i = 0; i < N; ++i) {
            apply_step(tmp_grid[i], scratch.grid.states[i], k3[i], h);
        }
        scratch.grid.states.swap(tmp_grid);
        ode_->rhs(scratch, gates_t0, k4);
        scratch.grid.states.swap(tmp_grid);

        // y_{n+1} = y_n + (h/6) · (k1 + 2 k2 + 2 k3 + k4)
        const double sixth = h / 6.0;
        for (std::size_t i = 0; i < N; ++i) {
            ODESystem::Deriv combined{
                k1[i].dr       + 2.0 * k2[i].dr       + 2.0 * k3[i].dr       + k4[i].dr,
                k1[i].dH       + 2.0 * k2[i].dH       + 2.0 * k3[i].dH       + k4[i].dH,
                k1[i].dG       + 2.0 * k2[i].dG       + 2.0 * k3[i].dG       + k4[i].dG,
                k1[i].dM       + 2.0 * k2[i].dM       + 2.0 * k3[i].dM       + k4[i].dM,
                k1[i].dNpop    + 2.0 * k2[i].dNpop    + 2.0 * k3[i].dNpop    + k4[i].dNpop,
                k1[i].dS       + 2.0 * k2[i].dS       + 2.0 * k3[i].dS       + k4[i].dS,
                k1[i].dQelec   + 2.0 * k2[i].dQelec   + 2.0 * k3[i].dQelec   + k4[i].dQelec,
                k1[i].dQliq    + 2.0 * k2[i].dQliq    + 2.0 * k3[i].dQliq    + k4[i].dQliq,
                k1[i].dD_local + 2.0 * k2[i].dD_local + 2.0 * k3[i].dD_local + k4[i].dD_local,
            };
            state::TerritoryState advanced;
            apply_step(advanced, scratch.grid.states[i], combined, sixth);
            scratch.grid.states[i] = advanced;
        }
    }

    // Commit the integrated grid back to s. s.global was untouched during the
    // tick; gates were held constant per §8.2.
    s.grid.states = std::move(scratch.grid.states);
    return substeps;
}

void integrate_rk4_fixed_step(
    state::WorldState& s,
    double dt_years,
    uint32_t substeps_per_tick,
    const GateValues& gates_t0
) noexcept {
    // Convenience wrapper that constructs a default ODESystem + Rk4Integrator
    // for callers that don't hold a long-lived integrator. The advance loop
    // (tools/rcsim_run.cpp Phase 1+) prefers the long-lived form.
    ODESystem ode;
    IntegratorConfig cfg;
    cfg.dt_years = dt_years;
    cfg.substeps_per_tick = substeps_per_tick;
    Rk4Integrator integ(&ode, cfg);
    integ.step(s, dt_years, gates_t0);
}

}  // namespace rc::sim::dynamics
