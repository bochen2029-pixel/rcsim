#pragma once

// §5.1: Action variant and variant-struct declarations.
// §6.2: ManipulateObservation — first-class distortion action.
// §6.4: AttributeDistortion — probabilistic attribution action.

#include <cstdint>
#include <string>
#include <variant>

#include "rcsim/core/tick.hpp"
#include "rcsim/state/territory.hpp"

// Forward declarations — observer types authored in Task 7.
namespace rc::sim::observer {
enum class ObservationField : uint32_t;
}

namespace rc::sim::action {

// §5.1: Action variants. Each struct carries action-specific parameters.
// TODO(phase 2, §5.1): flesh out fields per DESIGN_v1.3.md §5 prose as implementation proceeds.

struct SeizeTerritory {
    state::TerritoryId target;
    // TODO(phase 2, §5.1): seizure-specific fields
};

struct Blockade {
    state::TerritoryId target;
    // TODO(phase 2, §5.1): blockade-specific fields
};

struct ExportControl {
    state::TerritoryId target;
    // TODO(phase 2, §5.1): export-control fields
};

struct DatacenterBuild {
    state::TerritoryId site;
    double investment_M;
    // TODO(phase 2, §5.1): additional fields (capacity, ramp duration)
};

struct BuildInfrastructure {
    state::TerritoryId site;
    double investment_M;
    // TODO(phase 2, §5.1): infrastructure-type enum
};

struct CapitalAllocation {
    double amount_M;
    // TODO(phase 2, §5.1): allocation-target fields
};

struct MonetaryIssuance {
    double amount_M;
    // TODO(phase 2, §5.1): issuance-regime metadata
};

struct CurrencySwap {
    // TODO(phase 2, §5.1): counterparty, amount fields
};

struct VirtualizationPush {
    // TODO(phase 2, §5.1): virtualization fields
};

struct AIDeploymentReallocation {
    double new_deployment_fraction_A;
    // TODO(phase 2, §5.1)
};

struct SovereignExpropriation {
    state::TerritoryId target;
    // TODO(phase 2, §5.1)
};

struct MobilizeMilitary {
    // TODO(phase 2, §5.1): mobilization fields (scale, kinetic-amplitude feeding Γ_kin)
};

struct CeaseFire {
    // TODO(phase 2, §5.1): counterparty fields
};

struct Sanction {
    state::PrincipalId target;
    // TODO(phase 2, §5.1): sanction-type fields
};

// §6.2: ManipulateObservation — treasury + legitimacy cost.
struct ManipulateObservation {
    state::PrincipalId target_observer;
    observer::ObservationField field;
    double investment_M;              // treasury cost (deducted on apply)
    double investment_S;              // legitimacy cost (deducted on apply, floored to available)
    double distortion_magnitude;
    core::Tick duration;
};

// §6.4: AttributeDistortion — probabilistic evaluation via seeded RNG.
struct AttributeDistortion {
    state::PrincipalId accused;
    observer::ObservationField field;
    double investment_M;              // forensics cost
    core::Tick duration;
};

struct Pass {
    // §5.3: losing actor degrades to Pass with reason code.
    // TODO(phase 2, §5.3): reason code enum
};

// §5.1: Action variant.
using Action = std::variant<
    SeizeTerritory,
    Blockade,
    ExportControl,
    DatacenterBuild,
    BuildInfrastructure,
    CapitalAllocation,
    MonetaryIssuance,
    CurrencySwap,
    VirtualizationPush,
    AIDeploymentReallocation,
    SovereignExpropriation,
    MobilizeMilitary,
    CeaseFire,
    Sanction,
    ManipulateObservation,
    AttributeDistortion,
    Pass
>;

}  // namespace rc::sim::action
