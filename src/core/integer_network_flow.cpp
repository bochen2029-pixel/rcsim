#include "rcsim/core/integer_network_flow.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>

#include <lemon/list_graph.h>
#include <lemon/network_simplex.h>

// §4.6: Integer Network Flow — deterministic replacement for FP LP.
//
// Implementation: LEMON Graph Library's NetworkSimplex on int64 capacities and
// costs. LEMON guarantees integer-arithmetic determinism given fixed insertion
// order; we enforce that order by sorting edges lexicographically on
// (src_id, dst_id) before inserting them, and by inserting nodes in
// territory_id ascending order.
//
// Pivot rule: NetworkSimplex's default (BLOCK_SEARCH) traverses arcs in
// insertion order, so deterministic insertion → deterministic pivot choice
// → deterministic result. For belt-and-braces we explicitly select
// FIRST_ELIGIBLE which has the simplest pivot rule and the strongest
// determinism guarantee.

namespace rc::sim::core {

void IntegerNetworkFlow::set_supply(uint32_t territory_id, int64_t supply) noexcept {
    // Resize supply vector if needed; index by territory_id.
    if (territory_id >= supplies_.size()) {
        supplies_.resize(static_cast<std::size_t>(territory_id) + 1, 0);
    }
    supplies_[territory_id] = supply;
}

void IntegerNetworkFlow::add_edge(const FlowEdge& edge) noexcept {
    // Insert lex-sorted on (src_id, dst_id).
    auto pos = std::upper_bound(edges_.begin(), edges_.end(), edge,
        [](const FlowEdge& a, const FlowEdge& b) noexcept {
            if (a.src_id != b.src_id) return a.src_id < b.src_id;
            return a.dst_id < b.dst_id;
        });
    edges_.insert(pos, edge);
}

FlowResult IntegerNetworkFlow::solve() noexcept {
    FlowResult result;
    if (edges_.empty()) return result;

    // Determine the unique node set (every territory_id appearing as src or dst,
    // plus every territory with a non-zero supply).
    std::vector<uint32_t> ids;
    ids.reserve(supplies_.size() + 2 * edges_.size());
    for (std::size_t i = 0; i < supplies_.size(); ++i) {
        if (supplies_[i] != 0) ids.push_back(static_cast<uint32_t>(i));
    }
    for (const auto& e : edges_) {
        ids.push_back(e.src_id);
        ids.push_back(e.dst_id);
    }
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

    // Build LEMON digraph. Insert nodes in id-ascending order so LEMON's
    // internal indexing matches our ordering.
    using Graph = lemon::ListDigraph;
    Graph g;
    std::unordered_map<uint32_t, Graph::Node> id_to_node;
    id_to_node.reserve(ids.size());
    for (uint32_t id : ids) {
        id_to_node.emplace(id, g.addNode());
    }

    Graph::ArcMap<int64_t> capacity(g);
    Graph::ArcMap<int64_t> cost(g);
    Graph::NodeMap<int64_t> supply(g, 0);

    // Set per-node supply.
    for (uint32_t id : ids) {
        int64_t s = (id < supplies_.size()) ? supplies_[id] : 0;
        supply[id_to_node[id]] = s;
    }

    // Insert arcs in the same lex-sorted order they live in `edges_`. Build
    // a parallel vector of arcs so we can read flows back in input order.
    std::vector<Graph::Arc> arcs;
    arcs.reserve(edges_.size());
    for (const auto& e : edges_) {
        Graph::Arc a = g.addArc(id_to_node[e.src_id], id_to_node[e.dst_id]);
        capacity[a] = e.capacity;
        cost[a]     = e.cost;
        arcs.push_back(a);
    }

    using NS = lemon::NetworkSimplex<Graph, int64_t, int64_t>;
    NS ns(g);
    ns.upperMap(capacity).costMap(cost).supplyMap(supply);
    auto status = ns.run(NS::FIRST_ELIGIBLE);

    result.flow.resize(edges_.size(), 0);
    if (status == NS::OPTIMAL) {
        for (std::size_t i = 0; i < arcs.size(); ++i) {
            result.flow[i] = ns.flow(arcs[i]);
        }
    }
    // INFEASIBLE / UNBOUNDED: return zero-flow result. Caller (coupling.cpp)
    // detects this by sum-of-flows == 0 when supply was non-zero, and treats
    // it as "no coupling this tick" — graceful degradation rather than halt.

    return result;
}

}  // namespace rc::sim::core
