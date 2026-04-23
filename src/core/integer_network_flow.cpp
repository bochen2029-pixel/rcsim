#include "rcsim/core/integer_network_flow.hpp"

// §4.6: Integer Network Flow — deterministic replacement for FP LP.
// TODO(phase 2, §4.6): implement via LEMON Network Simplex OR custom push-relabel (~500 LOC).
// Lexicographic tie-breaking on (src_id, dst_id) for determinism across compilers/platforms.

namespace rc::sim::core {

void IntegerNetworkFlow::set_supply(uint32_t /*territory_id*/, int64_t /*supply*/) noexcept {
    // TODO(phase 2, §4.6)
}

void IntegerNetworkFlow::add_edge(const FlowEdge& /*edge*/) noexcept {
    // TODO(phase 2, §4.6): insert lex-sorted on (src_id, dst_id)
}

FlowResult IntegerNetworkFlow::solve() noexcept {
    // TODO(phase 2, §4.6): min-cost flow with deterministic tie-breaking
    return FlowResult{};
}

}  // namespace rc::sim::core
