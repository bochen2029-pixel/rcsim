# CLAUDE.md — rcsim Scaffolding Generation Session

*Target: Claude Code session operating in an empty directory, goal is to produce a complete scaffolded rcsim repository ready for upload to GitHub and consumption by `/ultraplan`.*

*Reading target: read this file, then `DESIGN_v1.3.md`, then execute the scaffolding plan below. Do not begin implementation of simulation logic. Your output is declarations, stubs, test names, build config, and documentation only.*

---

## §0. What you are doing

You are generating a **scaffolded C++20 repository** from the specification in `DESIGN_v1.3.md`. The scaffold must:

1. Match the directory layout in DESIGN_v1.3.md §18
2. Declare every type, struct, enum, and function signature specified in the spec as compile-attemptable headers
3. Contain empty `.cpp` stub files with TODO comments pointing to the relevant spec section
4. Contain empty test files matching the test names in §16.1 through §16.3
5. Contain a working `CMakeLists.txt` that at minimum configures, even if nothing links yet
6. Contain a `README.md` pointing to the spec as the source of truth
7. Contain a `.gitignore` appropriate for C++20 + CMake + common editors

**You are not implementing anything.** Every function body is `// TODO(phase N, §X.Y): implement per DESIGN_v1.3.md §X.Y`. Every test body is `// TODO(phase N): implement test from DESIGN_v1.3.md §16.X`. The scaffold exists so that `/ultraplan` has a real repository to plan against and `/ultrareview` has a real PR structure to review against.

**Do not invent architecture.** Every declaration in every header comes from a specific section of DESIGN_v1.3.md. If the spec is ambiguous on a declaration, leave a `// SPEC_AMBIGUOUS(§X.Y): <what's unclear>` comment in place. Do not resolve the ambiguity yourself — the user will resolve it before implementation.

---

## §1. Execution protocol

### §1.1 Before you begin

1. Read this file (`CLAUDE.md`) end to end.
2. Read `DESIGN_v1.3.md` end to end. It is the authoritative source for every declaration you will emit.
3. Create a todo list with the tasks in §3 below. Mark them as you complete them. The list IS your progress tracker.

### §1.2 During execution

- Generate files in the order specified in §3. Do not jump ahead.
- After every 3-5 files created, run a `view` on the directory tree to verify structure is correct.
- Do NOT attempt to compile. The scaffold is for upload, not for local build. Pre-compilation will waste session tokens on header-dependency resolution that isn't ready yet.
- If you encounter a spec ambiguity that blocks scaffold generation, STOP and report the ambiguity — don't guess.
- If you find yourself writing implementation logic (an actual algorithm, not a declaration), STOP. That is out of scope. Revert to TODO stub.

### §1.3 After completion

Produce a summary:
- File count by directory
- List of `SPEC_AMBIGUOUS` markers encountered
- List of files skipped or deferred with reason
- Git commands the user runs next to initialize and push to GitHub

---

## §2. Naming and style conventions (load-bearing)

**Namespace.** All code goes under `rc::sim` per DESIGN_v1.3.md. Sub-namespaces:
- `rc::sim::core` for fixed_point, rng, hash, tick, tree_reduce, canonicalize, transcendentals
- `rc::sim::state` for world_state, territory, principal, global_state, pending, canonicalization_state
- `rc::sim::dynamics` for ode_system, integrator, coupling, algebraics
- `rc::sim::action` for action, queue, effect, action_wire, validate_epistemic, apply_with_fizzle
- `rc::sim::observer` for measurement, distortion, asymmetric_telemetry, attribution, cascade_stability
- `rc::sim::political` for clock, regime, response_latency, endogenous_transitions
- `rc::sim::io` for sync_log, replay, scenario_yaml, parquet_sink, parquet_rollup, ipc_multisubscriber
- `rc::sim::geo` for geojson, partition, adjacency
- `rc::sim::testbed` for harness, sweep, principal_policy, scripted_heuristic_policy

**File naming.** snake_case for files. `.hpp` for headers (C++, not `.h`). `.cpp` for implementation. Test files prefix with numeric ordinal per §16.1: `01_identical_rerun.cpp`, `02_checkpoint_replay.cpp`, etc.

