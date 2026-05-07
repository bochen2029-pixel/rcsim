# SPEC_AMBIGUOUS Resolutions — v1.3 first pass

**Status:** v1 first-pass resolutions for the Phase 0–2 commitment (`docs/PHASE_0_2_PLAN.md`).
**Authority:** these resolutions pin scaffold ambiguities so Phase 1+ can compile and run. Mathematical resolutions (R-04..R-08) are calibration targets — refined in Phase 7a/7b. Administrative resolutions (R-01..R-03, R-09..R-19) are intended to be permanent at schema_major.

**How to read this:** each entry has `Marker | §section | Location | Resolution | Notes`. Cite the marker (e.g. "R-08") in commit messages and code comments that consume the resolution.

---

## Phase 1 blockers (must land before Phase 1 starts)

### R-01 — §3.1 — TerritoryId / PrincipalId integer width
**Location:** `include/rcsim/state/territory.hpp:13`
**Resolution:** `using TerritoryId = uint32_t; using PrincipalId = uint32_t;`
**Notes:** Supports up to ~4.29×10⁹ entities — far above any plausible scenario (default N=150 territories, P≤32 principals). Matches scaffold's existing assumption. Frozen at schema_major.

### R-02 — §3.1 — `_pad[32]` size
**Location:** `include/rcsim/state/territory.hpp:44`
**Resolution:** Keep `uint8_t _pad[32] = {};` — total `sizeof(TerritoryState) == 128` bytes (9 doubles = 72 + 2 uint32 = 8 + 32 padding = 112 → padded to 128 by `alignas(64)`). Add `static_assert(sizeof(TerritoryState) == 128)` at file bottom.
**Notes:** Depends on R-01. If TerritoryId/PrincipalId widths change, recompute pad.

### R-03 — §3.3 — `ParamBlock` field enumeration
**Location:** `include/rcsim/state/global_state.hpp:17`
**Resolution:** Flat struct with all parameters named in §4 prose:
```cpp
struct ParamBlock {
    // §4.1 RC-1..RC-8 coefficients
    double delta       = 0.10;      // dr decay coefficient
    double kappa       = 1e-3;      // dr discovery coupling
    double lambda      = 0.05;      // dH growth coefficient
    double xi_d        = 0.5;       // dH displacement saturation
    double tau         = 5.0;       // dH displacement timescale (yr)
    double mu_f        = 1.0;       // dH friction multiplier
    double mu_m        = 0.02;      // dM monetary growth
    double g_N         = 0.005;     // dN demographic growth (per yr)
    double delta_s     = 0.05;      // dS legitimacy decay
    double kappa_elec  = 1.0;       // dQelec coefficient (EJ/yr)
    double kappa_liq   = 1.0;       // dQliq coefficient (EJ/yr)
    double delta_G     = 0.02;      // dG depreciation
    double E_build     = 50.0;      // GJ per HE-unit deployment

    // §4.2 global
    double rho_A       = 0.01;      // dA growth coefficient
    double kappa_f     = 0.5;       // dA feedback strength
    double G_floor     = 1e-3;      // S_AI weighting regularizer (NOT calibrated)

    // §4.3 conversion (R-10)
    double eta_l_to_e  = 0.35;
    double eta_e_to_l  = 0.60;

    // §4.5 sigmoid sharpness (constants from spec)
    double k_theta_soft = 50.0;
    double k_pi_hard    = 200.0;

    // §4.5 Hill function (R-04..R-07 use these defaults)
    double S_half       = 1.0;
    double n_hill       = 2.0;
    double rho_max      = 1.0;

    // §6.4 attribution
    double k_detect     = 3.0;

    // §17.2 migration (linear v1)
    double migration_coupling = 1e-4;
};
```
**Notes:** All defaults are first-pass; calibration targets in Phase 7a/7b. `G_floor` is regularization per §4.2, never calibrated. Frozen field set at schema_major; values may move within schema_minor.

