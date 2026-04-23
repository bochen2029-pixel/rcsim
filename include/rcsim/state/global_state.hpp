#pragma once

// §3.3: Global state — scalar quantities aggregated from per-territory grid.
// §3.3: D_global via tree-reduce (§2.4); A monotone non-decreasing (Diamond Floor);
// S_AI_index via tree-reduce with G-weights.
// §2.3: Seven PCG64 substreams derived from scenario_seed.

#include <cstdint>

#include "rcsim/core/rng.hpp"

namespace rc::sim::state {

// §3.3: ParamBlock — the free parameters of the ODE system, calibration targets.
// TODO(phase 1): populate with the parameters named in §4 (δ, κ, λ, τ, μ_f, ρ_max,
// S_half, g_N, κ_elec, κ_liq, η_l→e, η_e→l, k_detect, k=50, k=200, etc.) as calibration progresses.
// SPEC_AMBIGUOUS(§3.3): full enumeration of ParamBlock fields deferred to phase 1; spec
// names parameters in prose across §4 but does not list them as a single structured block.
struct ParamBlock {
    // Placeholder — fields added as phase 1 enumerates them from §4.
    // Kept default-initialized so GlobalState constructs without argument.
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
