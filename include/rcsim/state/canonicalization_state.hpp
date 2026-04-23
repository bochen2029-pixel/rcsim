#pragma once

// §2.7: Kahan-style quantization residual buffer.
// DESIGN_v1.3.md §2.7: truncation at tick boundary destroys slow derivatives
// (δvalue < LSB per tick). Fix: error-diffusion residual, stored in non-hashed sidecar.
// ~150 × 9 × 8 = 10.8 KB sidecar. Does not affect canonical hash.

#include <vector>

namespace rc::sim::state {

// Forward declaration — territory grid is authored in world_state.hpp.
struct TerritoryGrid;

// §2.7: CanonicalizationState — quantized canonical grid + Kahan residual sidecar.
// The grid is hashed; residual is not. Residual preserves thermodynamic conservation
// across 30-year runs (First Law violation fix).
struct CanonicalizationState {
    // §2.7: per-field residual vectors, indexed parallel to TerritoryGrid::states.
    // Each vector has size N (number of territories). Kept as doubles (pre-quantization).
    // NOT hashed. Accumulates precision across ticks.
    std::vector<double> residual_r;
    std::vector<double> residual_H;
    std::vector<double> residual_G;
    std::vector<double> residual_M;
    std::vector<double> residual_Npop;
    std::vector<double> residual_S;
    std::vector<double> residual_Qelec;
    std::vector<double> residual_Qliq;
    std::vector<double> residual_D_local;

    // §2.7: total sidecar size = N × 9 × 8 bytes.
    // At N=150: 150 × 9 × 8 = 10.8 KB.
};

}  // namespace rc::sim::state
