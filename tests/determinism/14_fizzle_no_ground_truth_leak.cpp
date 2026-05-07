#include <catch2/catch_test_macros.hpp>

#include <utility>
#include <vector>

#include "rcsim/action/action.hpp"
#include "rcsim/action/apply_with_fizzle.hpp"
#include "rcsim/action/validate_epistemic.hpp"
#include "rcsim/observer/measurement.hpp"
#include "rcsim/state/world_state.hpp"

// §16.1 test 14 / §5.4: Fizzle preserves fog-of-war.
//
// v1.3's fix to the Epistemic Oracle Leak. Two scenarios with identical
// principal observation but different ground truth must produce:
//   - identical validate_epistemic outcomes (validation only sees obs)
//   - identical resource deductions (deduct_resources runs unconditionally)
// Apply outcomes diverge (Applied vs Fizzled) — that path is information-bearing
// but leaks only "action consumed resources with no effect," not the specific
// ground-truth state.

namespace {

rc::sim::observer::Observation make_obs_with_target(rc::sim::state::TerritoryId tid) {
    rc::sim::observer::Observation o;
    o.as_of = 0;
    rc::sim::observer::TerritoryObservation tv;
    tv.id = tid;
    tv.r_reported = 5.0;
    tv.G_reported = 1e6;
    tv.Qelec_reported = 100.0;
    tv.Qliq_reported  = 100.0;
    o.territories_visible.push_back(tv);
    return o;
}

}  // namespace

TEST_CASE("14_fizzle_no_ground_truth_leak: validation depends only on observation",
          "[determinism]") {
    using namespace rc::sim;

    auto obs = make_obs_with_target(42);

    // Two distinct ground-truth states (constructed but never passed to validator).
    state::WorldState gt_present{};
    state::TerritoryState t_present{};
    t_present.id = 42; t_present.owner = 99;
    gt_present.grid.states.push_back(t_present);

    state::WorldState gt_absent{};   // territory 42 not present in ground truth.

    action::SeizeTerritory seize{42};

    // validate_epistemic takes only obs + actor + action; ground truth is invisible.
    auto vr_present = action::validate_epistemic(obs, /*actor=*/0, seize);
    auto vr_absent  = action::validate_epistemic(obs, /*actor=*/0, seize);

    REQUIRE(vr_present.ok     == vr_absent.ok);
    REQUIRE(vr_present.reason == vr_absent.reason);
    REQUIRE(vr_present.ok     == true);
}

TEST_CASE("14_fizzle_no_ground_truth_leak: deduct_resources independent of ground truth",
          "[determinism]") {
    using namespace rc::sim;

    state::PrincipalState p1{};
    p1.id = 0; p1.treasury_M = 1.0e10; p1.legitimacy_S = 1.0;
    state::PrincipalState p2 = p1;

    action::DatacenterBuild build{42, 1.0e9};

    action::deduct_resources(p1, build);
    action::deduct_resources(p2, build);
    REQUIRE(p1.treasury_M == p2.treasury_M);
    REQUIRE(p1.treasury_M == 1.0e10 - 1.0e9);
}

TEST_CASE("14_fizzle_no_ground_truth_leak: apply diverges Applied vs Fizzled",
          "[determinism]") {
    using namespace rc::sim;

    // Ground truth A: territory 42 exists → Applied.
    state::WorldState gt_a{};
    state::TerritoryState t{};
    t.id = 42;
    gt_a.grid.states.push_back(t);

    // Ground truth B: territory 42 does NOT exist → Fizzled.
    state::WorldState gt_b{};

    action::DatacenterBuild build{42, 1.0e9};
    auto ra = action::apply(std::move(gt_a), 0, build);
    auto rb = action::apply(std::move(gt_b), 0, build);

    REQUIRE(ra.applied == true);
    REQUIRE(ra.fizzled == false);
    REQUIRE(rb.applied == false);
    REQUIRE(rb.fizzled == true);
    REQUIRE(rb.reason  == action::FizzleReason::GeographyImpossible);
}
