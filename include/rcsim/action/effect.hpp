#pragma once

// §5.2: Pending-effect model and FizzleRecord.
// This header surfaces the state-layer PendingEffect types for action-layer consumers
// (apply_with_fizzle reads/writes PendingEffects).

#include "rcsim/state/pending.hpp"

namespace rc::sim::state { struct WorldState; }

namespace rc::sim::action {

// Re-export state-layer effect types into the action namespace for convenience.
using state::PendingEffect;
using state::FizzleRecord;
using state::BlockadeEdge;
using state::ExportControlFlow;
using state::TerritoryOwnerFlip;
using state::CapacityRamp;
using state::ShockImpulse;
using state::DistortionBudget;
using state::AttributionBudget;
using state::GateTransition;
using state::RegimeTransition;

// §9.1 / §5.2: walk WorldState::pending in canonical order; apply mutations
// for any effect whose activates_at == current tick.
void mature_pending(state::WorldState& s) noexcept;

// §9.8: erase pending effects whose expires_at < current tick.
void retire_expired(state::WorldState& s) noexcept;

}  // namespace rc::sim::action
