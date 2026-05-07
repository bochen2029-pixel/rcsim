#include <catch2/catch_test_macros.hpp>

#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/hash.hpp"
#include "rcsim/state/world_state.hpp"

// §16.2 / §2.6: hash invariant under signed-zero replacement.
// Rewriting a field as -0.0 vs +0.0 (numerically equal, bit-distinct) must
// produce identical canonical_hash output — proves the §2.6 sweep runs
// before bytes flow into Blake3.

namespace {

rc::sim::state::WorldState build_one(double Qelec_seed) {
    using namespace rc::sim;
    state::WorldState s{};
    state::TerritoryState t{};
    t.r = 5.0; t.H = 1e8; t.G = 1e6; t.M = 1e9; t.Npop = 3.3e8;
    t.S = 1.0; t.Qelec = Qelec_seed; t.Qliq = 100.0; t.D_local = 0.0;
    t.id = 0; t.owner = 0;
    s.grid.states.push_back(t);
    s.global.A = 1.0; s.global.S_AI_index = 1.0;
    return s;
}

}  // namespace

TEST_CASE("hash_stability: ±0.0 produces identical hash", "[property][determinism]") {
    using namespace rc::sim;

    auto a = build_one( 0.0);
    auto b = build_one(-0.0);

    auto ha = core::canonical_hash(a);
    auto hb = core::canonical_hash(b);

    REQUIRE(ha == hb);
}

TEST_CASE("hash_stability: re-hashing same state yields same bits", "[property]") {
    using namespace rc::sim;

    auto s = build_one(100.0);
    auto h1 = core::canonical_hash(s);
    auto h2 = core::canonical_hash(s);
    auto h3 = core::canonical_hash(s);

    REQUIRE(h1 == h2);
    REQUIRE(h2 == h3);
}

TEST_CASE("hash_stability: distinct states produce distinct hashes", "[property]") {
    using namespace rc::sim;

    auto a = build_one(100.0);
    auto b = build_one(100.0001);

    auto ha = core::canonical_hash(a);
    auto hb = core::canonical_hash(b);

    REQUIRE(ha != hb);
}
