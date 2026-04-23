#pragma once

// §3.1: Per-territory state — contiguous AoS (SoA removed in v1.3).
// §3.2: single contiguous std::vector<TerritoryState>.
// Rationale: at N=150, total ~= 19.2 KB fits in L1d; RK4 needs all fields per territory
// simultaneously; AoS gives single cache-line load per territory.

#include <cstdint>

namespace rc::sim::state {

// §3.1: TerritoryId and PrincipalId — identifier aliases.
// SPEC_AMBIGUOUS(§3.1): integer width of TerritoryId/PrincipalId is not explicitly stated.
// Using uint32_t to make TerritoryState total 128 bytes with explicit _pad[32]
// per CLAUDE.md §3 Task 4 instruction ("alignas(64) and explicit uint8_t _pad[32] = {};").
using TerritoryId = uint32_t;
using PrincipalId = uint32_t;

// §3.1: Per-territory state, AoS, cache-line aligned, explicit tail padding for
// deterministic byte-layout across compilers/ABIs.
struct alignas(64) TerritoryState {
    // §3.1: primary state fields, EJ/yr for energy per §2.2 unit shift.
    double r;         // EROEI, dimensionless, >= 1
    double H;         // Employed cognitive labor, persons
    double G;         // Deployed AI compute, human-equivalent units
    double M;         // Local financial claims, USD-eq
    double Npop;      // Population, persons
    double S;         // Local contribution to S_AI, dimensionless
    double Qelec;     // Conditioned-grid surplus exergy, EJ/yr
    double Qliq;      // Liquid-hydrocarbon surplus exergy, EJ/yr
    double D_local;   // Local cumulative discovery, accumulator

    // §3.1: derived (rebuilt pre-integration per §9 phase 4-5).
    double eta;
    double phi;

    // §3.1: identifiers.
    TerritoryId id;
    PrincipalId owner;

    // Explicit tail padding so total struct is exactly 128 B under common ABIs
    // and byte-layout is deterministic across compilers. Initialized to 0 to
    // ensure canonical hash reads identical bytes regardless of value-history.
    // SPEC_AMBIGUOUS(§3.1): explicit pad size assumes uint32_t TerritoryId/PrincipalId.
    //   If id types change, pad must be recomputed.
    uint8_t _pad[32] = {};
};

static_assert(sizeof(double) == 8,
    "§3.1 TerritoryState layout assumes 8-byte doubles (IEEE-754 binary64).");

}  // namespace rc::sim::state
