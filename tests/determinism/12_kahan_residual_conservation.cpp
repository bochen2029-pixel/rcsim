#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/fixed_point.hpp"
#include "rcsim/state/world_state.hpp"

// §16.1 test 12 / §2.7: Kahan-residual conservation across 30-yr (1560-tick) run.
//
// Sub-LSB derivative scenario: per-tick increment smaller than Q48.16 LSB.
// Without the residual buffer, naive truncation loses the increment every
// tick. With the residual sidecar, accumulated residuals surface periodically
// as quantized steps, and the total tracks N*delta within one LSB.

TEST_CASE("12_kahan_residual_conservation: sub-LSB increments accumulate",
          "[determinism]") {
    using namespace rc::sim;

    constexpr int    kTicks         = 1560;       // 30 yr at 1 wk/tick
    constexpr double kDeltaPerTick  = 1.0e-7;     // well below Q48.16 LSB ~ 1.526e-5

    state::WorldState s{};
    state::TerritoryState t{};
    t.r = 5.0; t.H = 0.0; t.G = 1e6; t.M = 1e9;
    t.Npop = 3.3e8; t.S = 1.0; t.Qelec = 100.0; t.Qliq = 100.0; t.D_local = 0.0;
    t.id = 0; t.owner = 0;
    s.grid.states.push_back(t);
    s.global.A = 1.0; s.global.S_AI_index = 1.0;

    for (int i = 0; i < kTicks; ++i) {
        s.grid.states[0].H += kDeltaPerTick;
        core::canonicalize_state_with_residual(s);
    }

    double expected = static_cast<double>(kTicks) * kDeltaPerTick;
    double actual   = s.grid.states[0].H;

    constexpr double kQ48_16_LSB = 1.0 / static_cast<double>(1 << 16);
    REQUIRE(std::abs(actual - expected) <= kQ48_16_LSB);
}

TEST_CASE("12_kahan_residual_conservation: naive truncation would lose all signal",
          "[determinism]") {
    constexpr int    kTicks = 1560;
    constexpr double kDeltaPerTick = 1.0e-7;

    double naive = 0.0;
    for (int i = 0; i < kTicks; ++i) {
        naive += kDeltaPerTick;
        double scale = static_cast<double>(1 << 16);
        naive = std::nearbyint(naive * scale) / scale;
    }
    REQUIRE(naive == 0.0);
}
