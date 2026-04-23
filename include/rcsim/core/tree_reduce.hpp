#pragma once

// §2.4: Deterministic ordering + hand-rolled tree-reduce (no std::execution::par_unseq).
// DESIGN_v1.3.md §2.4: std::execution::par_unseq is forbidden for reductions.
// Required primitive: deterministic_tree_reduce with fixed pair-ordering by territory_id.
// CI gate: parallel_hash == sequential_hash across 1/4/16/64 threads.

#include <bit>
#include <cstddef>
#include <span>
#include <vector>

namespace rc::sim::core {

// §2.4: Identity element for a binary operation. Specialized per Op below.
// Primary template is undefined — specialization required.
template <typename Op>
struct IdentityFor;

// §2.4: Deterministic tree-reduce template.
// - Input span is iterated in fixed order (caller provides canonically-ordered values).
// - Buffer padded to next power of two with identity element.
// - Pairs combined in-place at fixed stride. No thread-scheduling dependence.
// TODO(phase 1, §2.4): implement per DESIGN_v1.3.md §2.4 (reference body in spec)
template <typename T, typename Op>
T deterministic_tree_reduce(std::span<const T> values, Op binary_op) {
    // TODO(phase 1, §2.4): implement the reference body:
    //   if (values.empty()) return T{};
    //   std::vector<T> buffer(values.begin(), values.end());
    //   size_t padded = std::bit_ceil(buffer.size());
    //   buffer.resize(padded, IdentityFor<Op>::value);
    //   while (padded > 1) {
    //     for (size_t i = 0; i < padded / 2; ++i) {
    //       buffer[i] = binary_op(buffer[2*i], buffer[2*i + 1]);
    //     }
    //     padded /= 2;
    //   }
    //   return buffer[0];
    (void)values;
    (void)binary_op;
    return T{};
}

// §2.4: Sum-of-doubles specialization. Identity = 0.0.
struct SumDouble {
    double operator()(double a, double b) const noexcept { return a + b; }
};
template <>
struct IdentityFor<SumDouble> {
    static constexpr double value = 0.0;
};

// §2.4: Weighted sum specialization (used for S_AI aggregation with G-weights, §4.2).
// Stored as pair (weighted_value, weight); sum member-wise then divide at the end.
struct WeightedPair {
    double weighted_value;
    double weight;
};
struct SumWeighted {
    WeightedPair operator()(WeightedPair a, WeightedPair b) const noexcept {
        return {a.weighted_value + b.weighted_value, a.weight + b.weight};
    }
};
template <>
struct IdentityFor<SumWeighted> {
    static constexpr WeightedPair value{0.0, 0.0};
};

// §2.4: Byte-sum specialization for hash accumulation.
// TODO(phase 1, §2.4): consider canonical byte reduction for hash inputs.

}  // namespace rc::sim::core
