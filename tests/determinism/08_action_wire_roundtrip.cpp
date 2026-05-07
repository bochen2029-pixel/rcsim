#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "rcsim/action/action.hpp"
#include "rcsim/action/action_wire.hpp"
#include "rcsim/observer/measurement.hpp"

// §16.1 test 08 / §10.1a: action canonical wire format round-trip.
//
// For each action variant we exercise:
//   1. serialize → deserialize → serialize yields identical bytes.
//   2. variant_index after round-trip matches the original.
//
// The byte-stream itself is golden-master across compilers; once CI matrix
// runs, commit a per-variant byte vector here as the reference.

namespace {

void check_roundtrip(const rc::sim::action::Action& a) {
    using namespace rc::sim::action;
    REQUIRE(roundtrip_bit_identical(a));
    auto bytes = serialize_canonical(a);
    Action again = deserialize_canonical(std::span<const uint8_t>(bytes.data(), bytes.size()));
    REQUIRE(again.index() == a.index());
}

}  // namespace

TEST_CASE("08_action_wire_roundtrip: payload-less variants", "[determinism]") {
    using namespace rc::sim::action;
    check_roundtrip(Pass{});
    check_roundtrip(CurrencySwap{});
    check_roundtrip(VirtualizationPush{});
    check_roundtrip(MobilizeMilitary{});
    check_roundtrip(CeaseFire{});
}

TEST_CASE("08_action_wire_roundtrip: territory-target variants", "[determinism]") {
    using namespace rc::sim::action;
    check_roundtrip(SeizeTerritory{42});
    check_roundtrip(Blockade{7});
    check_roundtrip(ExportControl{99});
    check_roundtrip(SovereignExpropriation{1234});
}

TEST_CASE("08_action_wire_roundtrip: investment-bearing variants", "[determinism]") {
    using namespace rc::sim::action;
    check_roundtrip(DatacenterBuild{42, 1.5e9});
    check_roundtrip(BuildInfrastructure{7, 3.0e8});
    check_roundtrip(CapitalAllocation{1.0e9});
    check_roundtrip(MonetaryIssuance{5.0e8});
}

TEST_CASE("08_action_wire_roundtrip: AI deployment", "[determinism]") {
    using namespace rc::sim::action;
    check_roundtrip(AIDeploymentReallocation{0.42});
}

TEST_CASE("08_action_wire_roundtrip: sanction", "[determinism]") {
    using namespace rc::sim::action;
    check_roundtrip(Sanction{17});
}

TEST_CASE("08_action_wire_roundtrip: ManipulateObservation", "[determinism]") {
    using namespace rc::sim::action;
    ManipulateObservation m;
    m.target_observer      = 3;
    m.field                = rc::sim::observer::ObservationField::CPI;
    m.investment_M         = 1.0e8;
    m.investment_S         = 0.05;
    m.distortion_magnitude = 0.5;
    m.duration             = 52;
    check_roundtrip(m);
}

TEST_CASE("08_action_wire_roundtrip: AttributeDistortion", "[determinism]") {
    using namespace rc::sim::action;
    AttributeDistortion ad;
    ad.accused      = 5;
    ad.field        = rc::sim::observer::ObservationField::S_AI_index;
    ad.investment_M = 5.0e7;
    ad.duration     = 26;
    check_roundtrip(ad);
}

TEST_CASE("08_action_wire_roundtrip: SeizeTerritory wire bytes are stable",
          "[determinism]") {
    using namespace rc::sim::action;
    auto bytes = serialize_canonical(SeizeTerritory{42});
    // variant_index 0 (LE): 00 00 00 00; target 42 (LE): 2a 00 00 00.
    REQUIRE(bytes.size() == 8);
    REQUIRE(bytes[0] == 0x00); REQUIRE(bytes[1] == 0x00);
    REQUIRE(bytes[2] == 0x00); REQUIRE(bytes[3] == 0x00);
    REQUIRE(bytes[4] == 0x2a); REQUIRE(bytes[5] == 0x00);
    REQUIRE(bytes[6] == 0x00); REQUIRE(bytes[7] == 0x00);
}