### R-04 — §4.1 — `e_U(ν)` functional form
**Location:** `include/rcsim/dynamics/algebraics.hpp:46`, `src/dynamics/algebraics.cpp:23`
**Resolution:**
```cpp
// e_U(nu) — residual employability multiplier in dM equation (RC-4).
// nu is unemployment fraction (N - H) / N; e_U is 1 at full employment, decays smoothly.
double e_U(double nu) noexcept {
    double clamped = nu < 0.0 ? 0.0 : (nu > 1.0 ? 1.0 : nu);
    double one_minus = 1.0 - clamped;
    return one_minus * one_minus;     // (1 - nu)^2
}
```
**Notes:** First-pass. Quadratic decay matches v5's "convex disutility of unemployment." Calibration target in Phase 7b. Definition implies `e_U(0) = 1.0` (full employment) and `e_U(1) = 0.0` (full unemployment).

### R-05 — §4.1 — `ξ(D_global)` functional form
**Location:** `include/rcsim/dynamics/algebraics.hpp:51`, `src/dynamics/algebraics.cpp:28`
**Resolution:**
```cpp
// xi_of_D — discovery efficacy with diminishing returns.
inline constexpr double kXi_zero  = 1.0;
inline constexpr double kXi_Dhalf = 1.0e10;   // saturation point of cumulative discovery

double xi_of_D(double D_global) noexcept {
    return kXi_zero / (1.0 + D_global / kXi_Dhalf);
}
```
**Notes:** Diminishing-returns form per v5 prose ("discovery saturates"). Calibration target.

### R-06 — §4.2 — `η` derived quantity per territory
**Location:** `include/rcsim/dynamics/algebraics.hpp:35`, `src/dynamics/algebraics.cpp:13`
**Resolution:**
```cpp
// eta — fractional AI labor share for territory i.
// Range [0, 1]. Used in S_AI weighting and e_U arguments.
double eta(double r, double H, double G) noexcept {
    (void)r;
    constexpr double kG_floor = 1.0e-3;            // matches ParamBlock.G_floor
    return G / (G + H + kG_floor);
}
```
**Notes:** This per-territory `η` is distinct from `η_l→e` and `η_e→l` (R-10) — those are conversion-matrix entries. First-pass; revisit if calibration shows H+G is wrong denominator.

### R-07 — §4.2 — `φ` derived quantity per territory
**Location:** `include/rcsim/dynamics/algebraics.hpp:41`, `src/dynamics/algebraics.cpp:18`
**Resolution:**
```cpp
// phi — AI deployment per capita.
// Used in dH equation as fragility multiplier (1 + mu_f * phi).
double phi(double r, double H, double G) noexcept {
    (void)r; (void)H;
    constexpr double kN_floor = 1.0;     // unit person to avoid div-by-zero
    // Note: signature takes H, G, r — Npop must be passed via caller closing over it,
    // OR change signature to accept (G, Npop). For first pass, treat H as proxy population.
    return G / (H + kN_floor);
}
```
**Notes:** Header-declared signature takes `(r, H, G)`. **Caller-side adaptation:** `algebraics::phi` is called from `ode_system.cpp` with full `TerritoryState` available, so pre-integration rebuild can pass `(t.r, t.Npop, t.G)` via a wrapper. Adjust signature if confusing — additive change within schema_minor.

### R-08 — §4.5 — Padé [6/6] coefficients for `polynomial_exp_bitexact`
**Location:** `include/rcsim/core/transcendentals_polynomial.hpp:12`, `src/core/transcendentals_polynomial.cpp:5`
**Resolution:** Standard Padé [6/6] approximant for `exp(r)` with even/odd split:
```
even part: p_e(r) = 1 + (5/44) r²  + (1/792) r⁴  + (1/665280) r⁶
odd part:  p_o(r) =     (1/2)   r  + (1/66)   r³ + (1/15840)  r⁵
exp(r) ≈ (p_e + p_o) / (p_e - p_o)
```
Range reduction: `x = k * ln(2) + r`, `|r| ≤ ln(2)/2 ≈ 0.347`, where `k = std::nearbyint(x * (1/ln(2)))`. Final result: `exp(x) = std::ldexp(exp(r), k)`. Use full-double `kLn2 = 0x3FE62E42FEFA39EFp-0` — single multiplication acceptable for `|x| ≤ 50` since output is ULP-bounded under strict IEEE-754. Evaluate even part as `((c6*r² + c4)*r² + c2)*r² + 1` (Horner on r²); odd part as `r * ((c5*r² + c3)*r² + c1)`.
**Notes:** Bit-exactness across compilers requires: (a) `/fp:strict` and `-ffp-contract=off` (already set in `CMakeLists.txt`); (b) Horner evaluation order pinned in code. Ranged to `[-50, 50]` per spec; out-of-range falls through to pinned `Sleef_exp_u10`. CI test `07_pinned_sleef_variant.cpp` verifies only scalar Sleef is referenced.

