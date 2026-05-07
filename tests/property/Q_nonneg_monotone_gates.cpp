#include <catch2/catch_test_macros.hpp>

#include "rcsim/dynamics/algebraics.hpp"

// §16.2 / §4.1 / §4.5: gate monotonicity + range.
// Theta_soft(k=50) and Pi_hard(k=200) are sigmoids; output in [0, 1] and
// monotone non-decreasing in argument.

TEST_CASE("Q_nonneg_monotone_gates: Theta_soft monotone non-decreasing", "[property]") {
    using rc::sim::dynamics::Theta_soft;
    double prev = Theta_soft(-1.0);
    for (double x = -1.0; x <= 1.0; x += 0.01) {
        double cur = Theta_soft(x);
        REQUIRE(cur >= 0.0);
        REQUIRE(cur <= 1.0);
        REQUIRE(cur >= prev - 1e-15);     // tolerate FP noise
        prev = cur;
    }
}

TEST_CASE("Q_nonneg_monotone_gates: Pi_hard monotone non-decreasing", "[property]") {
    using rc::sim::dynamics::Pi_hard;
    double prev = Pi_hard(-0.1);
    for (double x = -0.1; x <= 0.1; x += 0.001) {
        double cur = Pi_hard(x);
        REQUIRE(cur >= 0.0);
        REQUIRE(cur <= 1.0);
        REQUIRE(cur >= prev - 1e-15);
        prev = cur;
    }
}

TEST_CASE("Q_nonneg_monotone_gates: gate values bounded at extremes", "[property]") {
    using rc::sim::dynamics::Theta_soft;
    using rc::sim::dynamics::Pi_hard;

    // At very negative argument, gate ~ 0; at very positive, gate ~ 1.
    REQUIRE(Theta_soft(-10.0) < 1.0e-10);
    REQUIRE(Theta_soft( 10.0) > 1.0 - 1.0e-10);
    REQUIRE(Pi_hard(  -1.0)   < 1.0e-10);
    REQUIRE(Pi_hard(   1.0)   > 1.0 - 1.0e-10);
}
