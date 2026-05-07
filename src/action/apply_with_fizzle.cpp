#include "rcsim/action/apply_with_fizzle.hpp"

#include <algorithm>
#include <utility>
#include <variant>

#include "rcsim/state/principal.hpp"
#include "rcsim/state/world_state.hpp"

// §5.4: Ground-truth application with Fizzle mechanic.
//
// §9.2b deduct_resources: unconditional on ground truth. Treasury and legitimacy
// are debited from the actor's PrincipalState. If actor's funds insufficient,
// the action will Fizzle at apply() — but resources are STILL consumed up to
// the available balance. This preserves the Fizzle information-cost: actor sees
// their resources gone with no effect, learns their observation was off.
//
// §9.2c apply: ground-truth feasibility check. Returns Fizzled with a typed
// FizzleReason when target state has changed since the principal's observation
// (per §5.4 closes the Epistemic Oracle Leak: validation passed because actor
// believed action valid, but ground truth has diverged).
//
// Phase 2 implements: SeizeTerritory, Blockade, DatacenterBuild, Pass.
// Other variants are wired as no-op apply (always Applied) and refined in
// Phase 3 alongside observer integration.

namespace rc::sim::action {

namespace {

state::TerritoryState* find_territory(state::WorldState& s, state::TerritoryId id) noexcept {
    for (auto& t : s.grid.states) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

}  // namespace

void deduct_resources(state::PrincipalState& p, const Action& a) noexcept {
    // §9.2b: unconditional debit. Floor at zero — investment_S above
    // legitimacy_S is truncated to available legitimacy (§6.2 S-floor).
    auto debit_M = [&](double m) noexcept {
        double take = m < 0.0 ? 0.0 : m;
        p.treasury_M = (p.treasury_M > take) ? (p.treasury_M - take) : 0.0;
    };
    auto debit_S = [&](double s) noexcept {
        double take = s < 0.0 ? 0.0 : s;
        p.legitimacy_S = (p.legitimacy_S > take) ? (p.legitimacy_S - take) : 0.0;
    };

    std::visit([&](const auto& spec) noexcept {
        using T = std::decay_t<decltype(spec)>;
        if constexpr (std::is_same_v<T, ManipulateObservation>) {
            debit_M(spec.investment_M);
            debit_S(spec.investment_S);
        } else if constexpr (std::is_same_v<T, AttributeDistortion>) {
            debit_M(spec.investment_M);
        } else if constexpr (std::is_same_v<T, DatacenterBuild> ||
                              std::is_same_v<T, BuildInfrastructure>) {
            debit_M(spec.investment_M);
        } else if constexpr (std::is_same_v<T, CapitalAllocation> ||
                              std::is_same_v<T, MonetaryIssuance>) {
            // Allocation moves M; issuance creates M (no debit).
            (void)spec;
        } else {
            (void)spec;
        }
    }, a);
}

ApplyResult apply(state::WorldState s, state::PrincipalId actor, const Action& a) noexcept {
    ApplyResult r;
    r.applied = false;
    r.fizzled = false;
    r.reason  = FizzleReason::None;

    // Dispatch on action variant.
    std::visit([&](const auto& spec) noexcept {
        using T = std::decay_t<decltype(spec)>;

        if constexpr (std::is_same_v<T, SeizeTerritory>) {
            auto* t = find_territory(s, spec.target);
            if (t == nullptr) {
                r.fizzled = true;
                r.reason  = FizzleReason::TargetAlreadyDestroyed;
                return;
            }
            // Ground-truth check: target may have been seized earlier this tick
            // by a higher-priority principal. If it's already actor's, that's a
            // no-op-Apply.
            if (t->owner == actor) {
                r.applied = true;
                return;
            }
            // Queue a TerritoryOwnerFlip pending effect; matures next tick per §5.2.
            state::PendingEffect pe;
            pe.activates_at = s.tick + 1;
            pe.expires_at   = s.tick + 1;
            pe.origin       = 0;     // Phase 4: caller fills ActionLogId
            pe.mutation     = state::TerritoryOwnerFlip{spec.target, actor};
            s.pending.push_back(pe);
            r.applied = true;
        } else if constexpr (std::is_same_v<T, Blockade>) {
            if (find_territory(s, spec.target) == nullptr) {
                r.fizzled = true;
                r.reason  = FizzleReason::TargetAlreadyDestroyed;
                return;
            }
            state::PendingEffect pe;
            pe.activates_at = s.tick + 1;
            pe.expires_at   = s.tick + 52;     // 1-yr default; calibrated in 7b
            pe.origin       = 0;
            pe.mutation     = state::BlockadeEdge{};
            s.pending.push_back(pe);
            r.applied = true;
        } else if constexpr (std::is_same_v<T, DatacenterBuild>) {
            if (find_territory(s, spec.site) == nullptr) {
                r.fizzled = true;
                r.reason  = FizzleReason::GeographyImpossible;
                return;
            }
            state::PendingEffect pe;
            pe.activates_at = s.tick + 1;
            pe.expires_at   = s.tick + 52 * 5;   // 5-yr ramp
            pe.origin       = 0;
            pe.mutation     = state::CapacityRamp{};
            s.pending.push_back(pe);
            r.applied = true;
        } else if constexpr (std::is_same_v<T, ManipulateObservation>) {
            // §6.2: queue DistortionBudget; actual distortion-on-observe lands Phase 3.
            state::PendingEffect pe;
            pe.activates_at = s.tick;
            pe.expires_at   = s.tick + spec.duration;
            pe.origin       = 0;
            pe.mutation     = state::DistortionBudget{};
            s.pending.push_back(pe);
            r.applied = true;
        } else if constexpr (std::is_same_v<T, AttributeDistortion>) {
            state::PendingEffect pe;
            pe.activates_at = s.tick;
            pe.expires_at   = s.tick + spec.duration;
            pe.origin       = 0;
            pe.mutation     = state::AttributionBudget{};
            s.pending.push_back(pe);
            r.applied = true;
        } else if constexpr (std::is_same_v<T, Pass>) {
            r.applied = true;
        } else {
            // Phase 3 implementations: CapitalAllocation, MonetaryIssuance,
            // CurrencySwap, VirtualizationPush, AIDeploymentReallocation,
            // SovereignExpropriation, MobilizeMilitary, CeaseFire, Sanction,
            // ExportControl, BuildInfrastructure. Until then they Apply with
            // no physical effect (resources already debited in 9.2b).
            (void)spec;
            r.applied = true;
        }
    }, a);

    r.state = std::move(s);
    return r;
}

}  // namespace rc::sim::action