**Type naming.** PascalCase for types, structs, classes, enums. camelCase for functions and methods. snake_case for variables. `k` prefix for constants: `kDefaultTerritories = 150`.

**Header guards.** Use `#pragma once` (supported by GCC 13, Clang 17, MSVC 19.40 — all target compilers).

**Include order.**
1. Matching header (for .cpp files)
2. Standard library
3. Third-party (blake3, pcg, sleef, lemon, ryml, nlohmann, arrow, catch2)
4. Project (rcsim/...)

**Comments.** Every header declaration that references the spec gets a `// §X.Y` comment. Every TODO gets a phase number and section reference. Example:

```cpp
// §2.4: deterministic tree-reduction for associative ops
// TODO(phase 1): implement per DESIGN_v1.3.md §2.4
template<typename T, typename Op>
T deterministic_tree_reduce(std::span<const T> values, Op binary_op);
```

---

## §3. Scaffolding task list (execute in order)

### Task 1: Create top-level structure

Create these files in order:

1. `.gitignore` (C++, CMake, common editors)
2. `README.md` (minimal, points at `docs/DESIGN_v1.3.md`)
3. `CMakeLists.txt` (top-level, at least configures)
4. `LICENSE` (MIT placeholder — the user will decide final license)

Then create these directories as empty (touch a `.gitkeep` inside each):
- `include/rcsim/core/`
- `include/rcsim/state/`
- `include/rcsim/dynamics/`
- `include/rcsim/action/`
- `include/rcsim/observer/`
- `include/rcsim/political/`
- `include/rcsim/io/`
- `include/rcsim/geo/`
- `include/rcsim/testbed/`
- `src/core/`
- `src/state/`
- `src/dynamics/`
- `src/action/`
- `src/observer/`
- `src/political/`
- `src/io/`
- `src/geo/`
- `src/testbed/`
- `tools/`
- `tests/determinism/`
- `tests/property/`
- `tests/retrodiction/`
- `tests/unit/`
- `scenarios/`
- `data/`
- `docs/`

Verify tree with `view` on repo root.

### Task 2: Copy the spec into docs/

Copy `DESIGN_v1.3.md` → `docs/DESIGN_v1.3.md`. This is the source of truth referenced by every other artifact in the repo.

Create `docs/CALIBRATION.md` with a stub pointing to DESIGN §19.1 (phase 7a/7b).

Create `docs/PAPER.md` with a stub pointing to DESIGN §20 and the framing "Deterrence, Extended" for the eventual publication.

Create `docs/MIGRATION_v1.2_to_v1.3.md` with a stub explaining the unit shift from J/yr to EJ/yr per §2.2.

### Task 3: Core headers (§2 + §3 of spec)

Generate these headers. Each one declares the types named in the spec, with TODO comments for implementation. No bodies beyond type definitions and pure-function signatures.

1. `include/rcsim/core/tick.hpp` — `TickNumber = uint64_t` alias, `Duration` type if spec specifies
2. `include/rcsim/core/fixed_point.hpp` — per-field Q-format functions per §2.2 (quantize, dequantize for each field format; the Q32.32, Q48.16, Q56.8 variants as needed)
3. `include/rcsim/core/rng.hpp` — `Pcg64` wrapper, `Philox4x32` counter-based interface per §2.3
4. `include/rcsim/core/hash.hpp` — `StateHash` type, `canonical_hash()` declaration, `canonicalize_for_hash()` per §2.6
5. `include/rcsim/core/tree_reduce.hpp` — `deterministic_tree_reduce<T, Op>` template per §2.4
6. `include/rcsim/core/transcendentals.hpp` — `exp_canonical()`, `pow_canonical()` wrappers per §4.5
7. `include/rcsim/core/transcendentals_polynomial.hpp` — `polynomial_exp_bitexact()` declaration per §4.5
8. `include/rcsim/core/canonicalize.hpp` — `canonicalize_for_hash()` signed-zero sweep + NaN/Inf trap per §2.6
9. `include/rcsim/core/integer_network_flow.hpp` — `IntegerNetworkFlow` class skeleton per §4.6

### Task 4: State headers (§3 of spec)

