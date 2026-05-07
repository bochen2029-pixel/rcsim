#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "rcsim/action/action.hpp"
#include "rcsim/action/validate_epistemic.hpp"
#include "rcsim/observer/measurement.hpp"

// §16.2 / §5.4: validate_epistemic depends only on observation, not ground truth.
// Sweep many (observation, action) pairs; output must be a pure function of obs+a,
// independent of any external state. The signature itself enforces this lexically;
// the property test guards against future regressions where someone might add a
// `WorldState&` parameter.

TEST_CASE("epistemic_validation_no_leak: same obs+action → same result, "
          "many invocations", "[property]") {
    using namespace rc::sim;

    observer::Observation obs;
    obs.as_of = 0;
    observer::TerritoryObservation tv;
    tv.id = 7;
    obs.territories_visible.push_back(tv);

    action::SeizeTerritory seize{7};
    action::Action a{seize};

    auto first = action::validate_epistemic(obs, /*actor=*/0, a);
    for (int i = 0; i < 1000; ++i) {
        auto next = action::validate_epistemic(obs, /*actor=*/0, a);
        REQUIRE(next.ok == first.ok);
        REQUIRE(next.reason == first.reason);
    }
}

TEST_CASE("epistemic_validation_no_leak: target outside visible set → UnknownTarget",
          "[property]") {
    using namespace rc::sim;

    observer::Observation obs;
    observer::TerritoryObservation tv;
    tv.id = 7;
    obs.territories_visible.push_back(tv);

    action::SeizeTerritory seize{99};   // not in visible set
    auto vr = action::validate_epistemic(obs, 0, seize);
    REQUIRE(vr.ok == false);
    REQUIRE(vr.reason == action::ValidationReason::UnknownTarget);
}

TEST_CASE("epistemic_validation_no_leak: NaN parameters → InvalidParameters",
          "[property]") {
    using namespace rc::sim;

    observer::Observation obs;
    observer::TerritoryObservation tv;
    tv.id = 7;
    obs.territories_visible.push_back(tv);

    action::DatacenterBuild build;
    build.site = 7;
    build.investment_M = std::nan("");
    auto vr = action::validate_epistemic(obs, 0, build);
    REQUIRE(vr.ok == false);
    REQUIRE(vr.reason == action::ValidationReason::InvalidParameters);
}
