#pragma once

// §5.2: Pending-effect model. Actions enqueue PendingEffects that mature/retire
// at tick boundaries (§9 phases 1 and 8).
// §5.2 FizzleRecord (new v1.3): when epistemically valid but ground-truth infeasible;
// resources consumed, zero physical effect, logged for forensic diagnosis.

#include <cstdint>
#include <variant>

#include "rcsim/core/tick.hpp"
#include "rcsim/state/territory.hpp"

namespace rc::sim::state {

// §5.2: ActionLogId — identifier for a logged action.
// SPEC_AMBIGUOUS(§5.2): width not specified. Using uint64_t for generational safety.
using ActionLogId = uint64_t;

// §5.2: PendingEffect variant members. Each represents a distinct mutation category.
// TODO(phase 2, §5.2): flesh out fields per DESIGN_v1.3.md §5.2 variants listed below.

struct BlockadeEdge {
    // TODO(phase 2, §5.2): blockade-specific fields (src, dst, strength, etc.)
};

struct ExportControlFlow {
    // TODO(phase 2, §5.2): export-control fields
};

struct TerritoryOwnerFlip {
    TerritoryId territory;
    PrincipalId new_owner;
    // TODO(phase 2, §5.2): additional flip metadata
};

struct CapacityRamp {
    // TODO(phase 2, §5.2): capacity ramp fields
};

struct ShockImpulse {
    // TODO(phase 2, §5.2): shock impulse fields (amplitude, field, decay)
};

struct DistortionBudget {
    // TODO(phase 2, §5.2): distortion budget fields; populated by ManipulateObservation apply
};

struct AttributionBudget {
    // TODO(phase 2, §5.2): attribution budget fields; populated by AttributeDistortion apply
};

struct GateTransition {
    // TODO(phase 2, §5.2): gate transition fields
};

struct RegimeTransition {
    // §7.5: Democracy → FailedState, Autocracy → TransitionalState.
    PrincipalId principal;
    // TODO(phase 3, §7.5): new regime + transition metadata
};

// §5.2 (new v1.3): FizzleRecord — resources consumed, no physical effect.
struct FizzleRecord {
    ActionLogId origin;
    // TODO(phase 2, §5.2): add reason code enum for forensic diagnosis
};

// §5.2: PendingEffect aggregate.
struct PendingEffect {
    core::Tick activates_at;
    core::Tick expires_at;
    ActionLogId origin;
    std::variant<
        BlockadeEdge,
        ExportControlFlow,
        TerritoryOwnerFlip,
        CapacityRamp,
        ShockImpulse,
        DistortionBudget,
        AttributionBudget,
        GateTransition,
        RegimeTransition,
        FizzleRecord
    > mutation;
};

}  // namespace rc::sim::state
