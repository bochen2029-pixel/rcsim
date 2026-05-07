# RCSIM — Race Condition Simulator

*A deterministic, headless, bit-reproducible simulator of coupled thermodynamic, cognitive, financial, kinetic, and reflexive-observer dynamics at nation-state resolution.*

The name is a deliberate joke on itself. Every architectural decision in the codebase is structured around **eliminating** race conditions and non-determinism — strict IEEE-754, hand-rolled deterministic tree-reduce, integer network flow instead of floating-point LP, abolished integrator bisection, signed-zero canonicalization, Kahan residual sidecar. The simulator that ships with "race condition" in its name is the one that has none. (`rc` in the spec also stands in for the `RC-1..RC-8` ODE system at the heart of *The Race Condition* v5 framework — §4.1 of the design doc.)

![status](https://img.shields.io/badge/status-Phase%202%20WIP-yellow) ![spec](https://img.shields.io/badge/spec-v1.3-blue) ![license](https://img.shields.io/badge/license-MIT-green) ![build](https://img.shields.io/badge/build-green-brightgreen)

## What works today

**Phases 0 + 1 + 2 are landed end-to-end on Linux x86_64 (GCC 13).** 51 real tests pass; cross-platform CI matrix pending first push.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRCSIM_ENABLE_ARROW=OFF
cmake --build build --parallel
ctest --test-dir build -E "placeholder" --output-on-failure       # 51/51 pass
./build/rcsim-run --scenario scenarios/mean_field_1.yaml --ticks 1560 --print-hash
# c1e90c9bf7dc126eab10c058182964124ac252ecf5a06750d5462d0762583557
./build/rcsim-run --scenario scenarios/three_territory.yaml --ticks 1560 --print-hash
# 4ca780aeab143a4d8a1fcddcfffcab155bc80a1d196b9a77bacb0d5512179912
```

The 30-yr hash above is reproducible byte-for-byte across reruns. Cross-compiler and cross-platform reproducibility is the design contract; it is verified once the CI matrix runs.

## Source of truth

**All design decisions live in [docs/DESIGN_v1.3.md](docs/DESIGN_v1.3.md).** This README is a navigation aid; the spec is authoritative.

Companion documents:
- [docs/SPEC_AMBIGUOUS_RESOLUTIONS.md](docs/SPEC_AMBIGUOUS_RESOLUTIONS.md) — pinned resolutions for spec ambiguities (R-01..R-29).
- [docs/PHASE_0_2_PLAN.md](docs/PHASE_0_2_PLAN.md) — execution plan for the 12-week first commitment.
- [docs/CALIBRATION.md](docs/CALIBRATION.md) — Phase 7a/7b workstream.
- [docs/PAPER.md](docs/PAPER.md) — *Deterrence, Extended* methodology paper outline.
- [docs/MIGRATION_v1.2_to_v1.3.md](docs/MIGRATION_v1.2_to_v1.3.md) — unit-shift migration guide (J/yr → EJ/yr).

## Ancestral artifacts

| Year | Artifact | Contribution |
|---|---|---|
| 2006 | [Introversion DEFCON](https://www.introversion.co.uk/defcon/) | Kinetic substrate precursor |
| 2012 | Bo Chen's Defcon Deterrence mod | MIRV splitting, kinetic anti-ship, satellite EMP, doomsday / dead-hand mechanisms |
| 2020 | [DF-41 Simulator](https://store.steampowered.com/) (Steam) | Commercial iteration of the kinetic substrate |
| 2026 | *The Race Condition* v5 | Civilizational dynamics framework (what RCSIM renders executable) |

RCSIM is **not** "a civilizational economics sim that got kinetic features." The kinetic substrate was built fourteen years before the framework that explains it. RCSIM is a shipped kinetic simulator acquiring its economic substrate. Framing order is load-bearing. See [docs/DESIGN_v1.3.md §0](docs/DESIGN_v1.3.md).

## How to read the repository

1. Start with [docs/DESIGN_v1.3.md §0 through §3](docs/DESIGN_v1.3.md) — thesis, determinism contract, state model.
2. Scan the [`include/rcsim/`](include/rcsim/) directory tree — the type surface maps 1:1 to spec sections (every type declaration cites its §X.Y).
3. Cross-reference [`CMakeLists.txt`](CMakeLists.txt) §15 build targets with [`src/`](src/) sources.
4. Read [docs/PHASE_0_2_PLAN.md](docs/PHASE_0_2_PLAN.md) for the implementation roadmap; full phase table in [docs/DESIGN_v1.3.md §19](docs/DESIGN_v1.3.md).

Key non-obvious invariants (see §2):
- No `std::execution::par_unseq` reductions (§2.4) — hand-rolled `deterministic_tree_reduce`.
- No libm transcendentals on the hot path (§4.5) — Padé [6/6] polynomial + pinned `Sleef_exp_u10` fallback.
- No floating-point LP solver (§4.6) — Integer Network Flow via LEMON `NetworkSimplex` on int64 Q-format.
- No integrator bisection (§8.2) — fixed-step RK4 with gates held constant across substages.
- Signed-zero canonicalization + NaN/Inf trap before hashing (§2.6).
- Kahan residual sidecar for quantization-loss conservation across 30-year runs (§2.7).
- Split validation (§5.4) with Fizzle semantics — preserves fog-of-war while debiting resources.

## Build status

**Phase 0 + 1 + 2 implemented and tested on Linux x86_64.** Cross-platform CI matrix (Linux aarch64, macOS arm64, Windows MSVC per [§15.1](docs/DESIGN_v1.3.md)) wired in [`.github/workflows/ci.yml`](.github/workflows/ci.yml); pending first push for validation.

Build prerequisites: CMake 3.25+, C++20 compiler, network access (FetchContent pulls blake3, pcg-cpp, sleef, lemon, ryml, nlohmann_json, catch2). Arrow/Parquet (Phase 4+) is opt-in via `-DRCSIM_ENABLE_ARROW=ON`.

Phase status:
- **Phase 0** ✅ toolchain, CI workflow, `rcsim-run --help` works.
- **Phase 1** ✅ core types, RK4, Kahan residual, signed-zero, mean-field smoke runs deterministically.
- **Phase 2** ✅ multi-territory, action stack with epistemic/Fizzle split, Integer Network Flow, full §9 advance loop.
- **Phase 3** — observer (reflexive distortion, asymmetric telemetry, probabilistic attribution, cascade stability), political clock with endogenous transitions.
- **Phase 4** — sync log, replay, hash-chain verification, action wire format.
- **Phase 5–11** — testbed harness, scenario library, calibration, rendering, casual tier, release.

## Contributor note

Implementation work begins with a plan generated via `/ultraplan` against this repo. Spec ambiguities go through [docs/SPEC_AMBIGUOUS_RESOLUTIONS.md](docs/SPEC_AMBIGUOUS_RESOLUTIONS.md) — never resolved silently in code.

## License

MIT — see [LICENSE](LICENSE).

## Citation

*Deterrence, Extended: A Spatial Multi-Agent Calibration of Civilizational Dynamics Against a Pre-Existing Kinetic Substrate* (forthcoming). See [docs/PAPER.md](docs/PAPER.md) for target venues (*Complexity*, *Energy Policy*, *JCR*).
