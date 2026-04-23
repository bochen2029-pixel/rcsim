# rcsim

*A deterministic, headless, bit-reproducible simulator of coupled thermodynamic, cognitive, financial, kinetic, and reflexive-observer dynamics at nation-state resolution.*

<!-- Status badges (CI / coverage). To be wired once §15.1 matrix is live. -->
![status](https://img.shields.io/badge/status-scaffold-lightgrey) ![spec](https://img.shields.io/badge/spec-v1.3-blue) ![license](https://img.shields.io/badge/license-MIT-green)

## Source of truth

**All design decisions live in [docs/DESIGN_v1.3.md](docs/DESIGN_v1.3.md).** This README is a navigation aid; the spec is authoritative.

## Ancestral artifacts

| Year | Artifact | Contribution |
|---|---|---|
| 2006 | [Introversion DEFCON](https://www.introversion.co.uk/defcon/) | Kinetic substrate precursor |
| 2012 | Bo Chen's Defcon Deterrence mod | MIRV splitting, kinetic anti-ship, satellite EMP, doomsday / dead-hand mechanisms |
| 2020 | [DF-41 Simulator](https://store.steampowered.com/) (Steam) | Commercial iteration of the kinetic substrate |
| 2026 | *The Race Condition* v5 | Civilizational dynamics framework (the thing rcsim renders executable) |

rcsim is **not** "a civilizational economics sim that got kinetic features." The kinetic substrate was built fourteen years before the framework that explains it. rcsim is a shipped kinetic simulator acquiring its economic substrate. Framing order is load-bearing. See [docs/DESIGN_v1.3.md §0](docs/DESIGN_v1.3.md) for the thesis and [docs/PAPER.md](docs/PAPER.md) for the emerging publication framing.

## How to read the repository

1. Start with [docs/DESIGN_v1.3.md §0 through §3](docs/DESIGN_v1.3.md) — thesis, determinism contract, state model.
2. Scan the [`include/rcsim/`](include/rcsim/) directory tree — the type surface maps 1:1 to spec sections (every type declaration cites its §X.Y).
3. Cross-reference [`CMakeLists.txt`](CMakeLists.txt) §15 build targets with [`src/`](src/) sources.
4. Phase plan is in [docs/DESIGN_v1.3.md §19](docs/DESIGN_v1.3.md).

Key non-obvious invariants (see §2):
- No `std::execution::par_unseq` reductions (§2.4) — hand-rolled `deterministic_tree_reduce`.
- No libm transcendentals on the hot path (§4.5) — polynomial Padé + pinned `Sleef_exp_u10`.
- No floating-point LP solver (§4.6) — Integer Network Flow on Q-format integers.
- No integrator bisection (§8.2) — fixed-step RK4, gates held constant across substages.
- Signed-zero canonicalization + NaN/Inf trap before hashing (§2.6).
- Kahan residual sidecar for quantization-loss conservation across 30-year runs (§2.7).
- Split validation (§5.4) with Fizzle semantics to preserve fog-of-war.

## Build status

**Scaffold only. No implementation.** Every function body is a TODO pointing to a specific DESIGN_v1.3.md section. The repository configures but does not compile to a working binary; FetchContent URLs are placeholders filled at first implementation session. See [docs/DESIGN_v1.3.md §19](docs/DESIGN_v1.3.md) for the phase plan.

Phase 0 gate: `rcsim-run --help` works. Everything after is tracked in the phase table.

## Contributor note

**All implementation begins with a plan generated via `/ultraplan` against this scaffolded repo.** Do not modify files outside `docs/SPEC_BUGS.md` (if/when created) without going through the plan workflow. The scaffold exists as a stable ground truth for the planning and review tooling.

## License

MIT (placeholder — see [LICENSE](LICENSE)). Final license selection pending author decision before public release.

## Citation

*Deterrence, Extended: A Spatial Multi-Agent Calibration of Civilizational Dynamics Against a Pre-Existing Kinetic Substrate* (forthcoming). See [docs/PAPER.md](docs/PAPER.md) for target venues (Complexity, Energy Policy, JCR).
