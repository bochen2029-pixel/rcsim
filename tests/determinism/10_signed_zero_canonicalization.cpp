#include <catch2/catch_test_macros.hpp>

// §16.1, §2.6: Test 10 — hash invariant under ±0.0.
// TODO(phase 1, §2.6, §16.1): construct WorldState with -0.0 in various fields;
//   assert canonical_hash == canonical_hash with +0.0 versions; verifies signed-zero sweep.
TEST_CASE("placeholder for 10_signed_zero_canonicalization", "[determinism][pending]") {
    REQUIRE(false);
}
