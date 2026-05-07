#include "rcsim/action/effect.hpp"

#include <algorithm>
#include <variant>

#include "rcsim/state/world_state.hpp"

// §5.2 / §9.1 / §9.8: pending-effect maturation and retirement.
//
// §9.1 Phase 1 of advance loop: mature any PendingEffect whose activates_at == s.tick;
//   mutate WorldState according to the variant.
// §9.8 Phase 8: retire any PendingEffect whose expires_at < s.tick.
//
// Phase 2 implements a partial set: TerritoryOwnerFlip (active mutation),
// FizzleRecord (no-op for ground truth, just retained for log), and the
// remaining variants as inert pass-throughs (their mutations land in Phase 3).

namespace rc::sim::action {

namespace {

void apply_mutation(state::WorldState& s, const state::PendingEffect& pe) noexcept {
    std::visit([&](const auto& m) noexcept {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::is_same_v<T, state::TerritoryOwnerFlip>) {
            // Locate the territory in the AoS grid; flip owner.
            for (auto& t : s.grid.states) {
                if (t.id == m.territory) {
                    t.owner = m.new_owner;
                    break;
                }
            }
        }
        // BlockadeEdge / ExportControlFlow / CapacityRamp / ShockImpulse /
        // DistortionBudget / AttributionBudget / GateTransition / RegimeTransition
        // / FizzleRecord: Phase 3 implements the active mutations.  They remain
        // present in s.pending until expires_at, where Phase 8 retires them.
        (void)m;
    }, pe.mutation);
}

}  // namespace

void mature_pending(state::WorldState& s) noexcept {
    // §9.1: walk pending in canonical order (origin ActionLogId), apply any
    // whose activates_at == current tick.
    std::sort(s.pending.begin(), s.pending.end(),
        [](const state::PendingEffect& a, const state::PendingEffect& b) noexcept {
            if (a.activates_at != b.activates_at) return a.activates_at < b.activates_at;
            return a.origin < b.origin;
        });

    for (const auto& pe : s.pending) {
        if (pe.activates_at == s.tick) {
            apply_mutation(s, pe);
        }
    }
}

void retire_expired(state::WorldState& s) noexcept {
    // §9.8: erase pending effects that expired before this tick.
    s.pending.erase(
        std::remove_if(s.pending.begin(), s.pending.end(),
            [&](const state::PendingEffect& pe) noexcept {
                return pe.expires_at < s.tick;
            }),
        s.pending.end()
    );
}

}  // namespace rc::sim::action
