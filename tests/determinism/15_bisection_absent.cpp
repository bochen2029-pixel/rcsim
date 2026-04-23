#include <catch2/catch_test_macros.hpp>

// §16.1, §8.2: Test 15 — integrator never splits dt.
// TODO(phase 1, §8.2, §16.1): static / link-level check that no symbol matching
//   bisect_to_gate_crossing or dt_subdivision exists; runtime check that
//   Rk4Integrator::step always returns cfg.substeps_per_tick exactly.
TEST_CASE("placeholder for 15_bisection_absent", "[determinism][pending]") {
    REQUIRE(false);
}
