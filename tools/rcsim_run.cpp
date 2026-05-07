// §19 Phase 1+2: rcsim-run drives a single trajectory from a scenario YAML.
//
// Usage:
//   rcsim-run --help
//   rcsim-run --scenario PATH [--ticks N] [--print-hash]
//
// Phase 2 wires the §9 10-phase advance loop:
//   1. mature_pending                                 — action::mature_pending
//   2a. drain inbound actions, sort + validate_epistemic
//   2b. deduct_resources (unconditional on ground truth)
//   2c. apply (ground-truth with Fizzle)
//   3. compute observations per-principal             — Phase 3 stub (no-op)
//   4. compute coupling flows                         — dynamics::integer_network_flow_solve
//   5. evaluate gates once at tick boundary
//   6. integrate RK4 with gates closed-over constant
//   7. canonicalize state with Kahan residual
//   8. retire_expired pending effects                 — action::retire_expired
//   9. hash state                                     — core::canonical_hash
//   10. log                                           — Phase 4 stub (no-op)

#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

#include "rcsim/action/apply_with_fizzle.hpp"
#include "rcsim/action/effect.hpp"
#include "rcsim/action/queue.hpp"
#include "rcsim/action/validate_epistemic.hpp"
#include "rcsim/core/canonicalize.hpp"
#include "rcsim/core/hash.hpp"
#include "rcsim/dynamics/coupling.hpp"
#include "rcsim/dynamics/integrator.hpp"
#include "rcsim/dynamics/ode_system.hpp"
#include "rcsim/io/scenario_yaml.hpp"
#include "rcsim/observer/measurement.hpp"
#include "rcsim/state/world_state.hpp"

namespace {

void print_help() {
    std::puts("rcsim-run — single-trajectory simulation driver");
    std::puts("");
    std::puts("USAGE:");
    std::puts("  rcsim-run --scenario <path.yaml> [--ticks N] [--print-hash]");
    std::puts("  rcsim-run --help");
    std::puts("");
    std::puts("OPTIONS:");
    std::puts("  --scenario PATH   Scenario YAML file (see docs/DESIGN_v1.3.md §12)");
    std::puts("  --ticks  N        Override scenario tick count");
    std::puts("  --print-hash      Print final-tick canonical hash (hex) and exit");
    std::puts("  --help            Show this message and exit");
}

void print_hash_hex(const rc::sim::core::StateHash& h) {
    for (auto b : h.bytes) std::printf("%02x", b);
    std::printf("\n");
}

struct Args {
    std::string scenario;
    long long   ticks_override = -1;
    bool        print_hash = false;
    bool        help = false;
};

Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        const char* s = argv[i];
        if (std::strcmp(s, "--help") == 0 || std::strcmp(s, "-h") == 0) {
            a.help = true;
        } else if (std::strcmp(s, "--scenario") == 0 && i + 1 < argc) {
            a.scenario = argv[++i];
        } else if (std::strcmp(s, "--ticks") == 0 && i + 1 < argc) {
            a.ticks_override = std::strtoll(argv[++i], nullptr, 10);
        } else if (std::strcmp(s, "--print-hash") == 0) {
            a.print_hash = true;
        }
    }
    return a;
}

