#include "rcsim/dynamics/ode_system.hpp"

#include <cmath>

#include "rcsim/dynamics/algebraics.hpp"
#include "rcsim/dynamics/coupling.hpp"
#include "rcsim/core/transcendentals.hpp"
#include "rcsim/state/global_state.hpp"

// §4.1: Per-territory ODE RHS (RC-1 through RC-8).
// §8.2: Gates held constant across RK4 substages — no mid-step reevaluation.
// §4.4: Gamma_kinetic from active shocks; Phase 1 returns 0 (no kinetic shocks
//       loaded yet — wired in Phase 2 when ShockImpulse PendingEffects land).
//
// All `convert`/`flux` terms come from WorldState::coupling. Phase 1 runs with
// zero coupling (single territory or N>1 with adjacency disabled), so the
// integer-network-flow contributions are zero — matches the §19 Phase 1 gate
// (simplified v5 mean-field retrodiction).

namespace rc::sim::dynamics {

GateValues evaluate_gates(const state::WorldState& s) noexcept {
    const std::size_t N = s.grid.states.size();
    GateValues g;
    g.theta_elec.resize(N);
    g.pi_crit.resize(N);
    g.theta_supply.resize(N);

    for (std::size_t i = 0; i < N; ++i) {
        const auto& t = s.grid.states[i];
        // §4.5 Theta_soft on Q_elec availability — sigmoid centered at small
        // threshold; territories with positive Q_elec pass cleanly.
        g.theta_elec[i] = Theta_soft(t.Qelec - 1.0e-3);
        // Pi_hard on Q_crit threshold — indicator-like at k=200.
        g.pi_crit[i]    = Pi_hard(t.Qelec + t.Qliq - 1.0e-2);
        // Supply gate — combines liquid and electrical availability.
        g.theta_supply[i] = Theta_soft(t.Qliq - 1.0e-3);
    }
    return g;
}

// §4.1 RC-1..RC-8.
ODESystem::Deriv ODESystem::derivative(
    const state::TerritoryState& t,
    const state::WorldState& s,
    const GateValues& gates,
    std::size_t i
) const noexcept {
    const auto& p = s.global.params;

    // RC-1: dr = -delta · r² + xi(D_global) · G · kappa + Gamma_kin
    double dr = -p.delta * t.r * t.r
              + xi_of_D(s.global.D_global) * t.G * p.kappa
              + gamma_kinetic(i, s.time_years, s);

    // RC-2: dH = lambda · H · (G/N) · exp(-xi_d · G/N)
    //          - (1/tau) · H · (1 - 1/A) · (G/N) / (1 + mu_f · phi)
    double GoverN = (t.Npop > 0.0) ? t.G / t.Npop : 0.0;
    double phi_i  = phi(t.r, t.H, t.G);
    double A_safe = (s.global.A > 0.0) ? s.global.A : 1.0;
    double dH = p.lambda * t.H * GoverN * core::exp_canonical(-p.xi_d * GoverN)
              - (1.0 / p.tau) * t.H * (1.0 - 1.0 / A_safe) * GoverN / (1.0 + p.mu_f * phi_i);

    // RC-3: dG = (rho(S_AI) / E_build) · Q_elec · Theta_soft · Pi_hard · Theta_supply
    //          - delta_G · G
    double rho_val = rho(s.global.S_AI_index, p.S_half, p.n_hill, p.rho_max);
    double dG = (rho_val / p.E_build) * t.Qelec
                * gates.theta_elec[i] * gates.pi_crit[i] * gates.theta_supply[i]
              - p.delta_G * t.G;

    // RC-4: dM = mu_m · M + (M / Q_total) · e_U(nu) · (N - H)
    double Q_total = t.Qelec + t.Qliq;
    double nu = (t.Npop > 0.0) ? (t.Npop - t.H) / t.Npop : 0.0;
    double dM = p.mu_m * t.M;
    if (Q_total > 0.0) {
        dM += (t.M / Q_total) * e_U(nu) * (t.Npop - t.H);
    }

    // RC-5: dN = g_N · N + migration_flux (Phase 2: linear migration model)
    double dNpop = p.g_N * t.Npop + migration_flux(i, s);

    // RC-6: dS = (g_e - delta_s) · S; g_e = f(R_AI, H) — Phase 1 placeholder
    //   First-pass: g_e = lambda · eta(territory) saturating at AI labor share.
    double eta_i = eta(t.r, t.H, t.G);
    double g_e = p.lambda * eta_i;
    double dS = (g_e - p.delta_s) * t.S;

    // RC-7: dQelec = kappa_elec · r - c_elec(H, G, N) · gate + flux_elec - convert_liq_to_elec
    //   Coefficients are physical first-pass (calibration target Phase 7b):
    //     - 7e-8 EJ/yr per person ≈ 70 GJ/yr/person (OECD average per-capita energy)
    //     - 1e-9 EJ/yr per HE-unit ≈ AI compute share, far smaller than gross.
    //   Gating by tick-boundary theta_elec (§4.5 / §8.2 — gate held constant
    //   across RK4 substages) keeps Q in the basin without mid-tick discontinuity.
    double c_elec_raw = 7.0e-8 * t.Npop + 1.0e-9 * t.G;
    double dQelec = p.kappa_elec * t.r - c_elec_raw * gates.theta_elec[i];

    // RC-8: dQliq = kappa_liq · r - c_liq · gate + flux_liq - convert_elec_to_liq - blockade
    //   Liquid hydrocarbon is roughly 0.6× per-capita electric in industrial economies.
    double c_liq_raw = 4.2e-8 * t.Npop;
    double dQliq = p.kappa_liq * t.r - c_liq_raw * gates.theta_supply[i] - blockade(t, s);

    // dD_local = G · kappa
    double dD_local = t.G * p.kappa;

    return Deriv{dr, dH, dG, dM, dNpop, dS, dQelec, dQliq, dD_local};
}

void ODESystem::rhs(
    const state::WorldState& s,
    const GateValues& gates,
    std::vector<Deriv>& out_deriv
) const noexcept {
    const std::size_t N = s.grid.states.size();
    out_deriv.resize(N);
    // §2.4: per-territory parallel safe (no reductions inside this function).
    for (std::size_t i = 0; i < N; ++i) {
        out_deriv[i] = derivative(s.grid.states[i], s, gates, i);
    }
}

double ODESystem::gamma_kinetic(
    std::size_t /*territory_index*/,
    double /*time_years*/,
    const state::WorldState& /*s*/
) const noexcept {
    // §4.4: Σ A_k · 1[in-window] · exp(-λ_k (t - t_start))
    // Phase 1: no kinetic shocks loaded — wired in Phase 2 with ShockImpulse
    // PendingEffects walking WorldState::pending.
    return 0.0;
}

}  // namespace rc::sim::dynamics