---

## Phase 2 blockers (must land before Phase 2 starts)

### R-09 — §3.5 — `AdjacencyGraph` by-value vs pointer
**Location:** `include/rcsim/state/world_state.hpp:66`
**Resolution:** **By value.** Replace `geo::AdjacencyGraph* adjacency;` with `geo::AdjacencyGraph adjacency;` and `#include "rcsim/geo/adjacency.hpp"`. The include cycle the scaffold worried about does not exist — `geo/adjacency.hpp` only depends on `geo/geojson.hpp` and `state/territory.hpp`, neither of which include `world_state.hpp`.

### R-10 — §4.3 — η_l→e, η_e→l pinned values
**Location:** `include/rcsim/dynamics/coupling.hpp:13`
**Resolution:** `kEta_liq_to_elec = 0.35`, `kEta_elec_to_liq = 0.60` (already in scaffold). Promoted from "default" to "pinned v1 value." Calibration in Phase 7b refines.

### R-11 — §4.6 — `CouplingGraph` structure
**Location:** `include/rcsim/state/world_state.hpp:31`
**Resolution:**
```cpp
struct CouplingGraph {
    // Edges sorted lex by (src_id, dst_id).
    std::vector<core::FlowEdge> edges;
    // Per-edge solved flow, parallel to edges, Q-format integer.
    std::vector<int64_t> flows;
    // Per-territory net flow aggregates (consumed as constants by ODE rhs).
    // Indexed by territory_id; Q-format matches the field's storage format.
    std::vector<int64_t> net_flow_Q_elec;   // Q32.32 EJ/yr
    std::vector<int64_t> net_flow_Q_liq;    // Q32.32 EJ/yr
    std::vector<int64_t> net_flow_M;        // Q56.8 USD-eq
    std::vector<int64_t> net_migration;     // Q48.16 persons/yr
};
```

### R-12 — §5.2 — `ActionLogId` width
**Location:** `include/rcsim/state/pending.hpp:17`
**Resolution:** `using ActionLogId = uint64_t;` (matches scaffold). 64-bit gives generational safety even at 10⁹-action sweeps.

### R-13 — §5.4 — `ValidationReason` enum
**Location:** `include/rcsim/action/validate_epistemic.hpp:24`
**Resolution:** Replace `std::string reason` with scoped enum:
```cpp
enum class ValidationReason : uint32_t {
    Ok                     = 0,
    InsufficientTreasury   = 1,
    InsufficientLegitimacy = 2,
    NotOwnedByActor        = 3,
    PoliticalClockGated    = 4,
    DistortionLockout      = 5,
    UnknownTarget          = 6,
    InvalidParameters      = 7,
    DurationOutOfRange     = 8,
    PrincipalLockedOut     = 9
    // Frozen at schema_major; additive within schema_minor; unknown → halt per §10.1a.
};
struct ValidationResult {
    bool ok;
    ValidationReason reason;
};
```

### R-14 — §5.4 — `FizzleReason` enum
**Location:** `include/rcsim/action/apply_with_fizzle.hpp:22`
**Resolution:**
```cpp
enum class FizzleReason : uint32_t {
    None                    = 0,
    TargetAlreadyDestroyed  = 1,
    TargetOwnerChanged      = 2,
    SupplyExhausted         = 3,
    GeographyImpossible     = 4,
    GateClosedAtApply       = 5,
    ConflictDegraded        = 6
    // Frozen at schema_major; additive within schema_minor.
};
struct ApplyResult {
    state::WorldState state;
    bool applied = false;
    bool fizzled = false;
    FizzleReason reason = FizzleReason::None;
};
```
**Notes:** Distinct from `ValidationReason` — fizzle is post-validation ground-truth failure.

