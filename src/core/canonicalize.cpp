#include "rcsim/core/canonicalize.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "rcsim/state/world_state.hpp"
#include "rcsim/state/canonicalization_state.hpp"

// §2.6: signed-zero canonicalization + NaN/Inf trap.
// §2.7: Kahan-style quantization residual buffer at tick boundary.
//
// Determinism note: canonicalize_for_hash is the last operation before bytes
// flow into Blake3. After this function, every bit of the double must be a
// deterministic function of the abstract numerical value — no signed-zero
// ambiguity, no NaN payload, no Inf.

namespace rc::sim::core {

double canonicalize_for_hash(double v) noexcept {
    if (v == 0.0) {
        // Both +0.0 and -0.0 compare equal; assigning +0.0 strips the sign bit.
        v = 0.0;
    }
    if (!std::isfinite(v)) {
        trap_nonfinite(v);
    }
    return v;
}

void trap_nonfinite(double v) noexcept {
    // Field identification is the caller's responsibility — surface the value
    // and abort. Phase 4 sync-log integration will wrap this with the field
    // identifier and tick number for forensic analysis.
    std::fprintf(stderr,
        "rcsim §2.6 trap: non-finite value %.17g encountered before canonical hash. "
        "Scenario broken or integrator diverged. Halting.\n",
        v);
    std::abort();
}

void canonicalize_state_with_residual(state::WorldState& s) noexcept {
    // §2.7: error-diffusion residual per field per territory.
    //   double augmented = unquantized + previous_residual;
    //   double quantized = dequantize(quantize(augmented));
    //   double new_residual = augmented - quantized;
    //   `quantized` enters canonical state (hashed); `new_residual` stored in sidecar.
    //
    // The grid stores `double` for performance during integration; quantization
    // here re-rounds each field to its Q-format LSB and accumulates the discarded
    // sub-LSB portion in the sidecar so it surfaces in the next tick's augment.
    //
    // Quantize-then-dequantize gives the canonical post-truncation `double`
    // (bit-identical to what Blake3 will hash via canonicalize_for_hash).
    auto& grid = s.grid.states;
    auto& canon = s.canon;
    const std::size_t N = grid.size();

    // Lazy-resize residual sidecars on first invocation (or after grid resize).
    if (canon.residual_r.size() != N) {
        canon.residual_r.assign(N, 0.0);
        canon.residual_H.assign(N, 0.0);
        canon.residual_G.assign(N, 0.0);
        canon.residual_M.assign(N, 0.0);
        canon.residual_Npop.assign(N, 0.0);
        canon.residual_S.assign(N, 0.0);
        canon.residual_Qelec.assign(N, 0.0);
        canon.residual_Qliq.assign(N, 0.0);
        canon.residual_D_local.assign(N, 0.0);
    }

    auto step_Q32_32 = [](double& field, double& residual) noexcept {
        double augmented = field + residual;
        double quantized = dequantize_Q32_32(quantize_Q32_32(augmented));
        residual = augmented - quantized;
        field = quantized;
    };
    auto step_Q48_16 = [](double& field, double& residual) noexcept {
        double augmented = field + residual;
        double quantized = dequantize_Q48_16(quantize_Q48_16(augmented));
        residual = augmented - quantized;
        field = quantized;
    };
    auto step_Q56_8 = [](double& field, double& residual) noexcept {
        double augmented = field + residual;
        double quantized = dequantize_Q56_8(quantize_Q56_8(augmented));
        residual = augmented - quantized;
        field = quantized;
    };

    for (std::size_t i = 0; i < N; ++i) {
        // §2.2 per-field formats:
        //   r       Q32.32   dimensionless
        //   H       Q48.16   persons
        //   G       Q48.16   HE-units
        //   M       Q56.8    USD-eq
        //   Npop    Q48.16   persons
        //   S       Q32.32   dimensionless
        //   Qelec   Q32.32   EJ/yr
        //   Qliq    Q32.32   EJ/yr
        //   D_local Q48.16   accumulator
        step_Q32_32(grid[i].r,       canon.residual_r[i]);
        step_Q48_16(grid[i].H,       canon.residual_H[i]);
        step_Q48_16(grid[i].G,       canon.residual_G[i]);
        step_Q56_8 (grid[i].M,       canon.residual_M[i]);
        step_Q48_16(grid[i].Npop,    canon.residual_Npop[i]);
        step_Q32_32(grid[i].S,       canon.residual_S[i]);
        step_Q32_32(grid[i].Qelec,   canon.residual_Qelec[i]);
        step_Q32_32(grid[i].Qliq,    canon.residual_Qliq[i]);
        step_Q48_16(grid[i].D_local, canon.residual_D_local[i]);
    }
}

}  // namespace rc::sim::core
