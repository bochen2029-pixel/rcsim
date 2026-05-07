#include "rcsim/io/scenario_yaml.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <ryml.hpp>
#include <ryml_std.hpp>

#include "rcsim/geo/adjacency.hpp"
#include "rcsim/geo/geojson.hpp"
#include "rcsim/state/world_state.hpp"

// §12: Scenario YAML loader.
//
// v1 schema (Phase 1 minimum — extended in Phase 2 for principals + scripted actions):
//
//   scenario_seed: <uint64>
//   ticks: <uint32>                    # default tick count (override with --ticks)
//   territories:
//     - id: <uint32>
//       owner: <uint32>
//       r: <double>                    # EROEI, dimensionless
//       H: <double>                    # employed cognitive labor
//       G: <double>                    # deployed AI compute (HE-units)
//       M: <double>                    # financial claims (USD-eq)
//       Npop: <double>
//       S: <double>
//       Qelec: <double>                # EJ/yr
//       Qliq: <double>                 # EJ/yr
//       D_local: <double>
//   global:
//     A: <double>                      # capability ratio (default 1.0)
//     S_AI_index: <double>             # default 1.0
//     D_global: <double>               # default 0.0
//
// EJ/yr validation: Qelec/Qliq > 1e15 likely indicates v1.2 J/yr leftover —
// emit a warning per §2.2.

namespace rc::sim::io {

namespace {

double read_double(const ryml::ConstNodeRef& node, double fallback) noexcept {
    if (node.invalid() || !node.has_val()) return fallback;
    double v = fallback;
    node >> v;
    return v;
}

uint32_t read_uint32(const ryml::ConstNodeRef& node, uint32_t fallback) noexcept {
    if (node.invalid() || !node.has_val()) return fallback;
    uint32_t v = fallback;
    node >> v;
    return v;
}

uint64_t read_uint64(const ryml::ConstNodeRef& node, uint64_t fallback) noexcept {
    if (node.invalid() || !node.has_val()) return fallback;
    uint64_t v = fallback;
    node >> v;
    return v;
}

void warn_if_v1_2_units(double q, const char* field, uint32_t tid) noexcept {
    if (q > 1.0e15) {
        std::fprintf(stderr,
            "rcsim §2.2 warning: territory_id=%u %s = %.3e looks like v1.2 J/yr; "
            "v1.3 expects EJ/yr (1 EJ = 1e18 J). Run rcsim-migrate-v1.2-to-v1.3.\n",
            tid, field, q);
    }
}

}  // namespace

state::WorldState load_scenario(const std::string& yaml_path) {
    std::ifstream in(yaml_path);
    if (!in) {
        throw std::runtime_error("rcsim: cannot open scenario file: " + yaml_path);
    }
    std::stringstream buf;
    buf << in.rdbuf();
    std::string text = buf.str();

    ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(text));
    ryml::ConstNodeRef root = tree.rootref();

    state::WorldState s{};
    s.tick = 0;
    s.time_years = 0.0;
    s.global.D_global   = 0.0;
    s.global.A          = 1.0;
    s.global.S_AI_index = 1.0;

    s.global.seed_scenario = read_uint64(root["scenario_seed"], 0);

    // Optional companion GeoJSON for adjacency. Path is resolved relative to
    // the scenario file's directory if not absolute.
    if (root.has_child("geojson") && root["geojson"].has_val()) {
        std::string geojson_path;
        root["geojson"] >> geojson_path;
        if (!geojson_path.empty() && geojson_path.front() != '/') {
            auto slash = yaml_path.find_last_of('/');
            if (slash != std::string::npos) {
                geojson_path = yaml_path.substr(0, slash + 1) + geojson_path;
            }
        }
        auto geoms = geo::load_geojson(geojson_path);
        s.adjacency = geo::derive_adjacency(geoms);
    }

    if (root.has_child("global")) {
        auto g = root["global"];
        s.global.A           = read_double(g["A"],          1.0);
        s.global.S_AI_index  = read_double(g["S_AI_index"], 1.0);
        s.global.D_global    = read_double(g["D_global"],   0.0);
    }

    if (root.has_child("territories")) {
        auto terr_list = root["territories"];
        for (auto t_node : terr_list.children()) {
            state::TerritoryState t{};
            t.id      = read_uint32(t_node["id"],    0);
            t.owner   = read_uint32(t_node["owner"], 0);
            t.r       = read_double(t_node["r"],       1.0);
            t.H       = read_double(t_node["H"],       0.0);
            t.G       = read_double(t_node["G"],       0.0);
            t.M       = read_double(t_node["M"],       0.0);
            t.Npop    = read_double(t_node["Npop"],    0.0);
            t.S       = read_double(t_node["S"],       0.0);
            t.Qelec   = read_double(t_node["Qelec"],   0.0);
            t.Qliq    = read_double(t_node["Qliq"],    0.0);
            t.D_local = read_double(t_node["D_local"], 0.0);
            t.eta     = 0.0;
            t.phi     = 0.0;
            warn_if_v1_2_units(t.Qelec, "Qelec", t.id);
            warn_if_v1_2_units(t.Qliq,  "Qliq",  t.id);
            s.grid.states.push_back(t);
        }
    }

    // Initialize Kahan sidecar to match grid size.
    const std::size_t N = s.grid.states.size();
    s.canon.residual_r.assign(N, 0.0);
    s.canon.residual_H.assign(N, 0.0);
    s.canon.residual_G.assign(N, 0.0);
    s.canon.residual_M.assign(N, 0.0);
    s.canon.residual_Npop.assign(N, 0.0);
    s.canon.residual_S.assign(N, 0.0);
    s.canon.residual_Qelec.assign(N, 0.0);
    s.canon.residual_Qliq.assign(N, 0.0);
    s.canon.residual_D_local.assign(N, 0.0);

    return s;
}

void save_scenario(const state::WorldState& /*s*/, const std::string& /*yaml_path*/) {
    // Phase 2: emit canonical YAML for golden-master comparison and v1.2→v1.3 migration.
}

void migrate_scenario_v1_2_to_v1_3(const std::string& /*in_path*/, const std::string& /*out_path*/) {
    // Phase 2: divide Qelec/Qliq by 1e18 (J/yr → EJ/yr) per §2.2.
}

}  // namespace rc::sim::io