### R-15 — §6.2 — `ObservationField` enum closure
**Location:** `include/rcsim/observer/measurement.hpp:19`
**Resolution:** Close at the 7 values already in scaffold:
```cpp
enum class ObservationField : uint32_t {
    CPI            = 0,
    RealGDP        = 1,
    Unemployment   = 2,
    S_AI_index     = 3,
    MWhConsumed    = 4,    // physical, undistortable
    RailTonMiles   = 5,    // physical, undistortable
    CrudeExtracted = 6     // physical, undistortable
    // Frozen at schema_major; additive within schema_minor.
};
```

### R-16 — §10.1 — `ActionLogEntry` field layout
**Location:** `include/rcsim/state/world_state.hpp:44`
**Resolution:**
```cpp
enum class ApplyOutcome : uint32_t {
    Applied           = 0,
    Fizzled           = 1,
    RejectedEpistemic = 2
};
struct ActionLogEntry {
    ActionLogId           id;
    core::Tick            applied_at;
    PrincipalId           principal;
    uint32_t              action_seq;
    int32_t               priority;
    std::vector<uint8_t>  serialized_action;   // canonical wire format per §10.1a
    ApplyOutcome          outcome;
    uint32_t              reason;              // ValidationReason or FizzleReason value
};
```
**Notes:** `serialized_action` carries the action payload in canonical wire format (§10.1a) so the log is replayable without recursing into the variant. `reason` field is the underlying `uint32_t` of either reason enum, distinguished by `outcome`.

### R-17 — §12 — GeoJSON schema
**Location:** `include/rcsim/geo/geojson.hpp:15`
**Resolution:** GeoJSON `FeatureCollection` per RFC 7946; each `Feature` is a Polygon or MultiPolygon with required `properties.territory_id` (uint32, unique). Coordinates as `[longitude, latitude]` decimal degrees, snapped to fixed-point grid per R-19.
```json
{
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "geometry": {"type": "Polygon", "coordinates": [[[lon,lat], ...]]},
      "properties": {"territory_id": 0, "name": "optional"}
    }
  ]
}
```

### R-18 — §18 — `partition.cpp`
**Location:** `include/rcsim/geo/partition.hpp:4`
**Resolution:** **Remove.** Delete `include/rcsim/geo/partition.hpp` and `src/geo/partition.cpp`; remove from `CMakeLists.txt` line 147. Spatial structure is fully captured by `adjacency.cpp` + `geojson.cpp`. The §18 directory listing has it as an artifact from v1.2; v1.3's spec text never references partition functionality.

### R-19 — §18 — coordinate snap grid
**Location:** `include/rcsim/geo/adjacency.hpp:32`
**Resolution:** `1e-7` decimal degrees (~1.1 cm at equator). Snap on GeoJSON load: `snapped = std::round(coord * 1e7) / 1e7`. Sufficient precision for political boundaries; deterministic across IEEE-754 implementations because integer round-trip is exact.
**Notes:** Dateline crossings (lon ≈ ±180°) are not addressed by simple snapping; defer to Phase 2 implementation review.

---

## Phase 3+ resolutions (informational; not Phase-0–2 blockers)

### R-20 — §6.1 — `TerritoryObservation` field list
**Location:** `include/rcsim/observer/measurement.hpp:41`
**Resolution:** Include all 9 primary state fields with `_reported` suffix; per-field visibility controlled by `TelemetryAccess` (R-22):
```cpp
struct TerritoryObservation {
    state::TerritoryId id;
    double r_reported, H_reported, G_reported, M_reported;
    double Npop_reported, S_reported;
    double Qelec_reported, Qliq_reported, D_reported;
};
```

