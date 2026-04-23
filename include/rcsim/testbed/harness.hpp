#pragma once

// §11.1: Sweep configuration + harness top-level.
// DESIGN_v1.3.md §11.1: YAML structure preserved from v1.2; §11.2 emits Parquet tiers;
// §11.3 PrincipalPolicy abstract + ScriptedHeuristicPolicy v1 default; §11.4 Blake3 seed derivation.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rcsim/core/hash.hpp"
#include "rcsim/core/tick.hpp"
#include "rcsim/testbed/sweep.hpp"

// Forward declare — WorldState authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

// Forward declare PrincipalPolicy.
namespace rc::sim::testbed { class PrincipalPolicy; }

namespace rc::sim::testbed {

// §11.1: SweepConfig — parsed from YAML.
//   SPEC_AMBIGUOUS(§11.1): concrete YAML fields inherited from v1.2; re-enumerate in phase 5.
struct SweepConfig {
    uint64_t seed_root;
    uint32_t num_param_samples;
    uint32_t num_monte_carlo_samples;
    core::Tick sim_ticks;
    std::string scenario_base_yaml;
    std::vector<ParameterSweep> parameter_sweeps;
    std::vector<MetricSpec> metrics;
    std::vector<PriorSpec> priors;
    std::string output_dir;
};

// §11.1: RunResult — single-trajectory result summary.
struct RunResult {
    uint32_t sweep_id;
    uint32_t param_index;
    uint32_t sample_index;
    core::StateHash final_hash;
    // TODO(phase 5, §11.1): additional summary stats (RMSE, convergence flags, etc.)
};

// §11.1: Testbed — the Monte Carlo harness.
class Testbed {
public:
    // TODO(phase 5, §11.1): implement constructor taking SweepConfig + PrincipalPolicy.
    explicit Testbed(SweepConfig cfg, std::unique_ptr<PrincipalPolicy> policy);
    ~Testbed();

    // §11.1: run — iterates over param grid × Monte Carlo samples, deterministic seeding.
    // TODO(phase 5, §11.1, §11.4): implement per DESIGN_v1.3.md §11
    std::vector<RunResult> run();

private:
    SweepConfig cfg_;
    std::unique_ptr<PrincipalPolicy> policy_;
};

}  // namespace rc::sim::testbed
