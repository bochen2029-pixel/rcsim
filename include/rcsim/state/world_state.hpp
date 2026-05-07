#pragma once

// §3.5: Aggregate WorldState — top-level simulation state.
// §2.7: includes CanonicalizationState (Kahan residual sidecar).
// §4.6: includes CouplingGraph for Integer Network Flow.
// §9: Advance loop reads/writes WorldState in strict 10-phase order.

#include <cstdint>
#include <vector>

#include "rcsim/core/integer_network_flow.hpp"
#include "rcsim/core/tick.hpp"
#include "rcsim/geo/adjacency.hpp"
#include "rcsim/state/territory.hpp"
#include "rcsim/state/global_state.hpp"
#include "rcsim/state/principal.hpp"
#include "rcsim/state/canonicalization_state.hpp"
#include "rcsim/state/pending.hpp"

// Forward declarations for types authored elsewhere.
namespace rc::sim::core { struct StateHash; }

namespace rc::sim::state {

// §3.2: TerritoryGrid — single contiguous AoS vector.
struct TerritoryGrid {
    std::vector<TerritoryState> states;  // size = N, default 150
};

// R-11: CouplingGraph — flow-network state derived each tick by §4.6 Integer
// Network Flow. Edges sorted lex by (src_id, dst_id). Per-territory net-flow
// aggregates are consumed as constants by ODE rhs (§4.1 RC-7..RC-8).
struct CouplingGraph {
    // Edge list (sorted). Phase 2 fills from adjacency + scenario coupling spec.
    std::vector<core::FlowEdge> edges;
    // Per-edge solved flow (parallel to `edges`), in source-field Q-format.
    std::vector<int64_t> flows;
    // Per-territory net flow aggregates. Indexed by territory_id.
    std::vector<int64_t> net_flow_Q_elec;   // Q32.32 EJ/yr
    std::vector<int64_t> net_flow_Q_liq;    // Q32.32 EJ/yr
    std::vector<int64_t> net_flow_M;        // Q56.8  USD-eq
    std::vector<int64_t> net_migration;     // Q48.16 persons/yr
};

// R-16: ApplyOutcome — distinguishes how an action terminated. Stored as
// uint32 LE in sync log per §10.1a.
enum class ApplyOutcome : uint32_t {
    Applied           = 0,
    Fizzled           = 1,
    RejectedEpistemic = 2
};

// §9.10 / §10.1 / R-16: ActionLogEntry — per-action delta for sync-log write.
struct ActionLogEntry {
    ActionLogId          id;
    core::Tick           applied_at;
    PrincipalId          principal;
    uint32_t             action_seq;
    int32_t              priority;
    std::vector<uint8_t> serialized_action;   // canonical wire format §10.1a
    ApplyOutcome         outcome;
    uint32_t             reason;              // ValidationReason or FizzleReason
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

    // R-09: AdjacencyGraph held by value per §3.5 spec text. The original
    // include-cycle worry is moot — geo/adjacency.hpp does not include
    // world_state.hpp, so direct inclusion is safe.
    geo::AdjacencyGraph adjacency;

    std::vector<PendingEffect> pending;
    std::vector<ActionLogEntry> action_log_delta;
};

// §3.5: state_hash — declared here, implemented in core/hash.cpp.
// Aliased to rc::sim::core::canonical_hash for ergonomics.
core::StateHash state_hash(const WorldState& s) noexcept;

}  // namespace rc::sim::state
