# CALIBRATION.md — rcsim calibration workstream

*Stub. Populated during phase 7a and 7b of the implementation plan.*

## Scope

This document will describe the calibration methodology for rcsim against historical
observables (1990-2025). See DESIGN_v1.3.md §19.1 for phase 7a/7b schedule and
deliverables.

## Phase 7a (12 weeks) — Preliminary calibration

IMM-RBPF (Interacting Multiple Model Rao-Blackwellized Particle Filter) v0.1 posteriors
against 1990-2025 historical observables. 10,000+ particles, careful prior specification,
particle-degeneracy debugging. Produces defensible v0.1 posteriors. Paper drafting begins.

## Phase 7b (16-24 weeks) — Publication-grade calibration

- Per-regime response-time gamma-fits from historical response data
- Observer detection-efficacy estimation from public-record distortion-detection timelines
- η conversion-efficiency refinement from EIA balances
- Probabilistic attribution rate calibration

## Deliverables

- Posterior distributions over ParamBlock fields
- Calibration confidence metrics per variable
- Retrodiction RMSE against §16.3 historical events
- Feeds into v1.1 spec revisions and v6 framework extension

*See DESIGN_v1.3.md §16.3 (retrodiction suite), §19 (phase plan), §19.1 (calibration cost honesty).*
