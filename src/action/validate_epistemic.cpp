#include "rcsim/action/validate_epistemic.hpp"

#include <variant>

#include "rcsim/observer/measurement.hpp"

// §5.4: validate_epistemic — checks against principal's observation, NOT ground truth.
//
// Closes Epistemic Oracle Leak: ground-truth validation is performed in
// apply_with_fizzle. Validation here uses ONLY `obs` and the action payload
// — it never touches WorldState. The §16.2 property test
// `epistemic_validation_no_leak` enforces this lexically (the function takes
// only `obs` and `a`; if WorldState ever leaks in, the signature breaks).
//
// Validation rules (Phase 2 baseline; refined in Phase 3 with regime gating):
//   - DurationOutOfRange:   action duration outside the spec-allowed range.
//   - InvalidParameters:    NaN/Inf in any numeric field.
//   - UnknownTarget:        target territory_id not in obs.territories_visible.
//   - InsufficientTreasury: validation of treasury cost requires PrincipalState
//                           — pushed to apply_with_fizzle's deduct path.
//   - InsufficientLegitimacy / DistortionLockout / PrincipalLockedOut: same.
//
// The pure-epistemic checks here are intentionally narrow — they cover only
// what the principal can decide from observation alone. Resource adequacy is
// handled at deduction time and surfaces as Fizzle if ground-truth diverges.

namespace rc::sim::action {

namespace {

bool is_finite(double v) noexcept {
    return v == v && (v - v) == 0.0;   // NaN / ±Inf rejection
}

ValidationReason validate_action(const observer::Observation& obs, const Action& a) noexcept {
    return std::visit([&](const auto& spec) noexcept -> ValidationReason {
        using T = std::decay_t<decltype(spec)>;

        if constexpr (std::is_same_v<T, ManipulateObservation>) {
            if (!is_finite(spec.investment_M) ||
                !is_finite(spec.investment_S) ||
                !is_finite(spec.distortion_magnitude)) {
                return ValidationReason::InvalidParameters;
            }
            if (spec.duration == 0) return ValidationReason::DurationOutOfRange;
            return ValidationReason::Ok;
        } else if constexpr (std::is_same_v<T, AttributeDistortion>) {
            if (!is_finite(spec.investment_M)) return ValidationReason::InvalidParameters;
            if (spec.duration == 0) return ValidationReason::DurationOutOfRange;
            return ValidationReason::Ok;
        } else if constexpr (std::is_same_v<T, SeizeTerritory> ||
                              std::is_same_v<T, Blockade> ||
                              std::is_same_v<T, ExportControl> ||
                              std::is_same_v<T, SovereignExpropriation>) {
            // Target must be in the actor's visible territory set.
            for (const auto& tv : obs.territories_visible) {
                if (tv.id == spec.target) return ValidationReason::Ok;
            }
            return ValidationReason::UnknownTarget;
        } else if constexpr (std::is_same_v<T, DatacenterBuild> ||
                              std::is_same_v<T, BuildInfrastructure>) {
            if (!is_finite(spec.investment_M)) return ValidationReason::InvalidParameters;
            for (const auto& tv : obs.territories_visible) {
                if (tv.id == spec.site) return ValidationReason::Ok;
            }
            return ValidationReason::UnknownTarget;
        } else if constexpr (std::is_same_v<T, CapitalAllocation> ||
                              std::is_same_v<T, MonetaryIssuance>) {
            return is_finite(spec.amount_M) ? ValidationReason::Ok
                                            : ValidationReason::InvalidParameters;
        } else if constexpr (std::is_same_v<T, AIDeploymentReallocation>) {
            if (!is_finite(spec.new_deployment_fraction_A)) return ValidationReason::InvalidParameters;
            if (spec.new_deployment_fraction_A < 0.0 || spec.new_deployment_fraction_A > 1.0) {
                return ValidationReason::InvalidParameters;
            }
            return ValidationReason::Ok;
        } else if constexpr (std::is_same_v<T, Sanction>) {
            (void)spec;
            return ValidationReason::Ok;
        } else if constexpr (std::is_same_v<T, Pass> ||
                              std::is_same_v<T, CurrencySwap> ||
                              std::is_same_v<T, VirtualizationPush> ||
                              std::is_same_v<T, MobilizeMilitary> ||
                              std::is_same_v<T, CeaseFire>) {
            return ValidationReason::Ok;
        } else {
            return ValidationReason::Ok;
        }
    }, a);
}

}  // namespace

ValidationResult validate_epistemic(
    const observer::Observation& obs,
    state::PrincipalId /*actor*/,
    const Action& a
) noexcept {
    ValidationReason r = validate_action(obs, a);
    return {r == ValidationReason::Ok, r};
}

}  // namespace rc::sim::action
