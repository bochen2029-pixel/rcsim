#pragma once

// §3.3: Global state — scalar quantities aggregated from per-territory grid.
// §3.3: D_global via tree-reduce (§2.4); A monotone non-decreasing (Diamond Floor);
// S_AI_index via tree-reduce with G-weights.
// §2.3: Seven PCG64 substreams derived from scenario_seed.

#include <cstdint>

#include "rcsim/core/rng.hpp"

namespace rc::sim::state {

// §3.3: ParamBlock — the free parameters of the ODE system, calibration targets.
// R-03 in docs/SPEC_AMBIGUOUS_RESOLUTIONS.md pins this enumeration. Defaults are
// first-pass; calibration in Phase 7a/7b refines.
struct ParamBlock {
    // §4.1 RC-1..RC-8 coefficients
    double delta       = 0.10;     // dr decay coefficient
    double kappa       = 1.0e-3;   // dr discovery coupling
    double lambda      = 0.05;     // dH growth coefficient
    double xi_d        = 0.5;      // dH displacement saturation
    double tau         = 5.0;      // dH displacement timescale (yr)
    double mu_f        = 1.0;      // dH friction multiplier
    double mu_m        = 0.02;     // dM monetary growth
    double g_N         = 0.005;    // dN demographic growth (per yr)
    double delta_s     = 0.05;     // dS legitimacy decay
    double kappa_elec  = 1.0;      // dQelec coefficient (EJ/yr)
    double kappa_liq   = 1.0;      // dQliq coefficient (EJ/yr)
    double delta_G     = 0.02;     // dG depreciation
    double E_build     = 50.0;     // GJ per HE-unit deployment

    // §4.2 global
    double rho_A       = 0.01;     // dA growth coefficient
    double kappa_f     = 0.5;      // dA feedback strength
    double G_floor     = 1.0e-3;   // S_AI weighting regularizer (NOT calibrated)

    // §4.3 conversion (R-10)
    double eta_l_to_e  = 0.35;
    double eta_e_to_l  = 0.60;

    // §4.5 sigmoid sharpness (constants from spec)
    double k_theta_soft = 50.0;
    double k_pi_hard    = 200.0;

    // §4.5 Hill function
    double S_half       = 1.0;
    double n_hill       = 2.0;
    double rho_max      = 1.0;

    // §6.4 attribution
    double k_detect     = 3.0;

    // §17.2 migration (linear v1)
    double migration_coupling = 1.0e-4;
};

// §3.3: GlobalState.
struct GlobalState {
    // §3.3, §4.2: aggregated scalars.
    double D_global;      // Σ D_local via tree-reduce
    double A;             // Capability ratio, monotone non-decreasing (Diamond Floor)
    double S_AI_index;    // Aggregated via tree-reduce with G-weights

    // §3.3: free parameters.
    ParamBlock params;

    // §2.3, §3.3: scenario seed + seven substreams.
    uint64_t seed_scenario;
    core::Pcg64 rng_shocks;
    core::Pcg64 rng_supply;
    core::Pcg64 rng_geopolitical;
    core::Pcg64 rng_demographic;
    core::Pcg64 rng_discovery;
    core::Pcg64 rng_market;
    core::Pcg64 rng_observer;
};

}  // namespace rc::sim::state
