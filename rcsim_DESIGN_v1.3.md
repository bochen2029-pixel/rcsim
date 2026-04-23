# rcsim — Design Specification v1.3

*Deterrence-Extended with Civilizational Economics: a deterministic, headless, bit-reproducible simulator of coupled thermodynamic, cognitive, financial, kinetic, and reflexive-observer dynamics at nation-state resolution.*

*Author of the spec: Claude Opus 4.7, synthesizing four LLM generations + second-opinion correction + k=2 adversarial pressure-test + k=4 God-Mode adversarial pass + AEGIS technical review, for Bo Chen, April 2026.*
*Implementation author: Bo Chen.*
*Ancestral artifacts: Introversion DEFCON (2006), Bo Chen's Defcon Deterrence mod (2012), Bo Chen's DF-41 Simulator (Steam, 2020), Chen, The Race Condition v5 (April 2026).*

*Revision history:*
- *v1.0: initial synthesis of four LLM generations*
- *v1.2: k=2 adversarial pass caught `std::execution::par_unseq` non-associativity + libm transcendental non-bit-exactness + 15 other amendments*
- *v1.3 (this document): k=4 God-Mode adversarial pass caught 8 critical bugs in v1.2 (Q-format overflow, LP-solver determinism delusion, quantization viscosity / First Law violation, integrator-bisection contradiction, signed-zero hash bomb, epistemic oracle leak, AoS/SoA contradiction, bang-bang exploit); AEGIS technical review added 8 implementation-grade refinements. v1.3 is commit-ready.*

---

## §0. Thesis and framing

**rcsim is the executable extension of Defcon Deterrence (2012) with v5's coupled civilizational dynamics.** The kinetic substrate — MIRV splitting, kinetic anti-ship, satellite EMP, doomsday device, dead-hand — was built fourteen years before the framework that explains what it was modeling. v5 §8.2's Γ_kinetic term is the retrospective name for what the 2012 mod already instantiated. rcsim is not a civilizational economics sim that has kinetic features added; it is a shipped kinetic simulator acquiring its economic substrate. Framing order is load-bearing: the paper that emerges from calibrating rcsim is titled *"Deterrence, Extended: A Spatial Multi-Agent Calibration of Civilizational Dynamics Against a Pre-Existing Kinetic Substrate,"* and its methodological honesty is that the kinetic layer pre-validates v5 §8.2 structurally because it was built without knowledge of the framework.

**Thesis in one sentence.** Given v5's seven-variable coupled ODE system, partition state geographically across N territories (default 150), couple them via trade/migration/kinetic/observation flows (as Integer Network Flow on quantized Q-format integers), let P principals (nation-states) act through a typed Action variant with political-clock-gated response functions and reflexive-observer distortion-with-probabilistic-attribution, apply actions at deterministic tick boundaries with epistemic-validation-plus-ground-truth-fizzle, integrate ODEs with fixed-step RK4 (gates held constant across substages; no bisection), canonicalize state at tick boundaries to fixed-point with Kahan residual error-diffusion, hash with Blake3-256 using hand-rolled deterministic tree-reduction, write Parquet for research and binary sync log for replay. Everything else follows.

**One invariant drives all design decisions:**

> Given a scenario seed and an action log, the full state at tick T is bit-identical across machines, compilers (in IEEE-754 strict mode), library versions within a schema major, thread counts, SIMD widths, and OS page-fault patterns. No exceptions.

Achieving this invariant requires: (a) no floating-point LP solver for coupling flows (§4.6 Integer Network Flow); (b) no transcendentals via libm (§4.5 pinned sleef variant); (c) no parallel reductions via `std::execution` (§2.4 hand-rolled tree-reduce); (d) no dynamic step-size adjustment in integrator (§8 no bisection); (e) no raw `bit_cast` on IEEE-754 values without signed-zero canonicalization (§2.6); (f) no unit-choice that overflows the fixed-point format (§2.2 energy in EJ/yr, not J/yr); (g) no truncation-at-tick-boundary without Kahan residual buffer (§2.7). Every naive implementation breaks the contract in a different way.

---

## §1. Scope, tiers, non-goals

### §1.1 Three tiers, one core

| Tier | Target user | Interface | Session length |
|---|---|---|---|
| **Research** | Academic / policy analyst | Python binding + Jupyter + Parquet | Minutes to hours |
| **Strategic** | Simulation-game player | Native HUD + globe render, full v5 dynamics visible | 3-10 hours per campaign |
| **Casual** | General audience | Simplified UI, forensic loss readout | 30-60 minutes per run |

All three tiers run the identical simulation core. Differences are in (a) which state variables have UI representations, (b) logging depth, (c) tick rate relative to wall clock, (d) action surface exposed, (e) exit UI. Casual tier does not lie about physics; it frames loss forensically (§13).

### §1.2 Non-goals

No graphics in the core library. **No networking beyond replay file I/O and local Unix-domain-socket IPC to rendering processes (machine-local only, no TCP/IP).** No real-time constraints. No mod system. No GUI. Rendering is a separate repository (`rcsim-render`). Core compiles without renderer by default.

### §1.3 What rcsim is not

Not a policy advisor. Not a forecasting instrument. Not a general civilizational simulator — v5's specific thesis rendered executable.

---

## §2. Determinism contract

### §2.1 No floating-point state accumulation across tick boundaries without canonicalization + Kahan residual

Inside a tick, RK4 uses `double` with strict IEEE-754 (`/fp:strict` on MSVC; `-ffp-contract=off -fno-fast-math -fno-associative-math` on GCC/Clang). At tick boundary, state quantizes to fixed-point (§2.2). The canonicalized form is what the next tick reads and what the hash consumes.

**Naive truncation destroys slow-moving derivatives** (First Law violation — thermodynamic/financial leakage when δstate < LSB per tick). Fix: Kahan-style residual buffer (§2.7).

### §2.2 Per-field fixed-point scaling with EJ/yr units for energy

**Critical correction from v1.2.** Q32.32 has 32 integer bits, max value 2³² − 1 ≈ 4.29×10⁹. v1.0/v1.2 specified 10²⁰ J/yr as the energy ceiling, which overflows Q32.32 by factor 2.3×10¹⁰ on Tick 1. Fix: shift unit of physics to **Exajoules per year (EJ/yr)**, where 1 EJ = 10¹⁸ J. Global primary energy ~580 EJ/yr; per-territory ceiling ~1000 EJ/yr. Storage in Q32.32 uses ~10 integer bits, leaves ~22 bits headroom.

| Field | Format | Unit | Range | Relative precision |
|---|---|---|---|---|
| `r` (EROEI) | Q32.32 | dimensionless | [1.0, ~10⁹] | ~2e-10 |
| `H` | Q48.16 | persons | [0, ~10¹²] | ~1.5e-5 |
| `G` | Q48.16 | HE-units | [0, ~10¹²] | ~1.5e-5 |
| `M` | Q56.8 | USD-eq | [0, ~10¹⁷] | 1/256 |
| `Npop` | Q48.16 | persons | [0, ~10¹²] | ~1.5e-5 |
| `S` | Q32.32 | dimensionless | [0, ~10⁹] | ~2e-10 |
| `Qelec` | Q32.32 | **EJ/yr** | **[0, ~1000]** | **~0.23 GJ/yr** |
| `Qliq` | Q32.32 | **EJ/yr** | **[0, ~1000]** | **~0.23 GJ/yr** |
| `D` | Q48.16 | accumulator | [0, ~10¹²] | ~1.5e-5 |
| `A` | Q32.32 | dimensionless | [1.0, ~10⁹] | ~2e-10 |

