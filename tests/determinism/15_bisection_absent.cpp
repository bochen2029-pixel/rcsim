#include <catch2/catch_test_macros.hpp>

#include "rcsim/dynamics/integrator.hpp"
#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/state/world_state.hpp"

// §16.1 test 15 / §8.2: Rk4Integrator must never subdivide dt at gate crossings.
//
// Two checks:
//   1. Runtime: step() returns exactly cfg.substeps_per_tick on every invocation,
//      including across gate-transition territory states.
//   2. Construct a state where Q_elec is right at the Π_hard threshold (gate
//      crossing); verify step still returns the configured substep count.

TEST_CASE("15_bisection_absent: step returns configured substep count", "[determinism]") {
    using namespace rc::sim;

    state::WorldState s{};
    state::TerritoryState t{};
    t.r = 5.0; t.H = 1e8; t.G = 1e6; t.M = 1e9; t.Npop = 3.3e8;
    t.S = 1.0; t.Qelec = 100.0; t.Qliq = 100.0; t.D_local = 0.0;
    t.id = 0; t.owner = 0;
    s.grid.states.push_back(t);
    s.global.A = 1.0; s.global.S_AI_index = 1.0;

    dynamics::ODESystem ode;
    dynamics::IntegratorConfig cfg;
    cfg.dt_years = 1.0 / 52.0;
    cfg.substeps_per_tick = 4;
    dynamics::Rk4Integrator integ(&ode, cfg);

    auto gates = dynamics::evaluate_gates(s);
    uint32_t taken = integ.step(s, cfg.dt_years, gates);
    REQUIRE(taken == cfg.substeps_per_tick);
}

TEST_CASE("15_bisection_absent: at gate crossing step still uses fixed substeps",
          "[determinism]") {
    using namespace rc::sim;

    // Construct state where Q_elec is exactly at the soft-gate centroid; if any
    // bisection logic were present, it would manifest by varying the returned
    // substep count near this threshold.
    state::WorldState s{};
    state::TerritoryState t{};
    t.r = 5.0; t.H = 1e8; t.G = 1e6; t.M = 1e9; t.Npop = 3.3e8;
    t.S = 1.0;
    t.Qelec = 1.0e-3;     // sits at the Theta_soft centroid in evaluate_gates
    t.Qliq  = 100.0; t.D_local = 0.0;
    t.id = 0; t.owner = 0;
    s.grid.states.push_back(t);
    s.global.A = 1.0; s.global.S_AI_index = 1.0;

    dynamics::ODESystem ode;
    dynamics::IntegratorConfig cfg;
    cfg.dt_years = 1.0 / 52.0;
    cfg.substeps_per_tick = 8;       // exercise different substep count too
    dynamics::Rk4Integrator integ(&ode, cfg);

    auto gates = dynamics::evaluate_gates(s);
    uint32_t taken = integ.step(s, cfg.dt_years, gates);
    REQUIRE(taken == cfg.substeps_per_tick);
}
