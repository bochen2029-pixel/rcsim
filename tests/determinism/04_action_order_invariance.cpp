#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <random>
#include <vector>

#include "rcsim/action/action.hpp"
#include "rcsim/action/queue.hpp"

// §16.1 test 04 / §5.3: action order invariance.
//
// Sorting actions through ActionCompare must produce a canonical ordering
// independent of input permutation: any shuffle, then sort, == canonical sort.

TEST_CASE("04_action_order_invariance: ActionCompare is total-order", "[determinism]") {
    using namespace rc::sim;

    std::vector<action::PendingAction> base;
    uint32_t seq = 0;
    for (uint64_t tick : {10ULL, 5ULL, 20ULL, 5ULL, 10ULL}) {
        for (state::PrincipalId pid : {0u, 1u, 2u}) {
            action::PendingAction pa;
            pa.tick = tick;
            pa.principal_id = pid;
            pa.action_seq = seq++;
            pa.priority = 0;
            pa.action = action::Pass{};
            base.push_back(pa);
        }
    }

    auto canonical = base;
    std::sort(canonical.begin(), canonical.end(), action::ActionCompare{});

    for (uint64_t seed = 1; seed <= 6; ++seed) {
        auto shuffled = base;
        std::mt19937_64 rng(seed);
        std::shuffle(shuffled.begin(), shuffled.end(), rng);
        std::sort(shuffled.begin(), shuffled.end(), action::ActionCompare{});

        REQUIRE(shuffled.size() == canonical.size());
        for (std::size_t i = 0; i < shuffled.size(); ++i) {
            REQUIRE(shuffled[i].tick         == canonical[i].tick);
            REQUIRE(shuffled[i].principal_id == canonical[i].principal_id);
            REQUIRE(shuffled[i].action_seq   == canonical[i].action_seq);
        }
    }
}
