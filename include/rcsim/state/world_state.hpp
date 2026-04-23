#pragma once

// §3.5: Aggregate WorldState — top-level simulation state.
// §2.7: includes CanonicalizationState (Kahan residual sidecar).
// §4.6: includes CouplingGraph for Integer Network Flow.
// §9: Advance loop reads/writes WorldState in strict 10-phase order.

#include <vector>

#include "rcsim/core/tick.hpp"
#include "rcsim/state/territory.hpp"
#include "rcsim/state/global_state.hpp"
#include "rcsim/state/principal.hpp"
#include "rcsim/state/canonicalization_state.hpp"
#include "rcsim/state/pending.hpp"

// Forward declarations for types authored elsewhere.
namespace rc::sim::core { struct StateHash; }
namespace rc::sim::geo  { struct AdjacencyGraph; }

namespace rc::sim::state {

// §3.2: TerritoryGrid — single contiguous AoS vector.
struct TerritoryGrid {
    std::vector<TerritoryState> states;  // size = N, default 150
};

// §4.6: CouplingGraph — holds the flow-network state derived each tick.
// TODO(phase 2, §4.6): populate with edges + solved flows from Integer Network Flow.
struct CouplingGraph {
    // SPEC_AMBIGUOUS(§4.6): structure not enumerated. Will hold:
    //   - edge list sorted lex by (src_id, dst_id)
    //   - per-edge flows in Q-format integer (produced by IntegerNetworkFlow::solve)
    //   - per-territory net flow aggregates consumed by ODE rhs as constants.
    std::vector<int64_t> flows;   // placeholder; per-edge Q-format flow results
};

// §9.10, §10.1: ActionLogEntry — per-action delta for sync-log write.
// TODO(phase 4, §10.1): flesh out sync-log entry format.
struct ActionLogEntry {
    ActionLogId id;
    core::Tick applied_at;
    PrincipalId principal;
    // SPEC_AMBIGUOUS(§10.1): full entry fields (serialized Action payload,
    //   rejection/fizzle flag, reason code) deferred to phase 4 action_wire.cpp.
};

// §3.5: WorldState.
struct WorldState {
    core::TickNumber tick;
    double time_years;

    GlobalState global;
    TerritoryGrid grid;

    // §2.7: canonicalization sidecar (Kahan residuals, non-hashed).
    CanonicalizationState canon;

    std::vector<PrincipalState> principals;

    // §4.6: coupling graph for Integer Network Flow.
    CouplingGraph coupling;

    // §12, geo/adjacency.hpp: geographic adjacency derived from GeoJSON.
    // Use pointer to avoid circular include; geo/adjacency.hpp has the full type.
    // SPEC_AMBIGUOUS(§3.5): spec shows `AdjacencyGraph adjacency;` by value; scaffold
    //   uses pointer to dodge the include chain. Resolve once geo/adjacency.hpp is
    //   populated with a complete (non-forward) type.
    geo::AdjacencyGraph* adjacency;

    std::vector<PendingEffect> pending;
    std::vector<ActionLogEntry> action_log_delta;
};

// §3.5: state_hash — declared here, implemented in core/hash.cpp.
// Aliased to rc::sim::core::canonical_hash for ergonomics.
core::StateHash state_hash(const WorldState& s) noexcept;

}  // namespace rc::sim::state
