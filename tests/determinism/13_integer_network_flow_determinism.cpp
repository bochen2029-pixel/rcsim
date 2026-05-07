#include <catch2/catch_test_macros.hpp>

#include <random>

#include "rcsim/core/integer_network_flow.hpp"

// §16.1 test 13 / §4.6: Integer Network Flow determinism.
//
// Properties under test:
//   1. Same supplies + same edges (in any insertion order) yield the same
//      per-edge flow vector — driven by lex sort on (src_id, dst_id).
//   2. Solving twice gives identical results (no internal RNG, no FP).

namespace {

void build_diamond(rc::sim::core::IntegerNetworkFlow& f) {
    using rc::sim::core::FlowEdge;
    // Diamond: 0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3.
    // Supply: node 0 = +100, node 3 = -100. Net flow = 100.
    f.set_supply(0,  100);
    f.set_supply(3, -100);
    f.add_edge(FlowEdge{0, 1, 100, 1});
    f.add_edge(FlowEdge{0, 2, 100, 1});
    f.add_edge(FlowEdge{1, 3, 100, 1});
    f.add_edge(FlowEdge{2, 3, 100, 1});
}

}  // namespace

TEST_CASE("13_integer_network_flow_determinism: solve twice → identical flow",
          "[determinism]") {
    using namespace rc::sim::core;
    IntegerNetworkFlow a, b;
    build_diamond(a);
    build_diamond(b);

    auto ra = a.solve();
    auto rb = b.solve();

    REQUIRE(ra.flow.size() == rb.flow.size());
    for (std::size_t i = 0; i < ra.flow.size(); ++i) {
        INFO("edge " << i);
        REQUIRE(ra.flow[i] == rb.flow[i]);
    }
}

TEST_CASE("13_integer_network_flow_determinism: insertion order is canonicalized",
          "[determinism]") {
    using namespace rc::sim::core;

    // Insert edges in two different orders; lex sort inside add_edge should
    // produce identical internal layouts → identical flow results.
    IntegerNetworkFlow a, b;
    a.set_supply(0,  100);
    a.set_supply(3, -100);
    a.add_edge(FlowEdge{0, 1, 100, 1});
    a.add_edge(FlowEdge{0, 2, 100, 1});
    a.add_edge(FlowEdge{1, 3, 100, 1});
    a.add_edge(FlowEdge{2, 3, 100, 1});

    b.set_supply(0,  100);
    b.set_supply(3, -100);
    b.add_edge(FlowEdge{2, 3, 100, 1});
    b.add_edge(FlowEdge{1, 3, 100, 1});
    b.add_edge(FlowEdge{0, 2, 100, 1});
    b.add_edge(FlowEdge{0, 1, 100, 1});

    auto ra = a.solve();
    auto rb = b.solve();
    REQUIRE(ra.flow == rb.flow);

    // Sum of |flow| == 200 (50 along each leg of the diamond OR 100 along one
    // path; either is min-cost given uniform costs). Total throughput == 100.
    int64_t leaving_source = 0;
    for (std::size_t i = 0; i < ra.flow.size(); ++i) {
        // First two edges are 0→{1,2} (sources). Their flow leaves node 0.
    }
    leaving_source = ra.flow[0] + ra.flow[1];
    REQUIRE(leaving_source == 100);
}

TEST_CASE("13_integer_network_flow_determinism: empty network solves to empty",
          "[determinism]") {
    using namespace rc::sim::core;
    IntegerNetworkFlow f;
    auto r = f.solve();
    REQUIRE(r.flow.empty());
}