1. `include/rcsim/state/territory.hpp` — `TerritoryState` struct exactly as in §3.1, with `alignas(64)` and explicit `uint8_t _pad[32] = {};` for padding determinism (see reviewer flag on §3.1 about implicit padding)
2. `include/rcsim/state/principal.hpp` — `PrincipalState` per §3.4
3. `include/rcsim/state/global_state.hpp` — `GlobalState` per §3.3
4. `include/rcsim/state/pending.hpp` — `PendingEffect` variant per §5.2
5. `include/rcsim/state/canonicalization_state.hpp` — `CanonicalizationState` with Kahan residual sidecar per §2.7
6. `include/rcsim/state/world_state.hpp` — `WorldState` aggregate per §3.5

### Task 5: Dynamics headers (§4 and §8 of spec)

1. `include/rcsim/dynamics/algebraics.hpp` — `Theta_soft`, `Pi_hard`, `rho`, `eta`, `phi`, `e_U`, `xi_of_D` inline functions per §4.5
2. `include/rcsim/dynamics/ode_system.hpp` — `ODESystem` class with `derivative()` and `rhs()` declarations per §4.1
3. `include/rcsim/dynamics/coupling.hpp` — coupling flow computation interface per §4.6
4. `include/rcsim/dynamics/integrator.hpp` — `Integrator` interface + `Rk4Integrator` class per §8.1, NO bisection methods per §8.2

### Task 6: Action headers (§5 of spec)

1. `include/rcsim/action/action.hpp` — `Action` std::variant type with every variant from §5.1, and declarations for each variant struct (SeizeTerritory, Blockade, etc.)
2. `include/rcsim/action/queue.hpp` — `ActionQueue`, `PendingAction`, `action_compare` comparator per §5.3
3. `include/rcsim/action/effect.hpp` — `PendingEffect` mutation variant per §5.2 (including `FizzleRecord`)
4. `include/rcsim/action/action_wire.hpp` — `serialize_canonical()` and `deserialize_canonical()` per §10.1a
5. `include/rcsim/action/validate_epistemic.hpp` — `validate_epistemic()` signature per §5.4
6. `include/rcsim/action/apply_with_fizzle.hpp` — `apply()` returning `ApplyResult` with Fizzle semantics per §5.4

### Task 7: Observer headers (§6 of spec)

1. `include/rcsim/observer/measurement.hpp` — `Observation` struct per §6.1, `ObservationOperator` abstract class
2. `include/rcsim/observer/distortion.hpp` — `ManipulateObservation`, `DistortionInvestment` types per §6.2
3. `include/rcsim/observer/asymmetric_telemetry.hpp` — `TelemetryAccess`, `AsymmetricTelemetry` policy per §6.3
4. `include/rcsim/observer/attribution.hpp` — `AttributeDistortion`, `evaluate_attribution()` probabilistic per §6.4
5. `include/rcsim/observer/cascade_stability.hpp` — `legitimacy_S` bounds and lockout per §6.4.1
6. `include/rcsim/observer/operator_policies.hpp` — `TruthfulMeasurement`, `HedonicHallucination`, `PhysicalTelemetryOnly`, `AsymmetricTelemetry` concrete classes

### Task 8: Political headers (§7 of spec)

1. `include/rcsim/political/regime.hpp` — `PoliticalRegime` enum per §7.1 with explicit `uint8_t` underlying and explicit enumerator values frozen at schema_major per §10.1a
2. `include/rcsim/political/clock.hpp` — `PoliticalClock` struct per §7.2
3. `include/rcsim/political/response_latency.hpp` — regime response latency function signatures per §7.3
4. `include/rcsim/political/endogenous_transitions.hpp` — `Democracy→FailedState`, `Autocracy→TransitionalState` transition functions per §7.5

### Task 9: IO headers (§10 + §11 + §14 of spec)

1. `include/rcsim/io/sync_log.hpp` — `SyncLog` interface, `BinarySyncLog` class, `SyncLogReader` per §10.1
2. `include/rcsim/io/replay.hpp` — `ReplayDriver` per §10.2
3. `include/rcsim/io/scenario_yaml.hpp` — scenario loader per §12
4. `include/rcsim/io/parquet_sink.hpp` — Parquet writer per §11.2 Tier 1
5. `include/rcsim/io/parquet_rollup.hpp` — Parquet writer per §11.2 Tier 2
6. `include/rcsim/io/ipc_multisubscriber.hpp` — local Unix-domain-socket multi-subscriber IPC per §14

