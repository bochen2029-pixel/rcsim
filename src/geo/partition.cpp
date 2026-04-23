#include "rcsim/geo/partition.hpp"

// §18: Principal ownership CSV loader.

namespace rc::sim::geo {

std::vector<PartitionEntry> load_partition(const std::string& /*csv_path*/) {
    // TODO(phase 2, §18)
    return {};
}

void apply_partition(
    std::vector<PartitionEntry> /*entries*/,
    std::vector<state::TerritoryState>& /*territories*/
) {
    // TODO(phase 2, §18)
}

}  // namespace rc::sim::geo
