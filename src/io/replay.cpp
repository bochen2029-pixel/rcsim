#include "rcsim/io/replay.hpp"

#include "rcsim/state/world_state.hpp"

// §10.2: Replay seeks to nearest preceding checkpoint; desync → halt.

namespace rc::sim::io {

ReplayDriver::ReplayDriver(const std::string& sync_log_path) : reader_(sync_log_path) {
    // TODO(phase 4, §10.2)
}

void ReplayDriver::replay(core::Tick /*tick_start*/, core::Tick /*tick_end*/, state::WorldState& /*out*/) {
    // TODO(phase 4, §10.2)
}

void ReplayDriver::replay_to_end(state::WorldState& /*out*/) {
    // TODO(phase 4, §10.2)
}

}  // namespace rc::sim::io
