#include <catch2/catch_test_macros.hpp>

// §16.2, §2.7: Property — with Kahan residual, Σ mass / energy / fiat preserved.
// TODO(phase 1, §2.7, §16.2): 30-year sim; track Σ M, Σ Npop, Σ (Qelec + Qliq·η_l→e);
//   assert conservation within 1e-9 relative when no sources/sinks active in scenario.
TEST_CASE("placeholder for conservation", "[property][pending]") {
    REQUIRE(false);
}
