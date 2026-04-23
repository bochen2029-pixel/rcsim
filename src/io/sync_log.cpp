#include "rcsim/io/sync_log.hpp"

#include "rcsim/state/world_state.hpp"

// §10.1: Binary sync log with Blake3 hash chain + checkpoint + delta + CRC.

namespace rc::sim::io {

BinarySyncLog::BinarySyncLog(const std::string& path) : path_(path) {
    // TODO(phase 4, §10.1)
}

BinarySyncLog::~BinarySyncLog() = default;

void BinarySyncLog::write_tick(const state::WorldState& /*s*/, const core::StateHash& /*hash*/) {
    // TODO(phase 4, §10.1)
}

void BinarySyncLog::flush() {
    // TODO(phase 4, §10.1)
}

SyncLogReader::SyncLogReader(const std::string& /*path*/) {
    // TODO(phase 4, §10.1, §10.2)
}

core::StateHash SyncLogReader::hash_at(core::Tick /*t*/) {
    // TODO(phase 4, §10.2)
    return {};
}

void SyncLogReader::read_tick(core::Tick /*t*/, state::WorldState& /*out*/) {
    // TODO(phase 4, §10.2)
}

}  // namespace rc::sim::io