### Task 10: Geo headers (§12 of spec)

1. `include/rcsim/geo/geojson.hpp` — GeoJSON territory loader, `TerritoryGeom`, `GeoLoadResult`
2. `include/rcsim/geo/partition.hpp` — principal ownership CSV loader
3. `include/rcsim/geo/adjacency.hpp` — `AdjacencyGraph` derivation from shared polygon edges

### Task 11: Testbed headers (§11 of spec)

1. `include/rcsim/testbed/harness.hpp` — `Testbed` class, `SweepConfig`, `RunResult` per §11.1
2. `include/rcsim/testbed/sweep.hpp` — `ParameterSweep`, `MetricSpec`, `PriorSpec` types
3. `include/rcsim/testbed/principal_policy.hpp` — `PrincipalPolicy` abstract class per §11.3
4. `include/rcsim/testbed/scripted_heuristic_policy.hpp` — `ScriptedHeuristicPolicy` concrete v1 default

### Task 12: Source file stubs

For EVERY header in `include/rcsim/`, create the matching `.cpp` under `src/` with a single-line `#include` of the header and TODO comments for each declared function. Do not implement anything.

Example for `src/core/hash.cpp`:
```cpp
#include "rcsim/core/hash.hpp"

// TODO(phase 1, §2.4, §2.6): implement deterministic_tree_reduce-based canonical hash
// TODO(phase 1, §2.6): implement signed-zero canonicalization + NaN/Inf trap

namespace rc::sim::core {

// TODO
StateHash canonical_hash(const state::WorldState& s) noexcept {
    // TODO(phase 1)
    return StateHash{};
}

// TODO
double canonicalize_for_hash(double v) noexcept {
    // TODO(phase 1, §2.6)
    return v;
}

}  // namespace rc::sim::core
```

### Task 13: Tool executable stubs

1. `tools/rcsim_run.cpp` — `main()` that parses argc/argv and prints `--help`. TODO stubs for everything else.
2. `tools/rcsim_testbed.cpp` — same pattern
3. `tools/rcsim_replay.cpp` — same pattern

### Task 14: Test file stubs

For every test named in DESIGN §16.1, §16.2, §16.3:

- `tests/determinism/01_identical_rerun.cpp` through `tests/determinism/15_bisection_absent.cpp`
- `tests/property/` — per §16.2 filenames (Q_nonneg_monotone_gates.cpp, A_monotone.cpp, etc.)
- `tests/retrodiction/` — per §16.3 filenames (2008_financial_crisis.cpp, etc.)

Each test file contains:
```cpp
#include <catch2/catch_test_macros.hpp>

// TODO(phase N, §16.X): implement test per DESIGN_v1.3.md §16.X
TEST_CASE("placeholder for <test name>", "[<category>][pending]") {
    REQUIRE(false);  // intentionally fails until implemented
}
```

Mark all tests with `[pending]` tag so they can be excluded from default test run until implemented.

### Task 15: Top-level CMakeLists.txt

Generate CMakeLists.txt that:
- Sets project name, version (1.3.0), C++20
- Configures strict IEEE-754 flags per §15
- Declares FetchContent stanzas for blake3, pcg, sleef, lemon, ryml, nlohmann_json, arrow, catch2 (with placeholder URLs marked `# URL_PLACEHOLDER_AT_FIRST_IMPLEMENTATION`)
- Declares `rcsim-core` STATIC library with all src/*.cpp files enumerated
- Declares `rcsim-run`, `rcsim-testbed`, `rcsim-replay` executables
- Enables testing, adds `add_subdirectory(tests)`
- Does NOT require FetchContent URLs to be real — they are placeholders. `#TODO` marker on each.

Also generate `tests/CMakeLists.txt` that iterates the test directories and declares each stub file as a test target with Catch2.

### Task 16: README.md

Generate `README.md` that contains:

- Project name and one-line thesis
- Status badge placeholders (CI, coverage)
- Quick link to `docs/DESIGN_v1.3.md` as source of truth
- Ancestral artifacts list (DEFCON 2006, Deterrence 2012, DF-41 2020, v5 2026)
- How to read the repo: "Start with docs/DESIGN_v1.3.md §0 through §3, then scan include/ directory to see the type surface."
- Build status: "Scaffold only. No implementation. See docs/DESIGN_v1.3.md §19 for phase plan."
- Contributor note: "All implementation begins with a plan generated via `/ultraplan` against this scaffolded repo."

### Task 17: .gitignore

Standard C++ + CMake + JetBrains + VSCode + macOS + common artifacts. Exclude `build/`, `cmake-build-*/`, `.cache/`, `.vscode/`, `.idea/`, `*.o`, `*.so`, `*.a`, `.DS_Store`, Python bytecode (for future Python bindings), Parquet output directories.

### Task 18: SPEC_AMBIGUOUS audit

Scan all generated files for `SPEC_AMBIGUOUS` markers. List them in order of encounter. Do not resolve — the user resolves these before implementation begins.

### Task 19: Final summary

Produce a summary document as your final output:

```markdown
# Scaffold Generation Report

