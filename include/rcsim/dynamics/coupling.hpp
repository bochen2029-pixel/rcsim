#pragma once

// §4.3: Q_elec ↔ Q_liq conversion matrix (EJ/yr flows, η_l→e ≈ 0.35, η_e→l ≈ 0.6).
// §4.6: Integer Network Flow for coupling flows (replaces FP LP).
// §9.4: Phase 4 — Compute coupling flows via Integer Network Flow.

#include "rcsim/core/integer_network_flow.hpp"
#include "rcsim/state/world_state.hpp"

namespace rc::sim::dynamics {

// §4.3: Conversion-efficiency constants.
// SPEC_AMBIGUOUS(§4.3): spec gives defaults ~0.35 and ~0.6 but not pinned values.
//   Populate from CALIBRATION.md output (§19.1 phase 7b refinement).
inline constexpr double kEta_liq_to_elec = 0.35;
inline constexpr double kEta_elec_to_liq = 0.60;

// §4.3, §4.6: Apply Q_elec ↔ Q_liq conversion for a single territory.
//   convert = η · u  where η is the per-direction matrix and u is the flow rate (EJ/yr).
// TODO(phase 2, §4.3): implement per DESIGN_v1.3.md §4.3
struct ConversionFlows {
    double convert_liq_to_elec;   // EJ/yr added to Q_elec budget, removed from Q_liq
    double convert_elec_to_liq;   // EJ/yr added to Q_liq budget, removed from Q_elec
};
ConversionFlows compute_conversion_flows(
    const state::TerritoryState& t,
    double u_liq_to_elec,
    double u_elec_to_liq
) noexcept;

// §4.6, §9.4: solve per-tick coupling flows.
//   1. Read quantized (integer) Q-format state values.
//   2. Construct flow network: nodes = territories, edges = coupling_M / coupling_Q.
//   3. Edge costs and capacities are 64-bit integers (Q-format directly).
//   4. Solve min-cost flow with deterministic tie-breaking.
// TODO(phase 2, §4.6, §9.4): implement per DESIGN_v1.3.md §4.6
void integer_network_flow_solve(state::WorldState& s) noexcept;

// §4.1 RC-8: blockade_i — per-territory blockade effect on Q_liq.
// TODO(phase 2, §5.1 Blockade apply, §4.1): implement
double blockade(const state::TerritoryState& t, const state::WorldState& s) noexcept;

// §4.1: migration_flux — used in dNpop equation.
// TODO(phase 2, §4.1, §17.2): linear migration-flux per v1; calibration pending v1.1.
double migration_flux(std::size_t territory_index, const state::WorldState& s) noexcept;

}  // namespace rc::sim::dynamics