// §9 advance loop — Phase 2 wiring.
rc::sim::core::StateHash advance_one_tick(
    rc::sim::state::WorldState& s,
    rc::sim::dynamics::Rk4Integrator& integ,
    double dt_years,
    rc::sim::action::ActionQueue& inbound
) noexcept {
    using namespace rc::sim;
    s.tick++;

    // Phase 1: mature pending effects (TerritoryOwnerFlip, BlockadeEdge, etc.)
    action::mature_pending(s);

    // Phase 2a: drain inbound actions in canonical order (tick, principal_id, action_seq).
    std::sort(inbound.actions.begin(), inbound.actions.end(), action::ActionCompare{});

    for (auto& pa : inbound.actions) {
        if (pa.tick != s.tick) continue;

        // Resolve the actor's last_seen observation. Phase 2 fallback: empty
        // observation when principal doesn't exist yet — validate_epistemic
        // will reject with UnknownTarget for territory-targeting actions.
        observer::Observation obs_default;
        const observer::Observation* obs_ptr = &obs_default;
        for (const auto& p : s.principals) {
            if (p.id == pa.principal_id) { obs_ptr = &p.last_seen; break; }
        }

        auto vr = action::validate_epistemic(*obs_ptr, pa.principal_id, pa.action);
        if (!vr.ok) {
            // §9.2a: reject with reason — log, no state mutation.
            state::ActionLogEntry entry;
            entry.id          = 0;        // Phase 4 fills ActionLogId
            entry.applied_at  = s.tick;
            entry.principal   = pa.principal_id;
            entry.action_seq  = pa.action_seq;
            entry.priority    = pa.priority;
            entry.outcome     = state::ApplyOutcome::RejectedEpistemic;
            entry.reason      = static_cast<uint32_t>(vr.reason);
            s.action_log_delta.push_back(std::move(entry));
            continue;
        }

        // Phase 2b: resource deduction unconditional on ground truth.
        for (auto& p : s.principals) {
            if (p.id == pa.principal_id) {
                action::deduct_resources(p, pa.action);
                break;
            }
        }

        // Phase 2c: ground-truth apply with Fizzle.
        auto ar = action::apply(std::move(s), pa.principal_id, pa.action);
        s = std::move(ar.state);

        state::ActionLogEntry entry;
        entry.id         = 0;
        entry.applied_at = s.tick;
        entry.principal  = pa.principal_id;
        entry.action_seq = pa.action_seq;
        entry.priority   = pa.priority;
        if (ar.fizzled) {
            entry.outcome = state::ApplyOutcome::Fizzled;
            entry.reason  = static_cast<uint32_t>(ar.reason);
        } else if (ar.applied) {
            entry.outcome = state::ApplyOutcome::Applied;
            entry.reason  = 0;
        }
        s.action_log_delta.push_back(std::move(entry));
    }

    // Phase 3: compute observations per principal — stub until Phase 3 of §19.
    //   for (auto& p : s.principals) {
    //     p.last_seen = observer_->observe(s, p.id, ...);
    //   }

    // Phase 4: coupling flows via Integer Network Flow.
    dynamics::integer_network_flow_solve(s);

    // Phase 5: evaluate gates once at tick boundary.
    auto gates = dynamics::evaluate_gates(s);

    // Phase 6: integrate RK4 with gates closed-over constant (no bisection per §8.2).
    integ.step(s, dt_years, gates);

    // Phase 7: canonicalize state with Kahan residual sidecar.
    core::canonicalize_state_with_residual(s);

    // Phase 8: retire expired pending effects.
    action::retire_expired(s);

    // Phase 9: hash state.
    auto hash = core::canonical_hash(s);

    // Phase 10: log — sync_log writer is Phase 4 of §19.
    s.time_years += dt_years;

    return hash;
}

}  // namespace

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);
    if (args.help || argc == 1) { print_help(); return 0; }
    if (args.scenario.empty()) {
        std::fputs("rcsim-run: --scenario is required (or pass --help)\n", stderr);
        return 2;
    }

    using namespace rc::sim;
    state::WorldState s;
    try {
        s = io::load_scenario(args.scenario);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "rcsim-run: %s\n", e.what());
        return 1;
    }

    long long ticks = args.ticks_override >= 0 ? args.ticks_override : 520;

    dynamics::ODESystem ode;
    dynamics::IntegratorConfig cfg;
    cfg.dt_years          = 1.0 / 52.0;
    cfg.substeps_per_tick = 4;
    dynamics::Rk4Integrator integ(&ode, cfg);

    action::ActionQueue inbound;       // Phase 2: empty for now (no scripted actions yet)
    core::StateHash final_hash{};

    for (long long t = 0; t < ticks; ++t) {
        final_hash = advance_one_tick(s, integ, cfg.dt_years, inbound);
    }

    if (args.print_hash) {
        print_hash_hex(final_hash);
    } else {
        std::printf("rcsim-run: completed %lld ticks; final hash ", ticks);
        print_hash_hex(final_hash);
    }
    return 0;
}