## File count by directory
include/rcsim/core/: N headers
include/rcsim/state/: N headers
...

## SPEC_AMBIGUOUS markers encountered
1. <file>:<line>: <ambiguity description>
...

## Next steps for user
1. Review scaffold structure: `tree rcsim`
2. Resolve SPEC_AMBIGUOUS markers (see list above)
3. Initialize git:
   git init
   git add .
   git commit -m "Initial scaffold from DESIGN_v1.3"
   git remote add origin <github URL>
   git push -u origin main
4. Run `/ultraplan "implement phase 0 of rcsim per DESIGN_v1.3.md"` from Claude Code web
5. Review plan, iterate, commit to phase 0 implementation

## Files NOT generated (deliberate)
- FetchContent URLs are placeholder (will be filled at first implementation session)
- No implementation bodies (by design)
- No scenario YAML content (deferred to phase 2)
- No GeoJSON data file (deferred to phase 2)
```

---

## §4. What you must NOT do

1. **Do not implement any algorithm.** No RK4 stepping, no tree_reduce body, no canonical_hash body. Just declarations.
2. **Do not invent types not in the spec.** Every type you declare must have a direct citation to a DESIGN_v1.3.md section.
3. **Do not resolve SPEC_AMBIGUOUS items.** Log them for the user.
4. **Do not attempt to build, link, or test.** The scaffold is for GitHub upload, not local compilation.
5. **Do not add dependencies beyond what §15 specifies.** If the spec doesn't name it, it doesn't go in CMakeLists.txt.
6. **Do not produce scenario YAML content, GeoJSON content, or historical calibration data.** Those files are deferred to phase 2 implementation per the spec.
7. **Do not cross-edit DESIGN_v1.3.md.** The spec is read-only in this session. Bugs discovered go into a separate `SPEC_BUGS.md` file at repo root.

---

## §5. Recovery if you get lost

If you lose track of state mid-session:

1. Run `view` on the repo root to see current directory structure
2. Count files per directory to identify which tasks are incomplete
3. Consult your todo list
4. Resume at the next incomplete task

If you produce a file and it feels wrong:

1. Compare it against the spec section cited in its TODO comments
2. If mismatch: delete and regenerate from spec
3. If match but still feels wrong: leave SPEC_AMBIGUOUS comment and move on

---

## §6. Completion criteria

You are done when:

1. Every directory in DESIGN §18 exists
2. Every header file named in DESIGN §15 exists with declared types and TODO stubs
3. Every source file in DESIGN §15 exists as a `.cpp` stub
4. Every test file in DESIGN §16.1-§16.3 exists as a Catch2 stub with `[pending]` tag
5. `CMakeLists.txt` exists and configures without requiring FetchContent URLs to resolve
6. `README.md` and `.gitignore` exist
7. The Scaffold Generation Report summary is produced
8. Git commands for initialization are provided to the user

At that point: stop. Hand off to the user to `git init`, push to GitHub, and invoke `/ultraplan`.

---

*This file is the entire instruction set for the scaffolding session. Everything you need to know is in this file + DESIGN_v1.3.md. Do not read other files except the spec. Do not deviate from the task list. Do not skip tasks. Do not implement.*
