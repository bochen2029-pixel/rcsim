#pragma once

// §11.2 Tier 2: sweep_rollup Parquet writer — principal-aggregated via tree-reduce.
// DESIGN_v1.3.md §11.2: ~16 million rows for 10k × 15yr. Serves 90% of analysis.

#include <string>

#include "rcsim/core/tick.hpp"

// Forward decl — WorldState authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::io {

// §11.2 Tier 2: ParquetRollup writes per-tick per-principal aggregated rows.
// Aggregation uses core::deterministic_tree_reduce (§2.4) over principal-owned territories.
class ParquetRollup {
public:
    // TODO(phase 5, §11.2): implement constructor + close.
    ParquetRollup(const std::string& output_dir, uint32_t sweep_id, uint32_t param_index);
    ~ParquetRollup();

    // §11.2: write a tick of principal-aggregated data (uses tree-reduce per §2.4).
    // TODO(phase 5, §11.2, §2.4): implement per DESIGN_v1.3.md §11.2
    void write_tick(const state::WorldState& s);

    // §11.2: flush + close; call before shutdown.
    // TODO(phase 5, §11.2): implement
    void close();

private:
    std::string output_dir_;
    uint32_t sweep_id_;
    uint32_t param_index_;
};

}  // namespace rc::sim::io
