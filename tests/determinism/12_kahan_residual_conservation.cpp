#include <catch2/catch_test_macros.hpp>

// §16.1, §2.7: Test 12 — 30-year First Law test.
// TODO(phase 1, §2.7, §16.1): run 30yr sim with Kahan residual enabled/disabled;
//   assert conservation (mass / energy / fiat Σ within tight tolerance) holds only with residual.
TEST_CASE("placeholder for 12_kahan_residual_conservation", "[determinism][pending]") {
    REQUIRE(false);
}
