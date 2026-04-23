#include "rcsim/io/parquet_rollup.hpp"

#include "rcsim/state/world_state.hpp"

// §11.2 Tier 2: principal-aggregated Parquet via tree-reduce over owned territories.

namespace rc::sim::io {

ParquetRollup::ParquetRollup(const std::string& output_dir, uint32_t sweep_id, uint32_t param_index)
    : output_dir_(output_dir), sweep_id_(sweep_id), param_index_(param_index) {
    // TODO(phase 5, §11.2)
}

ParquetRollup::~ParquetRollup() = default;

void ParquetRollup::write_tick(const state::WorldState& /*s*/) {
    // TODO(phase 5, §11.2, §2.4)
}

void ParquetRollup::close() {
    // TODO(phase 5, §11.2)
}

}  // namespace rc::sim::io
