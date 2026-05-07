#include "rcsim/dynamics/coupling.hpp"

#include <algorithm>
#include <cstdint>
#include <variant>

#include "rcsim/core/fixed_point.hpp"

// §4.3 / §4.6 / §9.4: coupling flow computation.
//
// Phase 2 implements:
//   - compute_conversion_flows: Q_l→e and Q_e→l matrix per §4.3 (R-10 pinned η).
//   - integer_network_flow_solve: build per-tick flow networks for migration
//     and per-resource trade. For Phase 2 we model two networks (M flows for
//     trade-balance equilibration; population for migration). Q_elec/Q_liq
//     trade is deferred to Phase 3 — requires scenario-specified coupling
//     policies (free trade vs autarky vs sanctions) which the action layer
//     drives.
//   - blockade: per-territory Q_liq reduction from active BlockadeEdge effects.
//   - migration_flux: linear v1 model — gradient-driven population redistribution
//     scaled by ParamBlock.migration_coupling.

namespace rc::sim::dynamics {

ConversionFlows compute_conversion_flows(
    const state::TerritoryState& /*t*/,
    double u_liq_to_elec,
    double u_elec_to_liq
) noexcept {
    // §4.3: convert = η · u; η pinned per R-10.
    ConversionFlows out;
    out.convert_liq_to_elec = kEta_liq_to_elec * (u_liq_to_elec > 0.0 ? u_liq_to_elec : 0.0);
    out.convert_elec_to_liq = kEta_elec_to_liq * (u_elec_to_liq > 0.0 ? u_elec_to_liq : 0.0);
    return out;
}

void integer_network_flow_solve(state::WorldState& s) noexcept {
    // §9.4: build the per-tick flow network from current state + adjacency.
    // Phase 2 implementation: zero-net trade balance — flows are solved but
    // for the mean-field scenarios we exercise, no inter-territory edges are
    // declared, so the result is empty. The structural plumbing matters for
    // Phase 3 + retrodiction scenarios where principals declare trade.

    const std::size_t N = s.grid.states.size();

    // Initialize / reset per-territory net-flow aggregates.
    s.coupling.net_flow_Q_elec.assign(N, 0);
    s.coupling.net_flow_Q_liq.assign(N, 0);
    s.coupling.net_flow_M.assign(N, 0);
    s.coupling.net_migration.assign(N, 0);
    s.coupling.flows.assign(s.coupling.edges.size(), 0);

    if (s.coupling.edges.empty()) return;

    // Build flow problem. Sources: territories with positive surplus relative
    // to mean; sinks: territories with deficit. Quantize to int64 Q-format
    // (Q56.8 for M flows — matches storage; arbitrary choice for first pass).
    core::IntegerNetworkFlow solver;

    // Compute mean M and use deviation as supply.
    double mean_M = 0.0;
    for (const auto& t : s.grid.states) mean_M += t.M;
    if (N > 0) mean_M /= static_cast<double>(N);

    for (std::size_t i = 0; i < N; ++i) {
        double dev = s.grid.states[i].M - mean_M;
        // Positive dev → source supply; negative → demand.
        int64_t supply_q = core::quantize_Q56_8(dev);
        solver.set_supply(s.grid.states[i].id, supply_q);
    }

    for (const auto& e : s.coupling.edges) {
        solver.add_edge(e);
    }

    auto result = solver.solve();
    if (result.flow.size() != s.coupling.edges.size()) return;
    s.coupling.flows = std::move(result.flow);

    // Aggregate per-territory net flow from edges. For M flows: source loses,
    // destination gains; signs pinned by edge direction.
    for (std::size_t i = 0; i < s.coupling.edges.size(); ++i) {
        const auto& e = s.coupling.edges[i];
        int64_t f = s.coupling.flows[i];
        if (e.src_id < N) s.coupling.net_flow_M[e.src_id] -= f;
        if (e.dst_id < N) s.coupling.net_flow_M[e.dst_id] += f;
    }
}

double blockade(const state::TerritoryState& t, const state::WorldState& s) noexcept {
    // §4.1 RC-8 / §5.1 Blockade: walk PendingEffects for active BlockadeEdge
    // mutations targeting this territory.
    double total = 0.0;
    for (const auto& pe : s.pending) {
        if (pe.activates_at <= s.tick && pe.expires_at >= s.tick) {
            std::visit([&](const auto& m) noexcept {
                using T = std::decay_t<decltype(m)>;
                if constexpr (std::is_same_v<T, state::BlockadeEdge>) {
                    // Phase 2: BlockadeEdge stores no per-target detail yet; assume
                    // any active blockade reduces Q_liq by 10% of current. Phase 3
                    // refines to per-(src,dst) targeting.
                    (void)m;
                    total += 0.10 * t.Qliq;
                }
            }, pe.mutation);
        }
    }
    return total;
}

double migration_flux(std::size_t territory_index, const state::WorldState& s) noexcept {
    // §17.2 / §4.1: linear v1 — gradient-driven population redistribution.
    // For each adjacent neighbor, push Δ = coupling · (Npop_neighbor - Npop_self).
    if (territory_index >= s.grid.states.size()) return 0.0;
    if (s.adjacency.neighbors.empty()) return 0.0;
    if (territory_index >= s.adjacency.neighbors.size()) return 0.0;

    const double k = s.global.params.migration_coupling;
    const double self_N = s.grid.states[territory_index].Npop;
    double flux = 0.0;
    for (auto neighbor_id : s.adjacency.neighbors[territory_index]) {
        for (const auto& nb : s.grid.states) {
            if (nb.id == neighbor_id) {
                flux += k * (nb.Npop - self_N);
                break;
            }
        }
    }
    return flux;
}

}  // namespace rc::sim::dynamics