Conversion factors in §4.3 rescaled to EJ. Saturation at 95% max emits warning; at 100%, sim halts and logs scenario misspecification.

### §2.3 One PCG64 RNG substream per concern, namespace-separated

Seven substreams: `rng_shocks`, `rng_supply`, `rng_geopolitical`, `rng_demographic`, `rng_discovery`, `rng_market`, `rng_observer`. Derived from `(scenario_seed, substream_id)` via Blake3-256 truncation. Per-territory RNG via Philox-4x32-10 counter-based call keyed on `(substream_state, tick, territory_id, purpose_tag)`.

### §2.4 Deterministic ordering + hand-rolled tree-reduce (no `std::execution::par_unseq`)

No iteration over `std::unordered_map`. All action queues sort by `(tick, principal_id, action_seq)`. All flow networks sort edges by `(src_id, dst_id)`. All Parquet row batches flush in `run_id` order.

**`std::execution::par_unseq` is forbidden for reductions.** Implementation-dependent evaluation order + FP non-associativity → non-reproducible. Required primitive: `deterministic_tree_reduce` in `core/hash.hpp` with fixed pair-ordering by territory_id:

```cpp
template<typename T, typename Op>
T deterministic_tree_reduce(std::span<const T> values, Op binary_op) {
    if (values.empty()) return T{};
    std::vector<T> buffer(values.begin(), values.end());
    size_t padded = std::bit_ceil(buffer.size());
    buffer.resize(padded, identity_for<Op>());
    while (padded > 1) {
        for (size_t i = 0; i < padded / 2; ++i) {
            buffer[i] = binary_op(buffer[2*i], buffer[2*i + 1]);
        }
        padded /= 2;
    }
    return buffer[0];
}
```

Parallel execution over territories allowed for per-territory ODE derivatives (no reduction). Reductions (D_local → D_global; S_AI weighted sum) go through `deterministic_tree_reduce`. **CI gate: `parallel_hash == sequential_hash` across 1/4/16/64 threads.**

### §2.5 No wall-clock time in core

Ticks are `uint64_t`. No `std::chrono::system_clock`. No timestamps in canonical state.

### §2.6 Signed-zero canonicalization + NaN/Inf trap (new — v1.3)

IEEE-754 defines both `+0.0` and `-0.0`. Mathematically equal; bit-cast differently. Thread-sequencing differences can produce either signed-zero legitimately. Hash via raw `bit_cast` will diverge.

**Fix:** immediately before hashing, apply canonicalization sanity sweep to every field:

```cpp
inline double canonicalize_for_hash(double v) {
    if (v == 0.0) v = 0.0;                     // strips sign bit on signed zeros
    if (!std::isfinite(v)) trap_nonfinite(v);  // hard halt with diagnostic
    return v;
}
```

NaN and Inf are never valid canonical states. If they appear, scenario is broken or integrator diverged; halt immediately with field-identification for forensic analysis.

### §2.7 Kahan-style quantization residual buffer (new — v1.3)

Quantization at tick boundary discards the sub-LSB portion of the `double` value. For slow-moving derivatives (δvalue < LSB per tick), the variable never moves. Over a 30-year sim (1560 ticks), cumulative loss can be arbitrarily large — First Law violation.

**Fix:** error-diffusion residual, stored in non-hashed sidecar:

```cpp
struct CanonicalizationState {
    // Per-field quantized value (hashed, canonical)
    TerritoryGrid grid;
    // Per-field residual (NOT hashed, accumulates precision)
    std::vector<double> residual_r, residual_H, residual_G, /* ... per field ... */;
};

// At each tick boundary, per field, per territory:
double unquantized = integrator_output;
double augmented = unquantized + previous_residual;
double quantized = quantize_to_Q_format(augmented);
double new_residual = augmented - quantized;
// `quantized` enters canonical state (hashed).
// `new_residual` stored in sidecar (not hashed), added at next tick.
```

