#pragma once

// §10.1: Sync log binary format.
// DESIGN_v1.3.md §10.1: Blake3-256 hash chain + checkpoint + delta + CRCs.
// §9.10: advance loop emits sync log write per tick.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rcsim/core/hash.hpp"
#include "rcsim/core/tick.hpp"

// Forward decl — WorldState is authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::io {

// §10.1: SyncLog abstract interface.
class SyncLog {
public:
    virtual ~SyncLog() = default;

    // §9.10: write_tick — append a tick record (state + hash) to the log.
    // TODO(phase 4, §10.1): implement per DESIGN_v1.3.md §10.1
    virtual void write_tick(const state::WorldState& s, const core::StateHash& hash) = 0;

    // §10.1: flush any buffered writes.
    virtual void flush() = 0;
};

// §10.1: BinarySyncLog — concrete implementation.
//   - Header: schema_major/minor, scenario_seed, magic bytes.
//   - Checkpoints: every K ticks (K configurable).
//   - Delta frames: compressed state delta between checkpoints.
//   - CRC per frame; Blake3 hash-chain across frames.
class BinarySyncLog : public SyncLog {
public:
    // TODO(phase 4, §10.1): implement constructor, write_tick, flush.
    explicit BinarySyncLog(const std::string& path);
    ~BinarySyncLog() override;

    void write_tick(const state::WorldState& s, const core::StateHash& hash) override;
    void flush() override;

private:
    // SPEC_AMBIGUOUS(§10.1): checkpoint cadence K not pinned.
    //   Default suggested: every 100 ticks; revisit during phase 4.
    std::string path_;
};

// §10.1, §10.2: SyncLogReader — seek to checkpoint, decompress, apply deltas.
class SyncLogReader {
public:
    // TODO(phase 4, §10.1, §10.2): implement per DESIGN_v1.3.md §10.1-§10.2
    explicit SyncLogReader(const std::string& path);

    // §10.2: seek to tick; replay engine calls to find preceding checkpoint.
    core::StateHash hash_at(core::Tick t);

    // §10.2: read single tick record (state + hash).
    // TODO(phase 4, §10.2): populate WorldState by value/out-param.
    void read_tick(core::Tick t, state::WorldState& out);
};

}  // namespace rc::sim::io
