#pragma once

// §4.6: Coupling flows — Integer Network Flow (replaces floating-point LP).
// DESIGN_v1.3.md §4.6: No bit-exact cross-platform floating-point LP solver exists.
// Simplex and Interior Point methods resolve degenerate optimal bases via epsilon
// tolerances that vary across BLAS/LAPACK implementations and compilers.
// Fix: Integer Network Flow operating on quantized Q-format integer surpluses.
// LEMON Graph Library (BSD, §15) provides deterministic Network Simplex; or implement
// push-relabel directly (~500 LOC).

#include <cstdint>
#include <vector>

namespace rc::sim::core {

// §4.6: Edge in the flow network.
// Edges sorted lexicographically by (src_id, dst_id) for determinism.
struct FlowEdge {
    uint32_t src_id;      // territory_id of source
    uint32_t dst_id;      // territory_id of destination
    int64_t  capacity;    // Q-format integer (same format as the field being coupled)
    int64_t  cost;        // Q-format integer
};

// §4.6: Result — per-edge flow in Q-format integer.
struct FlowResult {
    std::vector<int64_t> flow;   // index parallel to edges input
};

// §4.6: IntegerNetworkFlow — deterministic min-cost flow over 64-bit signed Q-format integers.
// Tie-breaking: when multiple flow paths have equal cost, select path with
// lexicographically smallest (src_id, dst_id) sequence. Deterministic across compilers/platforms.
class IntegerNetworkFlow {
public:
    // §4.6: construct from nodes + sorted edges + supply/demand vector.
    // TODO(phase 2, §4.6): implement per DESIGN_v1.3.md §4.6
    IntegerNetworkFlow() noexcept = default;

    // §4.6: add node supply (positive) or demand (negative).
    // TODO(phase 2, §4.6): implement
    void set_supply(uint32_t territory_id, int64_t supply) noexcept;

    // §4.6: add edge with deterministic lex-sorted insertion.
    // TODO(phase 2, §4.6): implement
    void add_edge(const FlowEdge& edge) noexcept;

    // §4.6: solve min-cost flow with deterministic tie-breaking.
    // Returns per-edge flow in Q-format integers.
    // TODO(phase 2, §4.6): implement via LEMON Network Simplex or custom push-relabel
    FlowResult solve() noexcept;

    // Internal state placeholder; real implementation holds LEMON graph
    // or custom push-relabel data structures.
    // TODO(phase 2, §4.6): populate at first implementation
    std::vector<int64_t> supplies_;
    std::vector<FlowEdge> edges_;
};

}  // namespace rc::sim::core
