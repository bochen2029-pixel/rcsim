#pragma once

// §4.1: Per-territory ODE equations (RC-1 through RC-8).
// §4.2: Global equations (D_global, A, S_AI).
// §8.2: Gates evaluated once per tick at tick boundary, held constant across RK4 substeps.
// §9.5, §9.6: evaluate_gates in phase 5; RK4 integration in phase 6.

#include "rcsim/state/world_state.hpp"

namespace rc::sim::dynamics {

// §8.2, §9.5: Gate values computed once per tick from tick-boundary state.
// Passed as closed-over constant into RK4 rhs across all substages.
struct GateValues {
    // Per-territory, parallel to grid.states indexing.
    std::vector<double> theta_elec;     // §4.5 Θ_soft on Q_elec availability
    std::vector<double> pi_crit;        // §4.5 Π_hard on Q_crit threshold
    std::vector<double> theta_supply;   // §4.5 Θ on supply availability
};

// §8.2: evaluate_gates — phase 5 of Advance loop.
// TODO(phase 1, §8.2, §9.5): implement per DESIGN_v1.3.md
GateValues evaluate_gates(const state::WorldState& s) noexcept;

// §4.1: ODESystem — computes per-territory derivatives with gates held constant.
class ODESystem {
public:
    // §4.1: rhs — compute \dot r_i, \dot H_i, \dot G_i, \dot M_i, \dot Npop_i,
    //        \dot S_i, \dot Qelec_i, \dot Qliq_i, \dot D_i_local for territory i.
    //        Gates read from GateValues (closed over by integrator). See §4.1 RC-1 .. RC-8.
    // TODO(phase 1, §4.1): implement per DESIGN_v1.3.md §4.1 equations.
    struct Deriv {
        double dr, dH, dG, dM, dNpop, dS, dQelec, dQliq, dD_local;
    };

    // §4.1: per-territory derivative. `s` is a snapshot; `gates` closed-over constant.
    Deriv derivative(
        const state::TerritoryState& t,
        const state::WorldState& s,
        const GateValues& gates,
        std::size_t territory_index
    ) const noexcept;

    // §4.1: full rhs — fills derivative grid for all territories (§2.4 parallelizable
    // per-territory; NO reductions inside this function — reductions go through tree_reduce).
    // TODO(phase 1, §4.1, §2.4): implement; allow per-territory parallelism.
    void rhs(
        const state::WorldState& s,
        const GateValues& gates,
        std::vector<Deriv>& out_deriv
    ) const noexcept;

    // §4.4: Γ_kinetic(t) — sum of active kinetic shocks per territory.
    //   Γ_kin,i(t) = Σ_k A_k · 1[t_start ≤ t ≤ t_end] · exp(-λ_k (t - t_start))
    // TODO(phase 1, §4.4): implement per DESIGN_v1.3.md
    double gamma_kinetic(std::size_t territory_index, double time_years,
                         const state::WorldState& s) const noexcept;
};

}  // namespace rc::sim::dynamics
