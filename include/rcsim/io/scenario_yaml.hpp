#pragma once

// §12: Scenario format (YAML). Observer policy defaults, telemetry access blocks,
// principal regime/clock, territory overrides, scripted actions.
// DESIGN_v1.3.md §12: Energy values in EJ/yr post-v1.2→v1.3 unit shift.

#include <string>

// Forward decl — WorldState authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::io {

// §12: Load scenario YAML into a fresh WorldState. Uses rapidyaml (ryml, §15).
// TODO(phase 2, §12): implement per DESIGN_v1.3.md §12
//   Parse: scenario_seed, observer policy defaults, telemetry_access blocks,
//          principals (regime/clock), territory_overrides (r, H, G, Qliq, Qelec, ...),
//          scripted_actions.
//   Validate: territory_overrides Qelec/Qliq in EJ/yr (flag if user left v1.2 J/yr values).
state::WorldState load_scenario(const std::string& yaml_path);

// §12: Write scenario YAML back (for migration, golden-master verification).
// TODO(phase 2, §12): implement
void save_scenario(const state::WorldState& s, const std::string& yaml_path);

// §12: Unit-shift migration helper — v1.2 J/yr → v1.3 EJ/yr per DESIGN_v1.3.md §2.2.
// TODO(phase 2, §12, §2.2): implement per DESIGN_v1.3.md §2.2 + MIGRATION doc.
void migrate_scenario_v1_2_to_v1_3(const std::string& in_path, const std::string& out_path);

}  // namespace rc::sim::io
