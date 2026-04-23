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
//   Shared-edge test uses exact coordinate equality (snapped to fixed-point grid during
//   GeoJSON load) to avoid floating-point near-miss ambiguity.
// SPEC_AMBIGUOUS(§18): coordinate snapping grid not pinned; default to 1e-9 deg snap
//   during phase 2. Revisit for correctness on dateline crossings.
// TODO(phase 2, §18): implement per DESIGN_v1.3.md §18
AdjacencyGraph derive_adjacency(const GeoLoadResult& geoms);

}  // namespace rc::sim::geo
