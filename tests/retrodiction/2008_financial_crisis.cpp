#include <catch2/catch_test_macros.hpp>

// §16.3: Retrodiction — 2008 financial crisis. Reports RMSE; not pass/fail.
// TODO(phase 6, §16.3): configure scenario for 2007 initial conditions; run 3 years;
//   compare M / CPI / unemployment trajectories against FRED data.
TEST_CASE("placeholder for 2008_financial_crisis", "[retrodiction][pending]") {
    REQUIRE(false);
}
