#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "rcsim/action/action.hpp"
#include "rcsim/action/action_wire.hpp"
#include "rcsim/observer/measurement.hpp"

// §16.2 / §10.1a: serialized action log round-trips bit-identically.
// Bedrock property for ReplayDriver (§10.2) — Phase 4 sync log relies on it.

TEST_CASE("action_log_replayability: arbitrary action mix round-trips", "[property]") {
    using namespace rc::sim::action;
    using rc::sim::observer::ObservationField;

    std::vector<Action> log;
    log.push_back(Pass{});
    log.push_back(SeizeTerritory{1});
    log.push_back(DatacenterBuild{2, 1.0e9});
    log.push_back(CapitalAllocation{2.5e8});
    log.push_back(AIDeploymentReallocation{0.7});
    log.push_back(Sanction{4});
    {
        ManipulateObservation m;
        m.target_observer      = 5;
        m.field                = ObservationField::CPI;
        m.investment_M         = 1e7;
        m.investment_S         = 0.02;
        m.distortion_magnitude = 0.3;
        m.duration             = 12;
        log.push_back(m);
    }
    {
        AttributeDistortion ad;
        ad.accused      = 6;
        ad.field        = ObservationField::Unemployment;
        ad.investment_M = 5e6;
        ad.duration     = 8;
        log.push_back(ad);
    }

    for (const auto& a : log) {
        REQUIRE(roundtrip_bit_identical(a));
    }
}
