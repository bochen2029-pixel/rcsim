#pragma once

// §18: geo/partition.cpp — principal ownership CSV loader.
// SPEC_AMBIGUOUS(§18): spec lists partition.cpp in the directory layout but does not
//   specify CSV schema. Using (territory_id, principal_id) as the minimal schema;
//   refine during phase 2 when GeoJSON + partition wire up together.

#include <string>
#include <vector>

#include "rcsim/state/territory.hpp"

namespace rc::sim::geo {

// §18: Partition entry — pairs a territory to its owning principal.
struct PartitionEntry {
    state::TerritoryId territory;
    state::PrincipalId owner;
};

// §18: load_partition — parse (territory_id, principal_id) CSV.
// TODO(phase 2, §18): implement per DESIGN_v1.3.md §18
std::vector<PartitionEntry> load_partition(const std::string& csv_path);

// §18: apply_partition — set TerritoryState.owner per entries.
// TODO(phase 2, §18): implement
void apply_partition(
    std::vector<PartitionEntry> entries,
    std::vector<state::TerritoryState>& territories
);

}  // namespace rc::sim::geo
