#include "rcsim/geo/adjacency.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <vector>

// §18 / R-19: AdjacencyGraph derivation from snapped GeoJSON polygon edges.
//
// Algorithm:
//   1. For every (territory, polygon, ring), enumerate consecutive edges.
//   2. Canonicalize each edge as the pair of endpoints sorted lexicographically.
//   3. Build a map: edge_key -> set of territory_ids that touch it.
//   4. For each edge with >= 2 distinct territory_ids, emit one AdjacencyEdge
//      per (a, b) pair (a < b). Deduplicate.
//   5. Sort edges lexicographically per §2.4 determinism contract.
//
// edge_key is a 128-bit packed (lon_a, lat_a, lon_b, lat_b) using bit_cast on
// the snapped doubles — exact equality under strict IEEE-754. We avoid hashing
// floating-point bits directly by going through int64 reinterpretation; the
// snap grid (R-19) guarantees no NaN/-0.0 sneaks in.

namespace rc::sim::geo {

namespace {

struct EdgeKey {
    int64_t a_lon, a_lat, b_lon, b_lat;
    bool operator==(const EdgeKey& o) const noexcept {
        return a_lon == o.a_lon && a_lat == o.a_lat &&
               b_lon == o.b_lon && b_lat == o.b_lat;
    }
};

struct EdgeKeyHash {
    std::size_t operator()(const EdgeKey& k) const noexcept {
        // Mixing as in FNV-1a-like; not cryptographic, just collision-resistant
        // enough for the thousands-of-edges scale Phase 2 sees.
        uint64_t h = 1469598103934665603ULL;
        auto mix = [&](int64_t v) noexcept {
            h ^= static_cast<uint64_t>(v);
            h *= 1099511628211ULL;
        };
        mix(k.a_lon); mix(k.a_lat); mix(k.b_lon); mix(k.b_lat);
        return static_cast<std::size_t>(h);
    }
};

inline int64_t bit_int(double v) noexcept {
    int64_t out;
    std::memcpy(&out, &v, sizeof(out));
    return out;
}

// Canonicalize an edge: endpoint with smaller (lon, lat) goes first.
EdgeKey make_key(double lon0, double lat0, double lon1, double lat1) noexcept {
    EdgeKey k;
    bool first_smaller =
        (lon0 < lon1) || (lon0 == lon1 && lat0 <= lat1);
    if (first_smaller) {
        k.a_lon = bit_int(lon0); k.a_lat = bit_int(lat0);
        k.b_lon = bit_int(lon1); k.b_lat = bit_int(lat1);
    } else {
        k.a_lon = bit_int(lon1); k.a_lat = bit_int(lat1);
        k.b_lon = bit_int(lon0); k.b_lat = bit_int(lat0);
    }
    return k;
}

}  // namespace

AdjacencyGraph derive_adjacency(const GeoLoadResult& geoms) {
    // Step 1+2+3: build edge_key -> set of territory_ids.
    // Use std::vector<TerritoryId> (deduplicated on insert) to keep ordering
    // deterministic; std::unordered_set iteration is non-deterministic and
    // we need stable output per §2.4.
    std::unordered_map<EdgeKey, std::vector<state::TerritoryId>, EdgeKeyHash> edge_owners;

    for (const auto& tg : geoms.territories) {
        // Iterate every ring's consecutive edges. Note: ring_offsets has a
        // sentinel at the end (per geojson.cpp), so ring k spans
        // [ring_offsets[k], ring_offsets[k+1]) point indices.
        if (tg.ring_offsets.size() < 2) continue;
        for (std::size_t r = 0; r + 1 < tg.ring_offsets.size(); ++r) {
            uint32_t start = tg.ring_offsets[r];
            uint32_t end   = tg.ring_offsets[r + 1];
            if (end <= start + 1) continue;
            for (uint32_t i = start; i + 1 < end; ++i) {
                double lon0 = tg.coord_flat[2 * i + 0];
                double lat0 = tg.coord_flat[2 * i + 1];
                double lon1 = tg.coord_flat[2 * (i + 1) + 0];
                double lat1 = tg.coord_flat[2 * (i + 1) + 1];
                EdgeKey k = make_key(lon0, lat0, lon1, lat1);
                auto& vec = edge_owners[k];
                if (std::find(vec.begin(), vec.end(), tg.id) == vec.end()) {
                    vec.push_back(tg.id);
                }
            }
        }
    }

    // Step 4: build the unique adjacency-edge set in (min, max) form.
    std::vector<AdjacencyEdge> edges;
    {
        // Use a set keyed on (min, max) pair to dedupe across multiple shared edges.
        std::vector<std::pair<state::TerritoryId, state::TerritoryId>> pairs;
        pairs.reserve(edge_owners.size());
        for (const auto& [_, owners] : edge_owners) {
            if (owners.size() < 2) continue;
            // For each (a, b) pair from owners, add (min, max).
            for (std::size_t i = 0; i < owners.size(); ++i) {
                for (std::size_t j = i + 1; j < owners.size(); ++j) {
                    auto a = owners[i], b = owners[j];
                    if (a == b) continue;
                    if (a > b) std::swap(a, b);
                    pairs.emplace_back(a, b);
                }
            }
        }
        std::sort(pairs.begin(), pairs.end());
        pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());

        edges.reserve(pairs.size());
        for (const auto& [a, b] : pairs) {
            edges.push_back(AdjacencyEdge{a, b});
        }
    }

    // Step 5: edges are already sorted lex by (min, max).
    AdjacencyGraph g;
    g.edges = std::move(edges);

    // Per-territory neighbor lists. Indexed by 0..max_id; size = max_id + 1.
    state::TerritoryId max_id = 0;
    for (const auto& tg : geoms.territories) {
        if (tg.id > max_id) max_id = tg.id;
    }
    g.neighbors.assign(static_cast<std::size_t>(max_id) + 1, {});
    for (const auto& e : g.edges) {
        g.neighbors[e.a].push_back(e.b);
        g.neighbors[e.b].push_back(e.a);
    }
    // Sort each neighbor list for canonical ordering.
    for (auto& nb : g.neighbors) {
        std::sort(nb.begin(), nb.end());
    }

    return g;
}

}  // namespace rc::sim::geo
