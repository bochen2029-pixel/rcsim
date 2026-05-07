#pragma once

// §3.5, §18: AdjacencyGraph — derived from shared polygon edges between territories.
// Used by coupling (§4.6) for migration/trade flow edge list.

#include <cstdint>
#include <vector>

#include "rcsim/geo/geojson.hpp"
#include "rcsim/state/territory.hpp"

namespace rc::sim::geo {

// §18: AdjacencyGraph — undirected territory adjacency.
//   Edges sorted lex by (min(a,b), max(a,b)) for determinism (§2.4).
struct AdjacencyEdge {
    state::TerritoryId a;
    state::TerritoryId b;
};

struct AdjacencyGraph {
    // §2.4: edges stored lexicographically sorted.
    std::vector<AdjacencyEdge> edges;

    // Per-territory neighbor lists, parallel to TerritoryGrid indexing.
    std::vector<std::vector<state::TerritoryId>> neighbors;
};

// §18: derive_adjacency — compute adjacency graph from GeoJSON polygons via shared edges.
// R-19 pins the snap grid at 1e-7 deg (~1.1 cm at equator); coordinates are snapped on
// GeoJSON load (geo/geojson.cpp). Shared-edge test uses exact double-equality on snapped
// coordinates — integer round-trip is bit-exact under strict IEEE-754.
// Dateline crossing (lon ≈ ±180°) is not handled in v1; territories near the dateline
// must be authored to use a consistent sign convention or the adjacency will miss them.
AdjacencyGraph derive_adjacency(const GeoLoadResult& geoms);

}  // namespace rc::sim::geo
