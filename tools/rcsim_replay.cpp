// tools/rcsim_replay.cpp — deterministic replay driver per §10.2.
// TODO(phase 4+): wire up ReplayDriver + hash-chain verification + desync diagnostics.

#include <cstdio>
#include <cstring>

namespace {

void print_help() {
    std::puts("rcsim-replay — deterministic replay + hash-chain verification");
    std::puts("");
    std::puts("USAGE:");
    std::puts("  rcsim-replay --sync-log <path.bin> [--tick-start N] [--tick-end N]");
    std::puts("  rcsim-replay --help");
    std::puts("");
    std::puts("OPTIONS:");
    std::puts("  --sync-log PATH     Binary sync log (§10.1)");
    std::puts("  --tick-start N      Start tick (default: 0)");
    std::puts("  --tick-end N        End tick (default: end-of-log)");
    std::puts("  --help              Show this message and exit");
    std::puts("");
    std::puts("On hash mismatch: hard halt with diagnostic (§10.2).");
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
