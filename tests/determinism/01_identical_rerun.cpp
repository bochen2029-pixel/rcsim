#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/hash.hpp"
#include "rcsim/dynamics/integrator.hpp"
#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/state/world_state.hpp"

// §16.1 test 01: identical rerun.
// Run the same in-memory scenario twice; assert per-tick StateHash equality.
// Verifies RNG-stream / tree-reduce / canonicalize stability across reruns
// in the same process.

namespace {

rc::sim::state::WorldState build_mean_field_1() {
    using namespace rc::sim;
    state::WorldState s{};
    s.tick = 0;
    s.time_years = 0.0;

    state::TerritoryState t{};
    t.r = 5.0; t.H = 1e8; t.G = 1e6; t.M = 1e9;
    t.Npop = 3.3e8; t.S = 1.0; t.Qelec = 100.0; t.Qliq = 100.0;
    t.D_local = 0.0; t.eta = 0.0; t.phi = 0.0;
    t.id = 0; t.owner = 0;
    s.grid.states.push_back(t);

    s.global.A          = 1.0;
    s.global.S_AI_index = 1.0;
    s.global.D_global   = 0.0;
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
    return s;
}

std::vector<rc::sim::core::StateHash> run(int ticks) {
    using namespace rc::sim;
    state::WorldState s = build_mean_field_1();
    dynamics::ODESystem ode;
    dynamics::IntegratorConfig cfg;
    cfg.dt_years = 1.0 / 52.0;
    cfg.substeps_per_tick = 4;
    dynamics::Rk4Integrator integ(&ode, cfg);

    std::vector<core::StateHash> hashes;
    hashes.reserve(static_cast<std::size_t>(ticks));
    for (int t = 0; t < ticks; ++t) {
        auto gates = dynamics::evaluate_gates(s);
        integ.step(s, cfg.dt_years, gates);
        core::canonicalize_state_with_residual(s);
        hashes.push_back(core::canonical_hash(s));
        s.tick++;
        s.time_years += cfg.dt_years;
    }
    return hashes;
}

}  // namespace

TEST_CASE("01_identical_rerun: hash sequence bit-equal across reruns", "[determinism]") {
    constexpr int kTicks = 100;
    auto a = run(kTicks);
    auto b = run(kTicks);
    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        INFO("tick " << i);
        REQUIRE(a[i] == b[i]);
    }
}

TEST_CASE("01_identical_rerun: 520-tick (10yr) hash stable", "[determinism]") {
    auto a = run(520);
    auto b = run(520);
    REQUIRE(a.back() == b.back());
}
