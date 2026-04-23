// §19 Phase 0 gate: `rcsim-run --help` works.
// tools/rcsim_run.cpp — CLI driver for single-trajectory simulation from a scenario YAML.
// TODO(phase 1+): wire up scenario YAML loader, Advance loop, sync log emission.

#include <cstdio>
#include <cstring>
#include <string>

namespace {

void print_help() {
    std::puts("rcsim-run — single-trajectory simulation driver");
    std::puts("");
    std::puts("USAGE:");
    std::puts("  rcsim-run --scenario <path.yaml> [--sync-log <path.bin>] [--ticks <N>]");
    std::puts("  rcsim-run --help");
    std::puts("");
    std::puts("OPTIONS:");
    std::puts("  --scenario PATH   Scenario YAML file (see docs/DESIGN_v1.3.md §12)");
    std::puts("  --sync-log PATH   Binary sync-log output (see §10)");
    std::puts("  --ticks  N        Override scenario tick count");
    std::puts("  --help            Show this message and exit");
    std::puts("");
    std::puts("Scaffold build — implementation tracked in docs/DESIGN_v1.3.md §19 phase plan.");
}

}  // namespace

int main(int argc, char** argv) {
    // TODO(phase 1+): full argv parsing + scenario load + run.
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        }
    }

    // Minimal behavior: print help and exit 0 to satisfy §19 Phase 0 gate.
    print_help();
    return 0;
}
