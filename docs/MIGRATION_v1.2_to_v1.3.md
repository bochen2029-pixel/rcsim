# MIGRATION_v1.2_to_v1.3.md — unit shift and breaking changes

*Stub. Populated when `rcsim-migrate-v1.2-to-v1.3` utility is implemented.*

## Summary

v1.3 introduces eight critical-bug fixes over v1.2 (see DESIGN_v1.3.md §20 amendments).
The most externally-visible change is the **energy unit shift from J/yr to EJ/yr**
per DESIGN_v1.3.md §2.2, which requires migrating existing scenario YAML files.

## Breaking changes

### Unit shift (§2.2)

- `Q_elec`, `Q_liq` fields: J/yr → EJ/yr (divide by 10^18)
- Q-format storage unchanged in layout, but semantic interpretation shifts
- Conversion factors in §4.3 rescaled
- Γ_kinetic amplitudes in §4.4 rescaled (decay rates unit-invariant)

### API changes

- `validate_action(WorldState, ...)` → `validate_epistemic(Observation, ...)` + `apply()` with Fizzle (§5.4)
- Coupling flow solver: FP LP → Integer Network Flow (§4.6)
- Integrator: dynamic-dt bisection removed; fixed-step RK4 with gates held constant (§8.2)
- Attribution: deterministic threshold → probabilistic via seeded RNG (§6.4)

### State layout

- v1.2 had both AoS per-territory and SoA grid; v1.3 is AoS-only (§3.1-§3.2)
- `CanonicalizationState` now includes Kahan residual sidecar (§2.7)
- Signed-zero canonicalization applied pre-hash (§2.6)

## Migration utility

`rcsim-migrate-v1.2-to-v1.3` will handle:
- Scenario YAML energy-field rescaling (J/yr → EJ/yr)
- Sync log schema_major bump + replay-forward-only compatibility
- Action wire format enum-value freezing check

*See DESIGN_v1.3.md §12 (scenario format), §10.1a (action wire format), §20 (full list of v1.2 → v1.3 amendments).*
