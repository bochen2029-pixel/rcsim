#include "rcsim/geo/adjacency.hpp"

// §18: Adjacency graph derived from shared polygon edges (snapped coords).

namespace rc::sim::geo {

AdjacencyGraph derive_adjacency(const GeoLoadResult& /*geoms*/) {
    // TODO(phase 2, §18): shared-edge test with fixed-point coordinate snap.
    return {};
}

}  // namespace rc::sim::geo
