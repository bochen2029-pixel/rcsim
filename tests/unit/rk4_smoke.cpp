#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstring>

#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/hash.hpp"
#include "rcsim/dynamics/integrator.hpp"
#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/state/world_state.hpp"

// Phase-1 end-to-end smoke test:
//   1. Build a single-territory WorldState with reasonable initial values.
//   2. Run a few RK4 ticks via Rk4Integrator with gates evaluated once per tick.
//   3. Canonicalize and hash; verify hash is non-zero and non-NaN.
//
// This is the minimal proof-of-life for the §19 Phase 1 architecture
// (core types + RK4 + canonicalization + hash) before the mean_field_1.yaml
// retrodiction lands.

TEST_CASE("RK4 single-territory smoke", "[unit]") {
    using namespace rc::sim;

    state::WorldState s{};
    s.tick = 0;
    s.time_years = 0.0;

    // Single territory with v5 mean-field defaults (units in EJ/yr per §2.2).
    state::TerritoryState t{};
    t.r       = 5.0;        // EROEI ~5
    t.H       = 1.0e8;      // 100M employed cognitive labor
    t.G       = 1.0e6;      // initial deployed AI compute
    t.M       = 1.0e9;      // financial claims
    t.Npop    = 3.3e8;      // ~330M
    t.S       = 1.0;
    t.Qelec   = 100.0;      // EJ/yr
    t.Qliq    = 100.0;      // EJ/yr
    t.D_local = 0.0;
    t.eta     = 0.0;
    t.phi     = 0.0;
    t.id      = 0;
    t.owner   = 0;
    s.grid.states.push_back(t);

    s.global.D_global    = 0.0;
    s.global.A           = 1.0;
    s.global.S_AI_index  = 1.0;
    s.global.seed_scenario = 42;

    s.canon.residual_r.assign(1, 0.0);
    s.canon.residual_H.assign(1, 0.0);
    s.canon.residual_G.assign(1, 0.0);
    s.canon.residual_M.assign(1, 0.0);
    s.canon.residual_Npop.assign(1, 0.0);
    s.canon.residual_S.assign(1, 0.0);
    s.canon.residual_Qelec.assign(1, 0.0);
    s.canon.residual_Qliq.assign(1, 0.0);
    s.canon.residual_D_local.assign(1, 0.0);

    dynamics::ODESystem ode;
    dynamics::IntegratorConfig cfg;
    cfg.dt_years = 1.0 / 52.0;
    cfg.substeps_per_tick = 4;
    dynamics::Rk4Integrator integ(&ode, cfg);

    // Run 10 ticks (~70 days simulated time).
    for (int tick = 0; tick < 10; ++tick) {
        auto gates = dynamics::evaluate_gates(s);
        integ.step(s, cfg.dt_years, gates);
        s.tick++;
        s.time_years += cfg.dt_years;
    }

    // Canonicalize and hash; verify state is finite and hash is well-formed.
    core::canonicalize_state_with_residual(s);
    auto hash = core::canonical_hash(s);

    // Hash must not be all-zero (would indicate uninitialized).
    bool all_zero = true;
    for (auto b : hash.bytes) {
        if (b != 0) { all_zero = false; break; }
    }
    REQUIRE_FALSE(all_zero);

    // Trajectory should be non-degenerate after 10 ticks: r still positive and finite,
    // population non-negative.
    REQUIRE(s.grid.states[0].r       >  0.0);
    REQUIRE(s.grid.states[0].Npop    >  0.0);
    REQUIRE(std::isfinite(s.grid.states[0].r));
    REQUIRE(std::isfinite(s.grid.states[0].Qelec));
}

TEST_CASE("Hash determinism: identical state → identical hash", "[unit][determinism]") {
    using namespace rc::sim;

    auto build = []() {
        state::WorldState s{};
        s.tick = 0;
        s.time_years = 0.0;
        state::TerritoryState t{};
        t.r = 5.0; t.H = 1e8; t.G = 1e6; t.M = 1e9; t.Npop = 3.3e8;
        t.S = 1.0; t.Qelec = 100.0; t.Qliq = 100.0; t.D_local = 0.0;
        t.id = 0; t.owner = 0;
        s.grid.states.push_back(t);
        s.global.A = 1.0; s.global.S_AI_index = 1.0;
        return s;
    };

    auto a = build();
    auto b = build();

    auto ha = core::canonical_hash(a);
    auto hb = core::canonical_hash(b);

    REQUIRE(ha == hb);
}
