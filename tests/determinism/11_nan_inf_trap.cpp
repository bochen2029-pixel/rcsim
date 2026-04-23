#include <catch2/catch_test_macros.hpp>

// §16.1, §2.6: Test 11 — halt on non-finite.
// TODO(phase 1, §2.6, §16.1): inject NaN / ±Inf into state; assert canonicalize_for_hash
//   triggers trap_nonfinite (halt with field-identification diagnostic).
TEST_CASE("placeholder for 11_nan_inf_trap", "[determinism][pending]") {
    REQUIRE(false);
}
