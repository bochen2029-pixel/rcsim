#include <catch2/catch_test_macros.hpp>

#include "rcsim/dynamics/integrator.hpp"
#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/state/world_state.hpp"

// §16.2 / §3.3 / §4.2: Diamond Floor — A monotone non-decreasing.
// Random initial states + 100-tick advance loop; assert A never decreases.

namespace {

rc::sim::state::WorldState build(double A0, double S0) {
    using namespace rc::sim;
    state::WorldState s{};
    state::TerritoryState t{};
    t.r = 5.0; t.H = 1e8; t.G = 1e6; t.M = 1e9; t.Npop = 3.3e8;
    t.S = S0; t.Qelec = 100.0; t.Qliq = 100.0; t.D_local = 0.0;
    t.id = 0; t.owner = 0;
    s.grid.states.push_back(t);
    s.global.A = A0;
    s.global.S_AI_index = S0;
    s.global.D_global = 0.0;
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

}  // namespace

TEST_CASE("A_monotone: capability ratio non-decreasing over 100 ticks", "[property]") {
    using namespace rc::sim;

    for (double A0 : {1.0, 1.5, 3.0, 10.0}) {
        state::WorldState s = build(A0, 1.0);
        dynamics::ODESystem ode;
        dynamics::IntegratorConfig cfg;
        cfg.dt_years = 1.0 / 52.0;
        cfg.substeps_per_tick = 4;
        dynamics::Rk4Integrator integ(&ode, cfg);

        double A_prev = s.global.A;
        for (int t = 0; t < 100; ++t) {
            auto gates = dynamics::evaluate_gates(s);
            integ.step(s, cfg.dt_years, gates);
            INFO("tick " << t << " A_prev=" << A_prev << " A_now=" << s.global.A);
            REQUIRE(s.global.A >= A_prev);
            A_prev = s.global.A;
        }
    }
}
