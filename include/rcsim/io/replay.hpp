#pragma once

// §10.2: Replay semantics.
// DESIGN_v1.3.md §10.2: ReplayDriver seeks to nearest preceding checkpoint, decompresses,
// applies delta frames tick-by-tick, recomputes hash via deterministic_tree_reduce +
// canonicalize_for_hash, compares to stored. Mismatch = desync = halt.

#include <string>

#include "rcsim/core/hash.hpp"
#include "rcsim/core/tick.hpp"
#include "rcsim/io/sync_log.hpp"

// Forward decl — WorldState is authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::io {

// §10.2: ReplayDriver — deterministic replay from sync log.
class ReplayDriver {
public:
    // TODO(phase 4, §10.2): implement constructor.
    explicit ReplayDriver(const std::string& sync_log_path);

    // §10.2: replay from tick_start to tick_end, verifying hashes.
    // Halts on mismatch.
    // TODO(phase 4, §10.2): implement per DESIGN_v1.3.md §10.2
    void replay(core::Tick tick_start, core::Tick tick_end, state::WorldState& out);

    // §10.2: replay-to-end convenience.
    // TODO(phase 4, §10.2): implement
    void replay_to_end(state::WorldState& out);

private:
    SyncLogReader reader_;
};

}  // namespace rc::sim::io
