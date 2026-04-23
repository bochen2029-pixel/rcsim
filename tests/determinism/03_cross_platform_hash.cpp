#include <catch2/catch_test_macros.hpp>

// §16.1, §15.1: CI matrix test — GCC 13 / Clang 17 / MSVC 19.40 produce identical hashes.
// TODO(phase 1, §16.1, §15.1): compare emitted hash chain against a golden-master file
//   committed to data/golden_hashes.bin (to be populated once phase 1 hashes stabilize).
TEST_CASE("placeholder for 03_cross_platform_hash", "[determinism][pending]") {
    REQUIRE(false);
}
