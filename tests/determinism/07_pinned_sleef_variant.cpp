#include <catch2/catch_test_macros.hpp>

// §16.1, §4.5: Test 07 — only pinned Sleef_exp_u10 (scalar) is called.
// TODO(phase 1, §4.5, §16.1): link-level or runtime symbol check that SIMD variants
//   (Sleef_expd2_u10avx2, Sleef_expd4_u10avx512f, etc.) are never referenced from rcsim-core.
TEST_CASE("placeholder for 07_pinned_sleef_variant", "[determinism][pending]") {
    REQUIRE(false);
}
