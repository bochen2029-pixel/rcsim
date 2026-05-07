#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>

#include "rcsim/core/canonicalize.hpp"

// §16.1 / §2.6: signed-zero canonicalization.
// Hash invariant under +0.0 vs -0.0 — bit-pattern collapses to +0.0 before hashing.

TEST_CASE("signed-zero canonicalization strips negative zero", "[determinism]") {
    using rc::sim::core::canonicalize_for_hash;

    double pos_zero =  0.0;
    double neg_zero = -0.0;

    REQUIRE(pos_zero == neg_zero);  // numerically equal
    uint64_t pos_bits, neg_bits;
    std::memcpy(&pos_bits, &pos_zero, sizeof(pos_bits));
    std::memcpy(&neg_bits, &neg_zero, sizeof(neg_bits));
    REQUIRE(pos_bits != neg_bits);  // bit-distinct

    double canon_pos = canonicalize_for_hash(pos_zero);
    double canon_neg = canonicalize_for_hash(neg_zero);

    uint64_t canon_pos_bits, canon_neg_bits;
    std::memcpy(&canon_pos_bits, &canon_pos, sizeof(canon_pos_bits));
    std::memcpy(&canon_neg_bits, &canon_neg, sizeof(canon_neg_bits));

    REQUIRE(canon_pos_bits == canon_neg_bits);
    REQUIRE(canon_pos_bits == 0u);
}

TEST_CASE("canonicalize preserves non-zero finite values bit-exactly", "[determinism]") {
    using rc::sim::core::canonicalize_for_hash;

    for (double v : {1.0, -1.5, 3.141592653589793, 1e-300, 1e300}) {
        double canon = canonicalize_for_hash(v);
        uint64_t orig_bits, canon_bits;
        std::memcpy(&orig_bits, &v, sizeof(orig_bits));
        std::memcpy(&canon_bits, &canon, sizeof(canon_bits));
        REQUIRE(orig_bits == canon_bits);
    }
}