### R-21 — §6.3 — `AsymmetricTelemetry` naming (type vs policy)
**Location:** `include/rcsim/observer/operator_policies.hpp:53`
**Resolution:** `AsymmetricTelemetry` is the **policy class** name (concrete `ObservationOperator`); the per-principal access blob is `TelemetryAccess` (R-22). Rename any uses that conflate the two.

### R-22 — §6.3 — `TelemetryAccess` specifics
**Location:** `include/rcsim/observer/asymmetric_telemetry.hpp:17`
**Resolution:**
```cpp
struct TelemetryAccess {
    std::array<bool, 7> field_visible;   // indexed by ObservationField (R-15)
    double noise_sigma;                   // additive Gaussian noise std-dev per draw (rng_observer)
    std::vector<state::TerritoryId> territories_visible;
};
```

### R-23 — §6.4 — `evaluate_attribution` signature
**Location:** `include/rcsim/observer/attribution.hpp:51`
**Resolution:** Add `current_tick` and return result struct:
```cpp
struct AttributionResult { bool detected; double p_detect; };
AttributionResult evaluate_attribution(
    const action::AttributeDistortion& attr,
    const observer::DistortionInvestment& target,
    const state::PrincipalState& attributer,
    core::Tick current_tick,
    core::Pcg64& rng_observer
) noexcept;
```

### R-24 / R-25 — §7.3 — regime defaults (v1.2 inheritance)
**Location:** `include/rcsim/political/response_latency.hpp:12`, `src/political/response_latency.cpp:8`
**Resolution:** Inherit from v1.2:
| Regime | `tau_modal` (yr) | `tau_variance` placeholder |
|---|---|---|
| Democracy | 1.5 | (0.25 · tau_modal)² |
| Autocracy | 2.0 | (0.25 · tau_modal)² |
| TheocraticAutocracy | 2.5 | (0.25 · tau_modal)² |
| Petrostate | 1.0 | (0.25 · tau_modal)² |
| FailedState | 0.5 | (0.50 · tau_modal)² |
| TransitionalState | 1.0 | (0.50 · tau_modal)² |

Variance is a placeholder until Phase 7b gamma-fits land per §16.4 / CALIBRATION.md.

### R-26 — §10.1 — checkpoint cadence K
**Location:** `include/rcsim/io/sync_log.hpp:48`
**Resolution:** `K = 100` ticks (~2 simulated years at 1 week/tick). Configurable per scenario via `sync_log.checkpoint_cadence` YAML key.

### R-27 / R-28 — §11.1 — sweep YAML schema, prior distribution family
**Location:** `include/rcsim/testbed/harness.hpp:25`, `include/rcsim/testbed/sweep.hpp:14`, `include/rcsim/testbed/sweep.hpp:30`
**Resolution:** YAML schema:
```yaml
sweep:
  seed_root: <uint64>
  n_samples: <uint32>
  parameters:
    - name: <string>     # ParamBlock field name (R-03)
      prior:
        family: normal | lognormal | beta
        params: {mu: <double>, sigma: <double>}    # or {alpha, beta} for beta
```
Prior-family enum:
```cpp
enum class PriorFamily : uint32_t { Normal = 0, Lognormal = 1, Beta = 2 };
```

### R-29 — §14 — Windows IPC for multi-subscriber
**Location:** `include/rcsim/io/ipc_multisubscriber.hpp:44`
**Resolution:** AF_UNIX socket on Windows 10 1803+ (mainstream now); fallback to named pipe (`\\.\pipe\rcsim-render-N`) for older. Selection at compile time via `#ifdef _WIN32` plus runtime feature detection. Defer concrete implementation to Phase 9.

---

## Cross-reference back to PHASE_0_2_PLAN.md

| Plan section | Resolutions consumed |
|---|---|
| Phase 0 gate exit | R-01..R-19 must be present |
| Phase 1 critical path | R-01, R-02, R-03, R-04, R-05, R-06, R-07, R-08 |
| Phase 2 — Integer Network Flow + coupling | R-09, R-10, R-11 |
| Phase 2 — actions + wire | R-12, R-13, R-14, R-15, R-16 |
| Phase 2 — geo + scenario | R-17, R-18, R-19 |
| Phase 3+ | R-20..R-29 (not blocking) |
