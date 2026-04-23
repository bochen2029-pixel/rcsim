#include "rcsim/io/scenario_yaml.hpp"

#include "rcsim/state/world_state.hpp"

// §12: Scenario YAML loader / migrator via rapidyaml (ryml, §15).

namespace rc::sim::io {

state::WorldState load_scenario(const std::string& /*yaml_path*/) {
    // TODO(phase 2, §12)
    return state::WorldState{};
}

void save_scenario(const state::WorldState& /*s*/, const std::string& /*yaml_path*/) {
    // TODO(phase 2, §12)
}

void migrate_scenario_v1_2_to_v1_3(const std::string& /*in_path*/, const std::string& /*out_path*/) {
    // TODO(phase 2, §12, §2.2): divide Qelec/Qliq by 1e18 (J/yr → EJ/yr).
}

}  // namespace rc::sim::io
