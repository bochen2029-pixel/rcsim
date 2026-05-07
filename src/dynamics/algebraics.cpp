#include "rcsim/dynamics/algebraics.hpp"

// §4.2, §4.5: algebraic derived quantities. Resolutions R-04..R-07 in
// docs/SPEC_AMBIGUOUS_RESOLUTIONS.md pin the v1 functional forms.

namespace rc::sim::dynamics {

// §4.5: Hill function for S_AI-driven build rate.
//   rho(S) = rho_max · S^n / (S_half^n + S^n)
double rho(double S, double S_half, double n, double rho_max) noexcept {
    if (S <= 0.0) {
        return 0.0;
    }
    double Sn       = core::pow_canonical(S,      n);
    double S_half_n = core::pow_canonical(S_half, n);
    return rho_max * Sn / (S_half_n + Sn);
}

// R-06: eta_i — fractional AI labor share for territory i.
//   eta = G / (G + H + G_floor); range [0, 1].
double eta(double /*r*/, double H, double G) noexcept {
    constexpr double kG_floor = 1.0e-3;
    return G / (G + H + kG_floor);
}

// R-07: phi_i — AI deployment per capita (proxy via H denominator until
// phi gains a Npop-aware overload in Phase 2).
double phi(double /*r*/, double H, double G) noexcept {
    constexpr double kN_floor = 1.0;
    return G / (H + kN_floor);
}

// R-04: e_U(nu) — residual employability multiplier.
//   nu ∈ [0, 1] is unemployment fraction; e_U = (1 - nu)^2.
double e_U(double nu) noexcept {
    double clamped = nu < 0.0 ? 0.0 : (nu > 1.0 ? 1.0 : nu);
    double one_minus = 1.0 - clamped;
    return one_minus * one_minus;
}

// R-05: xi(D_global) — discovery efficacy with diminishing returns.
//   xi = xi_zero / (1 + D_global / D_half).
double xi_of_D(double D_global) noexcept {
    constexpr double kXi_zero  = 1.0;
    constexpr double kXi_Dhalf = 1.0e10;
    return kXi_zero / (1.0 + D_global / kXi_Dhalf);
}

}  // namespace rc::sim::dynamics
