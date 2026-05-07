#pragma once

// §12: GeoJSON territory loader.
// DESIGN_v1.3.md §18 directory layout: geo/ { geojson, partition, adjacency }.cpp
// Loads territory polygon definitions used to partition the globe into N territories.

#include <string>
#include <vector>

#include "rcsim/state/territory.hpp"

namespace rc::sim::geo {

// §12, §18: TerritoryGeom — polygonal shape for a single territory.
// R-17 schema: GeoJSON FeatureCollection per RFC 7946. Each Feature is a
// Polygon or MultiPolygon with required `properties.territory_id` (uint32).
// Coordinates are decimal degrees [longitude, latitude], snapped to 1e-7
// per R-19 on load.
struct TerritoryGeom {
    state::TerritoryId id;
    // Linear rings stored as flat (lon, lat) pairs. `ring_offsets` indexes
    // into `coord_flat`: ring k covers indices [ring_offsets[k], ring_offsets[k+1]).
    // Outer ring is ring 0 of each polygon; holes (if any) follow as additional rings.
    // For MultiPolygon, all rings of all polygons are concatenated; polygon
    // boundaries are recorded in `polygon_offsets` (indexes into ring_offsets).
    std::vector<double>   coord_flat;
    std::vector<uint32_t> ring_offsets;
    std::vector<uint32_t> polygon_offsets;
};

// §12: GeoLoadResult — aggregate of loaded territories.
struct GeoLoadResult {
    std::vector<TerritoryGeom> territories;
};

// §12: load_geojson — parse GeoJSON file into TerritoryGeom list.
// TODO(phase 2, §12): implement via nlohmann_json (§15 dependency list)
GeoLoadResult load_geojson(const std::string& path);

}  // namespace rc::sim::geo
