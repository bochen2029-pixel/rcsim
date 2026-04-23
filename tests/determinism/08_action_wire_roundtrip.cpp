#include <catch2/catch_test_macros.hpp>

// §16.1, §10.1a: Test 08 — action wire-format round-trip bit-identical across compilers.
// TODO(phase 4, §10.1a, §16.1): for each Action variant, assert
//   deserialize(serialize(a)) == a, byte-for-byte stable across GCC/Clang/MSVC.
TEST_CASE("placeholder for 08_action_wire_roundtrip", "[determinism][pending]") {
    REQUIRE(false);
}
