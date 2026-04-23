#pragma once

// §12: GeoJSON territory loader.
// DESIGN_v1.3.md §18 directory layout: geo/ { geojson, partition, adjacency }.cpp
// Loads territory polygon definitions used to partition the globe into N territories.

#include <string>
#include <vector>

#include "rcsim/state/territory.hpp"

namespace rc::sim::geo {

// §12, §18: TerritoryGeom — polygonal shape for a single territory.
// SPEC_AMBIGUOUS(§12): GeoJSON schema not pinned in spec. Using GeoJSON Feature with
//   Polygon/MultiPolygon geometry and "territory_id" property; refine during phase 2.
struct TerritoryGeom {
    state::TerritoryId id;
    // §12: ring of (lon, lat) pairs. Multi-polygon territories hold multiple rings.
    // TODO(phase 2, §12): populate with outer + holes per GeoJSON schema
    std::vector<double> coord_flat;   // [lon0, lat0, lon1, lat1, ...]
    std::vector<uint32_t> ring_offsets;
};

// §12: GeoLoadResult — aggregate of loaded territories.
struct GeoLoadResult {
    std::vector<TerritoryGeom> territories;
};

// §12: load_geojson — parse GeoJSON file into TerritoryGeom list.
// TODO(phase 2, §12): implement via nlohmann_json (§15 dependency list)
GeoLoadResult load_geojson(const std::string& path);

}  // namespace rc::sim::geo
