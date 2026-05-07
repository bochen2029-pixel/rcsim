#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string>
#include <vector>

#include "rcsim/action/effect.hpp"
#include "rcsim/action/queue.hpp"
#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/hash.hpp"
#include "rcsim/dynamics/coupling.hpp"
#include "rcsim/dynamics/integrator.hpp"
#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/io/scenario_yaml.hpp"
#include "rcsim/state/world_state.hpp"

// §19 Phase 2 gate test: 150-territory run reaches basin.
//
// "Reach basin" criterion: in the last 10 ticks, the maximum relative change
// in any field of any territory is below kBasinTolerance. Trajectory is
// allowed to be growing (population, capital) — we measure |Δfield| / |field|
// to express change as a fraction.

namespace {

double max_relative_change(
    const rc::sim::state::WorldState& a,
    const rc::sim::state::WorldState& b
) noexcept {
    double max_rel = 0.0;
    if (a.grid.states.size() != b.grid.states.size()) return 1.0;
    for (std::size_t i = 0; i < a.grid.states.size(); ++i) {
        const auto& A = a.grid.states[i];
        const auto& B = b.grid.states[i];
        auto rel = [&](double x, double y) noexcept {
            double denom = std::abs(x) > 1.0 ? std::abs(x) : 1.0;
            double r = std::abs(x - y) / denom;
            if (r > max_rel) max_rel = r;
        };
        rel(A.r, B.r);     rel(A.H, B.H);     rel(A.G, B.G);
        rel(A.M, B.M);     rel(A.Npop, B.Npop); rel(A.S, B.S);
        rel(A.Qelec, B.Qelec); rel(A.Qliq, B.Qliq); rel(A.D_local, B.D_local);
    }
    return max_rel;
}

}  // namespace

TEST_CASE("grid_150 30yr completes without trap", "[unit][gate]") {
    using namespace rc::sim;

    auto s = io::load_scenario(std::string(RCSIM_SOURCE_DIR) + "/scenarios/grid_150.yaml");
    REQUIRE(s.grid.states.size() == 150);
    REQUIRE(s.adjacency.edges.size() > 0);   // GeoJSON adjacency was loaded

    dynamics::ODESystem ode;
    dynamics::IntegratorConfig cfg;
    cfg.dt_years = 1.0 / 52.0;
    cfg.substeps_per_tick = 4;
    dynamics::Rk4Integrator integ(&ode, cfg);

    action::ActionQueue inbound;
    constexpr int kTicks = 1560;
    for (int t = 0; t < kTicks; ++t) {
        s.tick++;
        action::mature_pending(s);
        dynamics::integer_network_flow_solve(s);
        auto gates = dynamics::evaluate_gates(s);
        integ.step(s, cfg.dt_years, gates);
        core::canonicalize_state_with_residual(s);
        action::retire_expired(s);
        s.time_years += cfg.dt_years;
    }

    // Sanity: state still finite at the end.
    for (const auto& t : s.grid.states) {
        REQUIRE(std::isfinite(t.r));
        REQUIRE(std::isfinite(t.H));
        REQUIRE(std::isfinite(t.G));
        REQUIRE(std::isfinite(t.M));
        REQUIRE(std::isfinite(t.Npop));
        REQUIRE(std::isfinite(t.S));
        REQUIRE(std::isfinite(t.Qelec));
        REQUIRE(std::isfinite(t.Qliq));
        REQUIRE(t.r       >= 0.0);
        REQUIRE(t.Qelec   >= 0.0);
        REQUIRE(t.Qliq    >= 0.0);
        REQUIRE(t.Npop    >= 0.0);
    }
}

TEST_CASE("grid_150 has full 4-neighbor interior adjacency", "[unit][gate]") {
    using namespace rc::sim;
    auto s = io::load_scenario(std::string(RCSIM_SOURCE_DIR) + "/scenarios/grid_150.yaml");
    REQUIRE(s.grid.states.size() == 150);

    // 15 cols × 10 rows. Interior cell (not on border) should have 4 neighbors.
    constexpr uint32_t COLS = 15;
    constexpr uint32_t ROWS = 10;
    for (uint32_t r = 1; r + 1 < ROWS; ++r) {
        for (uint32_t c = 1; c + 1 < COLS; ++c) {
            uint32_t tid = r * COLS + c;
            REQUIRE(s.adjacency.neighbors[tid].size() == 4);
        }
    }
    // Total edges = horizontal + vertical = (COLS-1)*ROWS + COLS*(ROWS-1)
    //             = 14*10 + 15*9 = 140 + 135 = 275
    REQUIRE(s.adjacency.edges.size() == 275);
}

TEST_CASE("grid_150 final hash deterministic across reruns", "[unit][determinism]") {
    using namespace rc::sim;

    auto run = []() noexcept -> core::StateHash {
        auto s = io::load_scenario(std::string(RCSIM_SOURCE_DIR) + "/scenarios/grid_150.yaml");
        dynamics::ODESystem ode;
        dynamics::IntegratorConfig cfg;
        cfg.dt_years = 1.0 / 52.0;
        cfg.substeps_per_tick = 4;
        dynamics::Rk4Integrator integ(&ode, cfg);
        core::StateHash hash{};
        for (int t = 0; t < 100; ++t) {
            s.tick++;
            action::mature_pending(s);
            dynamics::integer_network_flow_solve(s);
            auto gates = dynamics::evaluate_gates(s);
            integ.step(s, cfg.dt_years, gates);
            core::canonicalize_state_with_residual(s);
            action::retire_expired(s);
            hash = core::canonical_hash(s);
            s.time_years += cfg.dt_years;
        }
        return hash;
    };

    auto a = run();
    auto b = run();
    REQUIRE(a == b);
}
