#pragma once

// §11.1: ParameterSweep, MetricSpec, PriorSpec types.
// DESIGN_v1.3.md §11: YAML structure preserved from v1.2.
// §11.4: Seed derivation via Blake3 per (seed_root, param_idx, sample_idx).

#include <cstdint>
#include <string>
#include <vector>

namespace rc::sim::testbed {

// §11.1: ParameterSweep — describes a single parameter's grid.
//   SPEC_AMBIGUOUS(§11.1): exact field names/ranges inherited from v1.2; enumerated in phase 5.
struct ParameterSweep {
    std::string parameter_name;
    double min_value;
    double max_value;
    uint32_t num_points;
    std::string sampling;    // e.g., "linear", "log"
};

// §11.1: MetricSpec — which observable to record at end-of-run.
struct MetricSpec {
    std::string name;
    std::string aggregator;  // e.g., "mean", "final", "max_drawdown"
};

// §11.1: PriorSpec — prior distribution for Bayesian calibration (phase 7a).
//   SPEC_AMBIGUOUS(§11.1): prior distribution family (normal, lognormal, beta) not pinned;
//   enumerate in phase 7a when IMM-RBPF priors are chosen.
struct PriorSpec {
    std::string parameter_name;
    std::string distribution_family;
    std::vector<double> parameters;   // e.g., [mean, stddev] for normal
};

// §11.4: derive per-(param, sample) seed from seed_root via Blake3-256 truncation.
// TODO(phase 5, §11.4): implement per DESIGN_v1.3.md §11.4
uint64_t derive_sample_seed(uint64_t seed_root, uint32_t param_idx, uint32_t sample_idx) noexcept;

}  // namespace rc::sim::testbed
