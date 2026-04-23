#include "rcsim/io/parquet_sink.hpp"

#include "rcsim/state/world_state.hpp"

// §11.2 Tier 1: territory-resolution Parquet, partitioned by (sweep_id, param_index).

namespace rc::sim::io {

ParquetSink::ParquetSink(const std::string& output_dir, uint32_t sweep_id, uint32_t param_index)
    : output_dir_(output_dir), sweep_id_(sweep_id), param_index_(param_index) {
    // TODO(phase 5, §11.2)
}

ParquetSink::~ParquetSink() = default;

void ParquetSink::write_tick(const state::WorldState& /*s*/) {
    // TODO(phase 5, §11.2)
}

void ParquetSink::close() {
    // TODO(phase 5, §11.2)
}

}  // namespace rc::sim::io
