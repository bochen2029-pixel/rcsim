#include "rcsim/geo/geojson.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

// §12 / §18 / R-17 / R-19: GeoJSON FeatureCollection loader.
//
// Schema (R-17):
//   {
//     "type": "FeatureCollection",
//     "features": [
//       { "type": "Feature",
//         "geometry": {"type": "Polygon"|"MultiPolygon", "coordinates": ...},
//         "properties": {"territory_id": <uint32>, "name": "optional"}
//       }, ...
//     ]
//   }
//
// Coordinate snapping (R-19): all lon/lat values rounded to 1e-7 degrees on
// ingest so the shared-edge adjacency test can use exact double equality.

namespace rc::sim::geo {

namespace {

constexpr double kSnapGrid = 1.0e-7;

inline double snap(double v) noexcept {
    return std::round(v / kSnapGrid) * kSnapGrid;
}

void append_ring(const nlohmann::json& ring, TerritoryGeom& tg) {
    tg.ring_offsets.push_back(static_cast<uint32_t>(tg.coord_flat.size() / 2));
    for (const auto& pt : ring) {
        if (!pt.is_array() || pt.size() < 2) {
            throw std::runtime_error("rcsim §12: malformed coordinate point in GeoJSON");
        }
        tg.coord_flat.push_back(snap(pt[0].get<double>()));    // longitude
        tg.coord_flat.push_back(snap(pt[1].get<double>()));    // latitude
    }
}

void append_polygon(const nlohmann::json& polygon, TerritoryGeom& tg) {
    tg.polygon_offsets.push_back(static_cast<uint32_t>(tg.ring_offsets.size()));
    if (!polygon.is_array()) {
        throw std::runtime_error("rcsim §12: polygon coordinates must be an array of rings");
    }
    for (const auto& ring : polygon) {
        append_ring(ring, tg);
    }
}

TerritoryGeom load_feature(const nlohmann::json& feature) {
    TerritoryGeom tg{};
    if (!feature.contains("properties") || !feature["properties"].contains("territory_id")) {
        throw std::runtime_error(
            "rcsim §12: every Feature must have properties.territory_id (R-17)");
    }
    tg.id = feature["properties"]["territory_id"].get<state::TerritoryId>();

    if (!feature.contains("geometry")) {
        throw std::runtime_error("rcsim §12: Feature missing geometry");
    }
    const auto& g = feature["geometry"];
    const std::string type = g.value("type", std::string{});

    if (type == "Polygon") {
        append_polygon(g.at("coordinates"), tg);
    } else if (type == "MultiPolygon") {
        for (const auto& poly : g.at("coordinates")) {
            append_polygon(poly, tg);
        }
    } else {
        throw std::runtime_error(
            "rcsim §12: unsupported geometry type '" + type +
            "' (R-17 accepts Polygon and MultiPolygon only)");
    }

    // Sentinels: ring k spans [ring_offsets[k], ring_offsets[k+1]); polygon m
    // spans [polygon_offsets[m], polygon_offsets[m+1]) in the ring index space.
    tg.ring_offsets.push_back(static_cast<uint32_t>(tg.coord_flat.size() / 2));
    tg.polygon_offsets.push_back(static_cast<uint32_t>(tg.ring_offsets.size() - 1));

    return tg;
}

}  // namespace

GeoLoadResult load_geojson(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("rcsim §12: cannot open GeoJSON file: " + path);
    }
    nlohmann::json doc;
    in >> doc;

    const std::string type = doc.value("type", std::string{});
    if (type != "FeatureCollection") {
        throw std::runtime_error(
            "rcsim §12: top-level type must be 'FeatureCollection' (got '" + type + "')");
    }

    GeoLoadResult result;
    if (!doc.contains("features") || !doc["features"].is_array()) {
        throw std::runtime_error("rcsim §12: FeatureCollection missing 'features' array");
    }
    result.territories.reserve(doc["features"].size());
    for (const auto& f : doc["features"]) {
        result.territories.push_back(load_feature(f));
    }
    return result;
}

}  // namespace rc::sim::geo
