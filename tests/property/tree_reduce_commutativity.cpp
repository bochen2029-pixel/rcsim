#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include "rcsim/core/tree_reduce.hpp"

// §16.2 / §2.4: deterministic_tree_reduce determinism properties.
//
// Two properties under test:
//   1. Reducing the same canonically-ordered input twice yields the same result.
//   2. SumDouble reductions across different sizes converge correctly with identity 0.0.
//
// Note: tree-reduce is order-DEPENDENT (FP non-associativity); we explicitly do
// NOT test commutativity of permutations. The §2.4 contract is that the SAME
// pre-sorted input always produces the SAME bit pattern.

using rc::sim::core::deterministic_tree_reduce;
using rc::sim::core::SumDouble;

namespace {
uint64_t bits_of(double v) {
    uint64_t b;
    std::memcpy(&b, &v, sizeof(b));
    return b;
}
}  // namespace

TEST_CASE("tree_reduce identical input produces identical bits", "[property][determinism]") {
    std::vector<double> values{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
    auto r1 = deterministic_tree_reduce<double, SumDouble>(
        std::span<const double>(values.data(), values.size()), SumDouble{});
    auto r2 = deterministic_tree_reduce<double, SumDouble>(
        std::span<const double>(values.data(), values.size()), SumDouble{});
    REQUIRE(bits_of(r1) == bits_of(r2));
}

TEST_CASE("tree_reduce on empty span returns identity", "[property]") {
    std::vector<double> empty;
    auto r = deterministic_tree_reduce<double, SumDouble>(
        std::span<const double>(empty.data(), 0), SumDouble{});
    REQUIRE(r == 0.0);
}

TEST_CASE("tree_reduce on single element returns that element", "[property]") {
    std::vector<double> v{42.0};
    auto r = deterministic_tree_reduce<double, SumDouble>(
        std::span<const double>(v.data(), v.size()), SumDouble{});
    REQUIRE(r == 42.0);
}

TEST_CASE("tree_reduce sums power-of-two correctly", "[property]") {
    std::vector<double> v{1.0, 2.0, 4.0, 8.0};
    auto r = deterministic_tree_reduce<double, SumDouble>(
        std::span<const double>(v.data(), v.size()), SumDouble{});
    REQUIRE(r == 15.0);
}

TEST_CASE("tree_reduce sums non-power-of-two with identity padding", "[property]") {
    std::vector<double> v{1.0, 2.0, 3.0};   // padded to 4 with identity 0.0
    auto r = deterministic_tree_reduce<double, SumDouble>(
        std::span<const double>(v.data(), v.size()), SumDouble{});
    REQUIRE(r == 6.0);
}