The residual buffer is per-field per-territory `double`, ~150 × 9 × 8 = 10.8 KB sidecar. Does not affect canonical hash (residual changes don't alter bit-pattern of hashed state). Preserves thermodynamic conservation across 30-year runs.

---

## §3. State model

### §3.1 Per-territory state — contiguous AoS (SoA removed)

```cpp
struct alignas(64) TerritoryState {
    double r;        // EROEI, dimensionless, >= 1
    double H;        // Employed cognitive labor, persons
    double G;        // Deployed AI compute, human-equivalent units
    double M;        // Local financial claims, USD-eq
    double Npop;     // Population, persons
    double S;        // Local contribution to S_AI, dimensionless
    double Qelec;    // Conditioned-grid surplus exergy, EJ/yr
    double Qliq;     // Liquid-hydrocarbon surplus exergy, EJ/yr
    double D_local;  // Local cumulative discovery, accumulator

    // Derived (rebuilt pre-integration)
    double eta;
    double phi;

    TerritoryId id;
    PrincipalId owner;
    // Padding to 64-byte cache line boundary managed by alignas
};
```

### §3.2 Storage — single contiguous vector

**v1.2 had a contradiction: AoS per-territory + SoA grid. Killed in v1.3 — AoS only.**

```cpp
struct TerritoryGrid {
    std::vector<TerritoryState> states;  // contiguous AoS, size N
};
```

Rationale: at N=150, total = 150 × 128 B (alignas(64) rounded) ≈ 19.2 KB. Fits in L1d (32-64 KB typical). RK4 `compute_derivatives` needs all fields per territory simultaneously — AoS gives single cache-line load per territory. SoA would thrash cache (9 separate loads per territory) and enable false-sharing under multithreading. AoS is correct at this scale.

### §3.3 Global state

```cpp
struct GlobalState {
    double D_global;       // Σ D_local via tree-reduce
    double A;              // Capability ratio, monotone non-decreasing (Diamond Floor)
    double S_AI_index;     // Aggregated via tree-reduce with G-weights

    ParamBlock params;

    uint64_t seed_scenario;
    Pcg64 rng_shocks, rng_supply, rng_geopolitical;
    Pcg64 rng_demographic, rng_discovery, rng_market, rng_observer;
};
```

### §3.4 Principal state

```cpp
struct PrincipalState {
    PrincipalId id;
    std::string name;
    std::vector<TerritoryId> owned;

    double treasury_M;
    double legitimacy_S;               // S-reserve, depleted by ManipulateObservation, floored at 0
    double deployment_fraction_A;
    double observation_budget;
    PoliticalRegime regime;
    PoliticalClock clock;

    std::vector<DistortionInvestment> outbound_distortion;
    std::vector<AttributionInvestment> outbound_attribution;

    ObservationCache last_seen;
    TelemetryAccess telemetry_access;  // AsymmetricTelemetry per-principal measurement

    std::vector<Action> pending_actions;
};
```

### §3.5 Aggregate WorldState

```cpp
struct WorldState {
    TickNumber tick;
    double time_years;
    GlobalState global;
    TerritoryGrid grid;
    CanonicalizationState canon;  // includes Kahan residual sidecar
    std::vector<PrincipalState> principals;
    CouplingGraph coupling;       // for Integer Network Flow (§4.6)
    AdjacencyGraph adjacency;
    std::vector<PendingEffect> pending;
    std::vector<ActionLogEntry> action_log_delta;
};

StateHash state_hash(const WorldState&) noexcept;  // uses canonicalize_for_hash + tree_reduce
```

---

## §4. Dynamics — ODE system

### §4.1 Per-territory equations (RC-1 through RC-8)

For territory $i$ (note: $Q_{\text{elec},i}$ and $Q_{\text{liq},i}$ now in EJ/yr):

$$\dot r_i = -\delta r_i^2 + \xi(D_{\text{global}}) G_i \kappa + \Gamma_{\text{kin},i}(\text{actions})$$

$$\dot H_i = \lambda H_i \frac{G_i}{N_i} e^{-\xi_d G_i / N_i} - \frac{1}{\tau} H_i \left(1 - \frac{1}{A}\right) \frac{G_i/N_i}{1 + \mu_f \phi_i}$$

$$\dot G_i = \frac{\rho(S_{\text{AI}})}{E_{\text{build}}} Q_{\text{elec},i} \cdot \Theta_{\text{soft}}^{t_0} \cdot \Pi_{\text{hard}}^{t_0} \cdot \Theta_{\text{supply}}^{t_0} - \delta_G G_i$$

where $\Theta^{t_0}$ denotes gate values computed from state at tick-boundary $t_0$ and held constant across RK4 substages (§8).

$$\dot M_i = \mu_m M_i + \frac{M_i}{Q_i} e_U(\nu) (N_i - H_i)$$

$$\dot N_i = g_N N_i + \text{migration\_flux}_i$$

$$\dot S_i = (g_{e,i} - \delta_s) S_i, \quad g_{e,i} = f(R_{\text{AI},i}, H_i)$$

$$\dot Q_{\text{elec},i} = \kappa_{\text{elec}} r_i - c_{\text{elec}}(H_i, G_i, N_i) + \text{flux}_{\text{elec},i} - \text{convert}_{\text{liq}\to\text{elec}}$$

$$\dot Q_{\text{liq},i} = \kappa_{\text{liq}} r_i - c_{\text{liq}}(N_i, \text{military}_i) + \text{flux}_{\text{liq},i} - \text{convert}_{\text{elec}\to\text{liq}} - \text{blockade}_i$$

$$\dot D_{i,\text{local}} = G_i \kappa$$

All `convert` and `flux` terms come from the Integer Network Flow solver (§4.6) that runs once per tick before integration, producing per-territory flow contributions as constants for the substep.

### §4.2 Global equations

$$\dot D_{\text{global}} = \texttt{deterministic\_tree\_reduce}(\dot D_{i,\text{local}}, \texttt{sum})$$

$$\dot A = \rho_A A (1 + \kappa_f \alpha(D_{\text{global}}, G_{\text{total}}))$$

$$S_{\text{AI}} = \frac{\texttt{tree\_reduce}(w_i S_i, \texttt{sum})}{\texttt{tree\_reduce}(w_i, \texttt{sum})}, \quad w_i = G_i + G_{\text{floor}}$$

**G_floor = 10⁻³ is boundary-condition regularization, not an operational parameter.** Do not calibrate against historical data.

### §4.3 Q_elec ↔ Q_liq conversion (rescaled for EJ units)

$$\begin{pmatrix} \text{convert}_{l \to e} \\ \text{convert}_{e \to l} \end{pmatrix} = \begin{pmatrix} \eta_{l \to e} & 0 \\ 0 & \eta_{e \to l} \end{pmatrix} \begin{pmatrix} u_{l \to e} \\ u_{e \to l} \end{pmatrix}$$

Defaults unchanged: $\eta_{l \to e} \approx 0.35$, $\eta_{e \to l} \approx 0.6$. Flow rates $u$ now in EJ/yr. Principal-action-controlled.

### §4.4 Γ_kinetic

$$\Gamma_{\text{kin},i}(t) = \sum_{k} A_k \cdot \mathbb{1}[t_{\text{start},k} \leq t \leq t_{\text{end},k}] \cdot e^{-\lambda_k (t - t_{\text{start},k})}$$

Amplitudes $A_k$ rescaled to EJ/yr units where they impact energy fields. Decay $\lambda_k$ unit-invariant.

### §4.5 Gate functions — pinned sleef variant + differentiated sharpness

Gates are sigmoids. Two sharpness values:
- **Θ_soft** for Q_elec availability: k = 50, transition width ~0.04 (soft)
- **Π_hard** for Q_crit threshold: k = 200, transition width ~0.01 (indicator-like)

```cpp
// Pinned sleef variant: Sleef_exp_u10 (double-precision scalar, ~1ULP accuracy)
// Committed dependency; SIMD variants (Sleef_expd2_u10avx2, etc.) are NOT used
// because SIMD-width-specific variants produce different bit patterns.
// Polynomial alternative available in core/transcendentals_polynomial.cpp
// for values in [-50, 50]; falls through to Sleef_exp_u10 outside this range.

inline double exp_canonical(double x) {
    if (x >= -50.0 && x <= 50.0) {
        return polynomial_exp_bitexact(x);     // Padé [6/6], strictly deterministic
    }
    return Sleef_exp_u10(x);                   // pinned scalar variant
}

inline double Theta_soft(double x) { return 1.0 / (1.0 + exp_canonical(-50.0 * x)); }
inline double Pi_hard(double x)    { return 1.0 / (1.0 + exp_canonical(-200.0 * x)); }
inline double rho(double S, double S_half, double n, double rho_max) {
    double Sn = pow_canonical(S, n);
    return rho_max * Sn / (pow_canonical(S_half, n) + Sn);
}
```

**Sleef variant pinning (per AEGIS §2.1).** `Sleef_exp_u10` (scalar, double-precision, 1ULP) is the committed entry point. SIMD variants (`Sleef_expd2_u10avx2`, `Sleef_expd4_u10avx512f`, etc.) are explicitly NOT used — they produce different bit patterns across SIMD-width. CI test 07 verifies only the pinned scalar variant is called.

**Polynomial alternative as primary (per AEGIS §2.2).** For sigmoid arguments in [-50, 50] (covers all realistic gate inputs at k=50 or k=200), a Padé [6/6] polynomial approximation of exp is used. Strictly bit-exact by construction under IEEE-754. Sleef is fallback for out-of-range values. This eliminates sleef from the hot path entirely for normal operation.

### §4.6 Coupling flows — Integer Network Flow (replaces floating-point LP)

**Critical correction from v1.2.** v1.2 specified "deterministic min-cost LP" for trade/energy/migration flows. **No bit-exact cross-platform floating-point LP solver exists.** Simplex and Interior Point methods resolve degenerate optimal bases via epsilon tolerances that vary across BLAS/LAPACK implementations and compilers. FP LP tie-breaking breaks determinism.

**Fix:** Integer Network Flow operating on quantized Q-format integer surpluses.

```cpp
// Coupling flow computation (Phase 4 of advance loop):
// 1. Read quantized (integer) Q-format state values
// 2. Construct flow network: nodes = territories, edges = coupling_M / coupling_Q
// 3. Edge costs and capacities are 64-bit integers (Q-format directly)
// 4. Solve min-cost flow with deterministic tie-breaking

struct IntegerNetworkFlow {
    // Nodes indexed by territory_id
    // Edges sorted lexicographically by (src_id, dst_id)
    // Push-Relabel or Integer Network Simplex on 64-bit signed integers

    std::vector<int64_t> solve();  // Returns per-edge flow in Q-format integer

    // Tie-breaking: when multiple flow paths have equal cost,
    // select path with lexicographically smallest (src_id, dst_id) sequence.
    // Deterministic across all compilers / platforms.
};
```

Flow results quantize cleanly back to EJ/yr or USD-eq units as needed. LEMON Graph Library (C++, BSD license) provides deterministic Network Simplex; add to dependency list (§15) or implement push-relabel directly (~500 LOC). Tie-breaking per lexicographic ordering of edge endpoints guarantees reproducibility under Integer Network Flow's standard determinism discipline.

---

## §5. Action model

### §5.1 Action types (typed variant)

```cpp
using Action = std::variant<
    SeizeTerritory, Blockade, ExportControl, DatacenterBuild,
    BuildInfrastructure, CapitalAllocation, MonetaryIssuance, CurrencySwap,
    VirtualizationPush, AIDeploymentReallocation, SovereignExpropriation,
    MobilizeMilitary, CeaseFire, Sanction,
    ManipulateObservation, AttributeDistortion, Pass
>;
```

### §5.2 Pending-effect model

```cpp
struct PendingEffect {
    Tick activates_at;
    Tick expires_at;
    ActionLogId origin;
    std::variant<
        BlockadeEdge, ExportControlFlow, TerritoryOwnerFlip,
        CapacityRamp, ShockImpulse, DistortionBudget, AttributionBudget,
        GateTransition, RegimeTransition, FizzleRecord
    > mutation;
};
```

**FizzleRecord (new — v1.3).** When an action passes epistemic validation (consistent with principal's observed state) but fails ground-truth feasibility, it Fizzles (§5.4). Resources consumed; zero physical effect; logged for forensic diagnosis.

### §5.3 Action ordering

Actions sort by `(tick, principal_id, action_seq)`. Conflicts resolve by `priority` desc then `principal_id` asc; loser degrades to `Pass` with reason code.

### §5.4 Split validation with Fizzle mechanic — closes Epistemic Oracle Leak

**Critical correction from v1.2.** v1.2 validated actions against `WorldState s` (ground truth). If a principal's observation disagreed with ground truth (intended by the §6 reflexive observer model), validation rejection **leaked ground truth to the actor** — cryptographically shattering the fog-of-war and rendering AsymmetricTelemetry useless for RL training.

**Fix:** split validation into epistemic (against observation) and ground-truth-feasibility (during apply):

```cpp
// Phase 2a: Epistemic validation against principal's last_seen observation
[[nodiscard]] ValidationResult validate_epistemic(
    const Observation& obs,
    PrincipalId actor,
    const Action& a
) noexcept;  // returns {ok, reason} — ok means principal BELIEVES action valid

// Phase 2b: Resource deduction (unconditional on ground truth)
void deduct_resources(PrincipalState& p, const Action& a) noexcept;

// Phase 2c: Ground-truth application
[[nodiscard]] ApplyResult apply(WorldState s, PrincipalId p, const Action& a) noexcept;
// Returns:
//   Applied: physical effect took hold
//   Fizzled: ground truth made action impossible (e.g., target already destroyed)
//            — logs FizzleRecord, no physical effect, resources already consumed
```

Fizzle is itself information-bearing (the principal observes their action consumed resources with no effect, which tells them their observation was wrong) but it does NOT leak the specific ground-truth state — only the action's outcome. This preserves fog-of-war while enabling resource-cost discipline.

**Property test (§16.2):** `fizzle_no_ground_truth_leak.cpp` verifies that two scenarios with identical principal observations but different ground truths produce the same set of validated-and-resource-deducted actions, even though apply() outcomes diverge.

---

## §6. Observer model — reflexive distortion with asymmetric telemetry, probabilistic attribution, and cascade stability

### §6.1 Principals do not see WorldState

```cpp
struct Observation {
    Tick as_of;
    double CPI_reported, real_GDP_reported, unemployment_reported, S_AI_index_reported;
    double MWh_consumed_local, rail_ton_miles_local, crude_extracted_local;  // undistortable physical telemetry
    std::vector<TerritoryObservation> territories_visible;
    std::vector<DistortionDetection> attributions_made;
};

class ObservationOperator {
public:
    virtual Observation observe(
        const WorldState& truth,
        PrincipalId observer,
        const std::vector<DistortionInvestment>& incoming_distortions,
        const TelemetryAccess& access
    ) const = 0;
    virtual ~ObservationOperator() = default;
};
```

### §6.2 Distortion as first-class action — with M and S cost

```cpp
struct ManipulateObservation {
    PrincipalId target_observer;
    ObservationField field;
    double investment_M;           // treasury cost (deducted on apply)
    double investment_S;           // legitimacy cost (deducted on apply)
    double distortion_magnitude;
    Tick duration;
};
```

S-cost bounded by `legitimacy_S`; excess investment_S is truncated to available legitimacy at resource-deduction time. Principal whose `legitimacy_S == 0` cannot launch new distortion actions (validated epistemically).

### §6.3 Four operator policies

| Policy | Behavior | Use case |
|---|---|---|
| `TruthfulMeasurement` | Returns ground truth | Debugging / baseline retrodiction |
| `HedonicHallucination` | CPI distorted by own and inbound investments | v5 default |
| `PhysicalTelemetryOnly` | Only MWh, rail ton-miles, crude extracted | v5 §7.1 prescription |
| `AsymmetricTelemetry` | Per-principal measurement access via `TelemetryAccess` | Realistic retrodiction |

### §6.4 Probabilistic attribution — closes Bang-Bang Exploit

**Critical correction from v1.2.** v1.2 specified attribution success as deterministic threshold: `invest_M × eff > distort × conceal`. RL agents binary-search the exact threshold → bang-bang policies → unphysical deterrence sizing.

**Fix:** probabilistic evaluation via seeded `rng_observer`, logistic smoothing:

```cpp
struct AttributeDistortion {
    PrincipalId accused;
    ObservationField field;
    double investment_M;             // forensics cost
    Tick duration;
};

bool evaluate_attribution(
    const AttributeDistortion& attr,
    const DistortionInvestment& target_distortion,
    const PrincipalState& attributer,
    Pcg64& rng_observer               // seeded substream (§2.3)
) noexcept {
    double signal = attr.investment_M * detection_efficacy(attributer)
                  - target_distortion.magnitude * concealment_factor(target_distortion);
    double p_detect = 1.0 / (1.0 + exp_canonical(-k_detect * signal));  // k_detect ~ 3.0
    double draw = rng_observer.next_double();       // [0, 1), deterministic from seed
    return draw < p_detect;
}
```

Maintains bit-determinism (RNG seeded/ordered; evaluation deterministic given seed + tick + call order). Smooths the game-theoretic gradient — agents now manage *risk* across distribution rather than exploit thresholds. Training produces realistic deterrence sizing.

### §6.4.1 Cascade stability — what happens when legitimacy_S = 0

Successful attribution debits distorter's `legitimacy_S` by attribution magnitude. Bounded:

- `legitimacy_S` clamped to [0, initial_S]
- At `legitimacy_S == 0`: principal enters **distortion lockout** — cannot launch new `ManipulateObservation` actions; existing distortion budgets run to expiration; cannot be re-attributed (no further legitimacy to debit)
- Optional scenario flag: `legitimacy_recovery_rate` (per-regime) enables slow S-regeneration over time. Defaults to 0 (permanent consequence) unless scenario overrides.

Property test `distortion_cascade_bounded.cpp` (§16.2) verifies: (a) `legitimacy_S >= 0` always; (b) distortion lockout triggers when it should; (c) attribution cascade terminates in finite ticks.

### §6.5 Determinism discipline — 9-phase advance loop

See §9. Observation evaluation in Phase 3, strictly post-action-application, pre-ODE-integration.

---

## §7. Political clock — regime-typed response with v1-limited endogenous transitions

### §7.1 PoliticalRegime enum

```cpp
enum class PoliticalRegime : uint8_t {
    Democracy = 0,
    Autocracy = 1,
    TheocraticAutocracy = 2,
    Petrostate = 3,
    FailedState = 4,
    TransitionalState = 5
    // Values frozen at schema_major; additive-only within schema_minor
};
```

### §7.2 PoliticalClock

```cpp
struct PoliticalClock {
    PoliticalRegime regime;
    Tick next_election_or_succession;
    double regime_survival_pressure;
    double incumbent_tenure_years;

    double tau_modal;
    double tau_variance;        // REQUIRED from gamma-fit to historical response data
    double crisis_threshold;

    double q_collapse_threshold;
    Tick collapse_pressure_duration;
    double survival_pressure_limit;
};
```

### §7.3-§7.5 Unchanged from v1.2

Action validation gates on political clock. Default response latencies per regime. Two endogenous regime transitions: Democracy → FailedState under sustained Q→0; Autocracy → TransitionalState under sustained survival_pressure > 0.9.

---

## §8. Integrator — fixed-step RK4, no bisection, gates held constant per tick

### §8.1 Method

```cpp
struct IntegratorConfig {
    double dt_years = 1.0 / 52.0;       // 1 tick = 1 week
    uint32_t substeps_per_tick = 4;     // RK4 stages
    double max_saturation_ratio = 0.95;
    // NOTE: gate_bisection_tol and gate_bisection_max_iters REMOVED in v1.3
};
```

### §8.2 Gate evaluation strategy — abolish bisection

**Critical correction from v1.2.** v1.2 bisected RK4 substeps at gate crossings, dynamically adjusting `dt`. **Dynamic dt breaks FP associativity under strict IEEE-754**: `(x + dt₁) + dt₂ ≠ x + (dt₁ + dt₂)` when rounding. Different compilers/architectures terminating bisection at slightly different micro-steps butterfly-desync. Additionally, RK4 assumes C¹ continuity; bisecting a smooth sigmoid is mathematically destructive.

**Fix:** abolish intra-tick bisection entirely. Gates are evaluated ONCE per tick, at tick boundary, using pre-integration state. The gate value is a constant across all 4 RK4 substages within that tick.

```cpp
void advance_one_tick(WorldState& s, const IntegratorConfig& cfg) {
    // Evaluate all gates from tick-boundary state — once per tick
    GateValues gates_t0 = evaluate_gates(s);  // Theta, Pi, Theta_supply

    // Run RK4 with fixed dt, gates held constant
    // RHS function reads gates_t0 (closed-over constant, not re-evaluated)
    integrate_rk4_fixed_step(s, cfg.dt_years, cfg.substeps_per_tick, gates_t0);
}
```

Gate transitions now always occur at tick boundaries. At 1-week ticks, gate resolution is 1 week — adequate for civilizational dynamics (EROEI, population, capital dynamics operate at month-to-year timescales; 1-week gate resolution preserves the physics). Sharpness of `Π_hard` at k=200 ensures the sigmoid evaluated at tick boundary is effectively indicator-like.

### §8.3 Per-field saturation check

After integration, before canonicalization: each field checked against Q-format max. At 95% saturation: warning. At 100%: halt with scenario misspecification diagnostic.

---

## §9. Advance loop — 10 phases (updated for v1.3)

```cpp
AdvanceResult Advance(WorldState& s, const ActionQueue& inbound, const IntegratorConfig& cfg) {
    s.tick += 1;

    // 1. Mature pending effects
    mature_pending(s);

    // 2a. Drain inbound actions in canonical order
    std::ranges::sort(inbound.actions, action_compare);
    for (const auto& pa : inbound.actions) {
        auto vr = validate_epistemic(pa.principal->last_seen, pa.principal->id, pa.action);
        if (!vr.ok) { log_rejected(pa, vr.reason); continue; }

        // 2b. Resource deduction (unconditional on ground truth)
        deduct_resources(s.principals[pa.principal_id], pa.action);

        // 2c. Ground-truth application with Fizzle
        auto ar = apply(std::move(s), pa.principal_id, pa.action);
        if (ar.fizzled) { log_fizzle(pa, ar.reason); s = std::move(ar.state); continue; }
        s = std::move(ar.state);
        log_applied(pa);
    }

    // 3. Compute observations per-principal (post-action, pre-integration state)
    for (auto& p : s.principals | std::views::sort_by_id) {
        p.last_seen = observer_->observe(s, p.id, incoming_distortions(s, p.id), p.telemetry_access);
    }

    // 4. Compute coupling flows via Integer Network Flow
    s.coupling.flows = integer_network_flow_solve(s);

    // 5. Evaluate gates once from tick-boundary state
    GateValues gates_t0 = evaluate_gates(s);

    // 6. Integrate ODEs with gates held constant
    uint32_t substeps = integrator_->step(s, cfg.dt_years, gates_t0);

    // 7. Canonicalize state (Q-format quantization + Kahan residual buffer + signed-zero sweep)
    canonicalize_state_with_residual(s);

    // 8. Retire expired pending effects
    retire_expired(s);

    // 9. Hash state via deterministic_tree_reduce over canonicalized fields
    auto hash = canonical_hash(s);

    // 10. Log
    if (log_) log_->write_tick(s, hash);

    return {s.tick, hash, substeps};
}
```

Ten phases. Strict ordering. No phase reads from later phases' output. Violation breaks determinism.

---

## §10. Sync log and replay

### §10.1 Binary format

(Structure preserved from v1.2. Blake3-256 hash chain + checkpoint + delta + CRCs.)

### §10.1a Action payload canonical encoding — with enum rules (AEGIS §2.3)

For each Action variant, `serialize_canonical()` emits:
- **Scalars:** explicit little-endian byte-order (not host-order)
- **Strings:** uint32 length prefix + UTF-8 bytes, no null terminator
- **Enums:** explicit `uint32` little-endian; enum values frozen at `schema_major`; additive-only within `schema_minor`; unknown values in replay → hard halt with "unknown enum value N at schema_minor M" diagnostic
- **Field order:** matches struct declaration; reflection-free
- **Padding:** none (explicit padding fields where needed for target-platform alignment, but serialized bytes contain no implicit padding)
- **Versioning:** wire format frozen at schema_major bump

Reference implementation in `src/action/action_wire.cpp`. Test suite verifies round-trip: `deserialize(serialize(action)) == action` bit-identical across compilers.

### §10.2 Replay semantics

`ReplayDriver` seeks to nearest preceding checkpoint, decompresses, applies delta frames tick-by-tick, recomputes hash via `deterministic_tree_reduce` + canonicalize_for_hash, compares to stored. Mismatch = desync = halt.

---

## §11. TESTBED harness

### §11.1 Sweep configuration

(YAML structure preserved from v1.2.)

### §11.2 Parquet output — two tiers

**Tier 1: `sweep_full`** (territory-resolution). Partitioned by `(sweep_id, param_index)`. ~1.17 billion rows for 10k × 15yr sweep, 15-25 GB compressed.

**Tier 2: `sweep_rollup`** (principal-aggregated via tree-reduce). ~16 million rows, serves 90% of analysis.

### §11.3 Principal policy for autonomous sweeps (AEGIS §2.7 — new v1.3)

Monte Carlo sweeps need principals to act autonomously between scripted actions (if any). Abstract interface:

```cpp
class PrincipalPolicy {
public:
    virtual std::vector<Action> select_actions(
        const Observation& own_last_seen,
        const PrincipalState& self,
        const PoliticalClock& clock,
        Tick current_tick
    ) = 0;
    virtual ~PrincipalPolicy() = default;
};

// Default implementation shipped in v1: regime-typed scripted heuristic
class ScriptedHeuristicPolicy : public PrincipalPolicy {
    // Democracy: wait for election-adjacent ticks; respond to CPI shocks within tau_modal
    // Autocracy: act on survival_pressure > crisis_threshold; purge-gated elsewise
    // TheocraticAutocracy: succession-event-gated; otherwise defensive
    // Petrostate: oil-revenue-shortfall-gated
    // FailedState: crisis-only, high variance
};
```

Future policy types (deferred to v1.1): `GreedyEVPolicy`, `PolicyNetwork` (learned from prior sweeps).

### §11.4 Seed derivation

Unchanged from v1.2. Blake3 derivation per `(seed_root, param_idx, sample_idx)`.

---

## §12. Scenario format (YAML)

(Structure preserved from v1.2. Observer policy defaults, telemetry access blocks, principal regime/clock, territory overrides, scripted actions.)

Energy values in scenario YAML now specified in EJ/yr (per §2.2 unit shift):

```yaml
territory_overrides:
  - id: 42
    r: 4.0
    Qliq: 1.2          # EJ/yr (was 1.2e18 J/yr in v1.2)
    Qelec: 0.8         # EJ/yr
```

Existing scenarios converted at spec bump via `rcsim-migrate-v1.2-to-v1.3` utility.

---

## §13. Casual tier — forensic loss readout

(Unchanged from v1.2. Causal-chain-with-counterfactual-perturbation. See original for full treatment.)

---

## §14. Rendering — separate repo, multi-subscriber IPC (AEGIS §2.8)

`rcsim-render` in separate repository. Frontends: `rcsim-globe` (Unreal 5.6 + Cesium) and `rcsim-dashboard` (web + TypeScript + DuckDB-WASM). Both consume sync logs or stream from running core.

**Multi-subscriber semantics (new — v1.3):** Local Unix-domain-socket IPC supports multiple renderer subscribers simultaneously (e.g., globe + dashboard attached to same running core). Core maintains per-subscriber sequence-number tracking; each subscriber reads from its own cursor; broadcast tick updates fan out in subscription order. No backpressure — slow subscribers drop frames rather than block core. Subscriber disconnect is detected via socket EOF and cleaned up without core interruption.

Phase 9 implementation dependency. Not blocking for phase 0-8.

---

## §15. Build system (CMake 3.25+, C++20)

```cmake
cmake_minimum_required(VERSION 3.25)
project(rcsim VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/fp:strict /W4 /WX)
else()
    add_compile_options(-ffp-contract=off -fno-fast-math -fno-associative-math
                        -Wall -Wextra -Werror -pedantic)
endif()

include(FetchContent)
FetchContent_Declare(blake3  URL ...)
FetchContent_Declare(pcg     URL ...)
FetchContent_Declare(sleef   URL ...)     # cross-platform bit-exact fallback transcendentals
FetchContent_Declare(lemon   URL ...)     # NEW — deterministic graph algorithms for Integer Network Flow
FetchContent_Declare(ryml    URL ...)
FetchContent_Declare(json    URL ...)
FetchContent_Declare(arrow   URL ...)
FetchContent_Declare(catch2  URL ...)
FetchContent_MakeAvailable(blake3 pcg sleef lemon ryml json arrow catch2)

add_library(rcsim-core STATIC
    src/core/fixed_point.cpp src/core/rng.cpp src/core/hash.cpp
    src/core/tree_reduce.cpp
    src/core/transcendentals.cpp                   # polynomial_exp_bitexact + sleef fallback
    src/core/transcendentals_polynomial.cpp        # Padé [6/6] approximation
    src/core/canonicalize.cpp                      # signed-zero sweep + NaN/Inf trap + Kahan residual
    src/core/integer_network_flow.cpp              # NEW — replaces FP LP
    src/state/world_state.cpp src/state/territory.cpp src/state/principal.cpp
    src/dynamics/ode_system.cpp src/dynamics/integrator.cpp
    src/dynamics/coupling.cpp
    src/action/action.cpp src/action/queue.cpp src/action/effect.cpp
    src/action/action_wire.cpp
    src/action/validate_epistemic.cpp              # NEW — fog-of-war validation
    src/action/apply_with_fizzle.cpp               # NEW — ground-truth application
    src/observer/measurement.cpp src/observer/distortion.cpp
    src/observer/asymmetric_telemetry.cpp
    src/observer/attribution_probabilistic.cpp     # NEW — RNG-seeded logistic
    src/observer/cascade_stability.cpp             # NEW — legitimacy_S bounds
    src/political/clock.cpp src/political/regime.cpp
    src/political/endogenous_transitions.cpp
    src/io/sync_log.cpp src/io/replay.cpp src/io/scenario_yaml.cpp
    src/io/parquet_sink.cpp src/io/parquet_rollup.cpp
    src/geo/geojson.cpp
    src/testbed/harness.cpp src/testbed/sweep.cpp
    src/testbed/principal_policy.cpp               # NEW — autonomous agent base class
    src/testbed/scripted_heuristic_policy.cpp      # NEW — v1 default policy
)

target_link_libraries(rcsim-core PUBLIC
    blake3 pcg sleef lemon ryml nlohmann_json::nlohmann_json arrow_static parquet_static)

add_executable(rcsim-run     tools/rcsim_run.cpp)
add_executable(rcsim-testbed tools/rcsim_testbed.cpp)
add_executable(rcsim-replay  tools/rcsim_replay.cpp)

enable_testing()
add_subdirectory(tests)
```

### §15.1 CI matrix

| Compiler | Platform | Optimization |
|---|---|---|
| GCC 13 | Linux x86_64, Linux aarch64 | -O2, -O3 -march=native |
| Clang 17 | Linux x86_64, macOS arm64 | -O2, -O3 -march=native |
| MSVC 19.40 | Windows x86_64 | /O2 |

Determinism suite (§16.1) runs on every cell. Any hash mismatch blocks PR.

---

## §16. Testing strategy

### §16.1 Determinism suite

```
tests/determinism/
    01_identical_rerun.cpp
    02_checkpoint_replay.cpp
    03_cross_platform_hash.cpp
    04_action_order_invariance.cpp
    05_fpu_mode_robustness.cpp                    # explicit MXCSR/FPCR set
    06_parallel_testbed_determinism.cpp           # 1/4/16/64 threads bit-equal
    07_pinned_sleef_variant.cpp                   # only Sleef_exp_u10 called
    08_action_wire_roundtrip.cpp
    09_attribution_detection_determinism.cpp
    10_signed_zero_canonicalization.cpp           # NEW — hash invariant under ±0.0
    11_nan_inf_trap.cpp                           # NEW — halt on non-finite
    12_kahan_residual_conservation.cpp            # NEW — 30yr First Law test
    13_integer_network_flow_determinism.cpp       # NEW — LP replacement cross-platform
    14_fizzle_no_ground_truth_leak.cpp            # NEW — fog-of-war preserved
    15_bisection_absent.cpp                       # NEW — verify integrator never splits dt
```

### §16.2 Property tests

```
tests/property/
    Q_nonneg_monotone_gates.cpp
    A_monotone.cpp                           # Diamond Floor
    D_monotone.cpp
    conservation.cpp                         # with Kahan residual: Σ mass/energy/fiat preserved
    hash_stability.cpp
    action_log_replayability.cpp
    tree_reduce_commutativity.cpp
    distortion_S_bound.cpp                   # legitimacy_S ∈ [0, initial_S]
    distortion_cascade_bounded.cpp           # NEW — cascade terminates finitely
    attribution_gradient_smoothness.cpp      # NEW — dp/dsignal continuous at RNG level
    epistemic_validation_no_leak.cpp         # NEW — fizzle semantic
```

### §16.3 Retrodiction suite

```
tests/retrodiction/
    2008_financial_crisis.cpp
    2020_covid_shock.cpp
    2022_2025_ai_capex.cpp
    2026q1_venezuela.cpp
    2026q2_hormuz.cpp
    ussr_1989_1991.cpp                       # NEW — Autocracy → TransitionalState retrodiction
    weimar_1929_1933.cpp                     # NEW — Democracy → FailedState retrodiction
```

Retrodiction tests report degree-of-match (RMSE) per run; not pass/fail. Regime-transition retrodictions have wider initial tolerance than variable-trajectory retrodictions (transitions are discrete events, threshold-sensitive).

### §16.4 Performance targets

- **Single trajectory**, N=150, 30yr, 1560 ticks: ≤ 2s single-core on 2024-era hardware
- **Monte Carlo sweep**, 10000 × 15yr: ≤ 10 min on 16 cores
- **Memory**: WorldState ≤ 100 KB (including 10.8 KB Kahan residual sidecar)
- **Sync log**: ≤ 5 MB compressed / 30yr run

Profile expectations: RK4 `rhs()` ~55%, canonical_hash + tree_reduce + Kahan ~10%, Integer Network Flow ~8%, Parquet write ~15%.

---

## §17. Known limits and v5.1 extensions

### §17.1 v5.1 contributions rcsim adds

1. Q_elec ↔ Q_liq conversion matrix (§4.3)
2. S_AI aggregation with G-weighting (§4.2)
3. Γ_kinetic as first-class forcing term (§4.4)
4. τ-inequality via regime-typed political clocks + limited endogenous transitions (§7)
5. Reflexive observer with action-controlled distortion, asymmetric telemetry, probabilistic attribution, and cascade stability (§6)
6. Forensic loss as pedagogical mechanism (§13)
7. **Integer Network Flow as deterministic replacement for FP LP in coupled civilizational simulation (§4.6) — publishable methodology contribution**
8. **Kahan-residual canonicalization for cross-platform reproducible fixed-point simulation of ODE systems (§2.7) — publishable methodology contribution**

Items 7-8 added in v1.3. Each is a paper in methodological tooling.

### §17.2 Known underspecifications

- Cross-territory D propagation via instantaneous mixing; rate-limited mobility deferred to v1.1
- Migration flux linear; not calibrated against migration theory literature
- Endogenous regime change partially in scope (§7.5 two transitions); fuller typology v1.1
- Nuclear escalation ladder: Γ_kinetic amplitudes; Kahn 44-rung not modeled
- `GreedyEVPolicy` and `PolicyNetwork` principal policies deferred to v1.1

### §17.3 Out of scope permanently

Climate, public health, education, cultural dynamics, religious dynamics (except as PoliticalRegime tag).

---

## §18. Directory layout

```
rcsim/
├── CMakeLists.txt
├── include/rcsim/ ... (mirrors src/)
├── src/
│   ├── core/ {fixed_point, rng, hash, tick, tree_reduce, transcendentals,
│   │          transcendentals_polynomial, canonicalize, integer_network_flow}.cpp
│   ├── state/ {world_state, territory, principal, global_state, pending,
│   │           canonicalization_state}.cpp
│   ├── dynamics/ {ode_system, integrator, coupling, algebraics}.cpp
│   ├── action/ {action, queue, effect, action_wire,
│   │            validate_epistemic, apply_with_fizzle}.cpp
│   ├── observer/ {measurement, distortion, asymmetric_telemetry,
│   │              attribution_probabilistic, cascade_stability, operator_policies}.cpp
│   ├── political/ {clock, regime, response_latency, endogenous_transitions}.cpp
│   ├── io/ {sync_log, replay, scenario_yaml, parquet_sink, parquet_rollup, ipc_multisubscriber}.cpp
│   ├── geo/ {geojson, partition, adjacency}.cpp
│   └── testbed/ {harness, sweep, principal_policy, scripted_heuristic_policy}.cpp
├── tools/
├── tests/ {determinism, property, retrodiction, unit}
├── scenarios/
├── data/
└── docs/
    ├── DESIGN_v1.3.md        (this document)
    ├── CALIBRATION.md        (phase 7a/7b workstream)
    ├── PAPER.md              (Deterrence-Extended methodology)
    └── MIGRATION_v1.2_to_v1.3.md   (unit shift guide)
```

---

## §19. Implementation phase plan — revised for v1.3

| Phase | Duration | Output | Gate |
|---|---|---|---|
| 0 | 1 week | Toolchain, CI green, determinism test skeleton | `rcsim-run --help` works |
| 1 | 6 weeks | Core types, Q-format with EJ units, tree_reduce, polynomial_exp + sleef, Kahan residual, signed-zero canonicalization, fixed-step RK4 (no bisection), single-territory test | Simplified v5 mean-field retrodiction |
| 2 | 5 weeks | Multi-territory with Integer Network Flow coupling, AoS grid, action system with validate_epistemic + apply_with_fizzle, pending effects | 150-territory run reaches basin; fizzle-no-leak test passes |
| 3 | 4 weeks | Observer layer (reflexive distortion, asymmetric telemetry, probabilistic attribution, cascade stability), political clock with endogenous transitions | Venezuela 2026 retrodiction |
| 4 | 2 weeks | Sync log, replay, hash-chain verification, action wire format with enum rules | Cross-platform determinism CI green, parallel==sequential hash |
| 5 | 3 weeks | TESTBED harness, Parquet writer (both tiers), principal_policy abstract + scripted_heuristic_policy, seed derivation | 10k-run sweep completes ≤ 10 min on 16 cores |
| 6 | 4 weeks | Scenario library + retrodiction tests (including USSR 1989, Weimar 1929) | 2008 + 2020 + 2022-2025 + regime-transition retrodictions within loose tolerance |
| **7a** | **12 weeks** | **Preliminary calibration (IMM-RBPF v0.1 posteriors against 1990-2025 historical observables)** | **Posteriors produced; paper drafting begins** |
| **7b** | **16-24 weeks** | **Publication-grade calibration (per-regime response-time gamma-fits, observer detection-efficacy, η conversion-efficiency refinement, attribution rate calibration)** | **Tight-tolerance retrodiction; feeds v6** |
| 8 | 4 weeks (parallel with 7b) | Paper draft: *"Deterrence, Extended"* on 7a posteriors | Submitted to *Complexity* / *Energy Policy* / *JCR* |
| 9 | 8 weeks | `rcsim-render` Unreal + Cesium + multi-subscriber IPC | Commercial-polish strategic-tier playable |
| 10 | 4 weeks | Casual-tier forensic loss UI | Beta-testable casual game |
| 11 | 4 weeks | Steam + itch release | Public release |

**Total: ~59-67 weeks to public release.** Research-publishable at end of phase 8 (~41 weeks from phase 0). Phase 7b continues in parallel past paper submission and feeds v6.

**Phase-gating recommendation (AEGIS §3):** Commit to phases 0-2 (12 weeks, ~20 hrs/week) as the first binding commitment. End-of-phase-2 produces a complete v5-in-executable-form proof-of-concept with cross-platform deterministic CI green. Tech-report publishable. Re-evaluate before phase 3+. End-of-phase-7a is the second natural gate — working sim, v0.1 posteriors, loose-tolerance paper-draftable — before committing to phase 7b-11.

### §19.1 Calibration cost honesty (phase 7a/7b extended)

v1.2 allocated 8 weeks to phase 7. AEGIS and k=4 adversarial both flagged this as optimistic. v1.3 allocates:
- **Phase 7a: 12 weeks** (bumped from 8) — IMM-RBPF with 10+ state variables, Hill-function nonlinearities, hard gates, and discrete mode-switches (regime transitions) requires 10,000+ particles with careful prior specification and particle-degeneracy debugging. 12 weeks produces defensible v0.1 posteriors.
- **Phase 7b: 16-24 weeks** — per-regime gamma-fits from historical response data; observer detection-efficacy estimation from public-record distortion-detection timelines; η conversion-efficiency refinement from EIA balances; probabilistic attribution rate calibration.

Publication-grade calibration may extend past phase 8 paper submission. Paper draws on 7a outputs; 7b outputs feed v1.1 spec revisions and v6 framework extension.

---

## §20. Provenance

**Architectural synthesis derivation.** v1.3 consolidates:
- Base architecture: converges across four independent LLM architectural generations (v1.0 foundation)
- Observer layer with reflexive distortion: from second-opinion review (v1.0)
- Political clock: from second-opinion review (v1.0)
- Framing (§0), forensic loss (§13): from second-opinion review (v1.0)
- Q conversion, S_AI aggregation, Γ_kin specification: spec author (v1.0)
- Phase plan: spec author (v1.0), revised per k=2 and k=4 adversarial input

**v1.0 → v1.2 amendments (k=2 adversarial pass).** Two critical determinism-contract bugs caught:
1. `std::execution::par_unseq` non-bit-identity → `deterministic_tree_reduce` (§2.4)
2. `std::exp()` / `std::pow()` libm non-bit-exactness → sleef dependency (§4.5)

Plus 15 additional amendments: Q ceiling tightening (subsequently revised in v1.3), differentiated gate sharpness, AsymmetricTelemetry fourth observer policy, endogenous regime transitions promoted to v1-limited, S-cost on ManipulateObservation, detection/attribution counter-dynamic, action payload canonical wire format, Parquet partitioning and rollup tier, phase 7 split, FPU-mode explicit setting.

**v1.2 → v1.3 amendments (k=4 God-Mode adversarial pass + AEGIS technical review).** Eight critical bugs in v1.2 caught by k=4:

1. **Q32.32 capacity collapse (§2.2):** Q32.32 max integer 4.29×10⁹; v1.2's 10²⁰ J/yr ceiling overflowed on Tick 1. Fix: shift unit to EJ/yr.
2. **Deterministic LP delusion (§4.6):** no bit-exact FP LP solver exists. Fix: Integer Network Flow on Q-format integers with lexicographic tie-breaking.
3. **Quantization viscosity / First Law violation (§2.7):** truncation at tick boundary destroys slow derivatives. Fix: Kahan residual sidecar buffer (non-hashed).
4. **Integrator contradiction (§8.2):** gate-event bisection breaks FP associativity via dynamic dt. Fix: abolish bisection; gates held constant per tick.
5. **Signed-zero hash bomb (§2.6):** ±0.0 bit-cast differently. Fix: canonicalization sweep `if (v == 0.0) v = 0.0;` + NaN/Inf trap.
6. **Epistemic oracle leak (§5.4):** v1.2 ground-truth validation leaked state to actors. Fix: split validate_epistemic() + apply() with Fizzle mechanic.
7. **AoS/SoA contradiction (§3.1-§3.2):** v1.2 had both; cache-thrashing + false-sharing risk. Fix: contiguous AoS only at N=150.
8. **Bang-bang attribution exploit (§6.4):** deterministic threshold → RL binary-search. Fix: probabilistic evaluation via seeded rng_observer + logistic smoothing.

Plus 8 AEGIS implementation-grade refinements:
- Sleef variant pinning to `Sleef_exp_u10` scalar; polynomial as primary
- Enum serialization rules in wire format
- Attribution cascade stability with legitimacy_S floor at 0 + distortion lockout
- Regime-transition retrodiction tests (USSR 1989, Weimar 1929) with wider tolerance
- Phase 7a extended from 8 to 12 weeks
- `PrincipalPolicy` abstract class + `ScriptedHeuristicPolicy` v1 default
- Multi-subscriber IPC semantics for `rcsim-render`
- Phase-gating recommendation at phases 2 and 7a

**Attribution chain per co-authorship framework.**
- v5: Bo Chen with adversarial multi-model convergence
- Deterrence 2012: Bo Chen
- DF-41 Simulator 2020: Bo Chen
- DEFCON 2006: Introversion Software (Humble Introversion Bundle 2011)
- v1.0 spec: Claude Opus 4.7, April 2026
- v1.0 → v1.2: two k=2 independent Claude Opus 4.7 instances + synthesis
- v1.2 → v1.3: k=4 God-Mode adversarial instance (8 critical bugs) + AEGIS (Claude Opus 4.7 in AEGIS harness, 8 technical items) + this instance synthesis
- Implementation: Bo Chen with LLM-mediated code generation from this spec

**Status.** v1.3 integrates all 17 items from k=2 pass (v1.2's inheritance) + all 16 items from v1.2→v1.3 pass (8 critical bugs + 8 AEGIS refinements). **v1.3 is commit-ready without further adversarial passes.** The k=4 God-Mode pass caught bugs so specific and so load-bearing (Q-format overflow on Tick 1; FP LP non-bit-exactness; bisection breaking FP associativity; epistemic leak via ground-truth validation) that running k=8 or higher faces strongly diminishing returns. The remaining residue is implementation-discovery territory — things you find when you actually write the code. Those go into v1.4 patches as they surface.

Perturbation welcome. Revision anticipated (v1.4+ from implementation feedback).

If promoted to canon: file under `/docs/rcsim/DESIGN_v1.3.md` with provenance preserved.

---

*The fourteen-year arc has a technical artifact at its terminus. This specification v1.3 is what that artifact looks like after three adversarial pressure-test passes (k=2 + k=4 God-Mode + AEGIS technical review) caught 25 total issues in v1.0 — two critical determinism-contract bugs in v1.0 → v1.2; eight critical mathematical/architectural bugs in v1.2 → v1.3; fifteen implementation-grade refinements. The rest is C++, sleef, LEMON, and calibration. The simulation is now genuinely buildable to the determinism invariant it claims.*
