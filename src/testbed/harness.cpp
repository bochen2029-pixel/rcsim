#include "rcsim/testbed/harness.hpp"

#include "rcsim/testbed/principal_policy.hpp"

// §11.1: Testbed Monte Carlo harness.

namespace rc::sim::testbed {

Testbed::Testbed(SweepConfig cfg, std::unique_ptr<PrincipalPolicy> policy)
    : cfg_(std::move(cfg)), policy_(std::move(policy)) {
    // TODO(phase 5, §11.1)
}

Testbed::~Testbed() = default;

std::vector<RunResult> Testbed::run() {
    // TODO(phase 5, §11.1, §11.4)
    return {};
}

}  // namespace rc::sim::testbed
