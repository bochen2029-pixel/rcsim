#pragma once

// §11.2 Tier 1: sweep_full Parquet writer — territory-resolution.
// DESIGN_v1.3.md §11.2: Partitioned by (sweep_id, param_index). ~1.17B rows for 10k × 15yr.
// 15-25 GB compressed.

#include <string>
#include <vector>

#include "rcsim/core/tick.hpp"
#include "rcsim/state/territory.hpp"

// Forward decl — WorldState authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::io {

// §11.2 Tier 1: ParquetSink writes per-tick per-territory rows.
class ParquetSink {
public:
    // TODO(phase 5, §11.2): implement constructor + close.
    ParquetSink(const std::string& output_dir, uint32_t sweep_id, uint32_t param_index);
    ~ParquetSink();

    // §11.2: write a single tick of territory-resolution data.
    // TODO(phase 5, §11.2): implement per DESIGN_v1.3.md §11.2
    void write_tick(const state::WorldState& s);

    // §11.2: flush + close current row group; call before shutdown.
    // TODO(phase 5, §11.2): implement
    void close();

private:
    std::string output_dir_;
    uint32_t sweep_id_;
    uint32_t param_index_;
};

}  // namespace rc::sim::io
