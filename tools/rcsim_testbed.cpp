// tools/rcsim_testbed.cpp — Monte Carlo sweep driver per §11.
// TODO(phase 5+): wire up SweepConfig YAML + Testbed::run() + Parquet tiered output.

#include <cstdio>
#include <cstring>

namespace {

void print_help() {
    std::puts("rcsim-testbed — Monte Carlo sweep driver");
    std::puts("");
    std::puts("USAGE:");
    std::puts("  rcsim-testbed --sweep <path.yaml> [--output-dir <dir>] [--threads <N>]");
    std::puts("  rcsim-testbed --help");
    std::puts("");
    std::puts("OPTIONS:");
    std::puts("  --sweep PATH       Sweep YAML (see docs/DESIGN_v1.3.md §11.1)");
    std::puts("  --output-dir PATH  Parquet output directory (Tier 1 + Tier 2 per §11.2)");
    std::puts("  --threads N        Thread-pool size (parallel per-territory; deterministic reductions)");
    std::puts("  --help             Show this message and exit");
    std::puts("");
    std::puts("Scaffold build — implementation tracked in docs/DESIGN_v1.3.md §19 phase plan.");
}

}  // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        }
    }

    print_help();
    return 0;
}
