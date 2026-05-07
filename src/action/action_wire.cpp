#include "rcsim/action/action_wire.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <variant>

// §10.1a: canonical wire encoding.
//
// Layout (little-endian throughout, no padding):
//   uint32 variant_index   // std::variant alternative index, frozen at schema_major
//   <variant payload bytes per type below>
//
// Numeric scalars:        IEEE-754 binary64 / int64 / uint64 / int32 / uint32, all LE
// Enums:                  uint32 LE
// TerritoryId/PrincipalId: uint32 LE (R-01)
// Tick:                   uint64 LE
// Strings:                uint32 LE length, then UTF-8 bytes (no null) — none in v1 actions.
//
// Unknown variant_index on deserialize → halt with diagnostic.

namespace rc::sim::action {

namespace {

void put_u32(std::vector<uint8_t>& out, uint32_t v) {
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFFu));
}

void put_u64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) out.push_back(static_cast<uint8_t>((v >> (8 * i)) & 0xFFu));
}

void put_double(std::vector<uint8_t>& out, double v) {
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    put_u64(out, bits);
}

uint32_t read_u32(std::span<const uint8_t> bytes, std::size_t& off) {
    if (off + 4 > bytes.size()) {
        std::fprintf(stderr, "rcsim §10.1a: truncated wire format at offset %zu\n", off);
        std::abort();
    }
    uint32_t v = 0;
    for (int i = 0; i < 4; ++i) v |= static_cast<uint32_t>(bytes[off + static_cast<std::size_t>(i)]) << (8 * i);
    off += 4;
    return v;
}

uint64_t read_u64(std::span<const uint8_t> bytes, std::size_t& off) {
    if (off + 8 > bytes.size()) {
        std::fprintf(stderr, "rcsim §10.1a: truncated wire format at offset %zu\n", off);
        std::abort();
    }
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(bytes[off + static_cast<std::size_t>(i)]) << (8 * i);
    off += 8;
    return v;
}

double read_double(std::span<const uint8_t> bytes, std::size_t& off) {
    uint64_t bits = read_u64(bytes, off);
    double v;
    std::memcpy(&v, &bits, sizeof(v));
    return v;
}

[[noreturn]] void halt_unknown_variant(uint32_t idx) {
    std::fprintf(stderr,
        "rcsim §10.1a: unknown variant_index %u at schema_minor %u — halting per spec.\n",
        idx, kCurrentSchemaVersion.schema_minor);
    std::abort();
}

}  // namespace

std::vector<uint8_t> serialize_canonical(const Action& action) noexcept {
    std::vector<uint8_t> out;
    out.reserve(64);
    uint32_t idx = static_cast<uint32_t>(action.index());
    put_u32(out, idx);

    std::visit([&](const auto& spec) {
        using T = std::decay_t<decltype(spec)>;
        if constexpr (std::is_same_v<T, SeizeTerritory> ||
                      std::is_same_v<T, Blockade> ||
                      std::is_same_v<T, ExportControl> ||
                      std::is_same_v<T, SovereignExpropriation>) {
            put_u32(out, spec.target);
        } else if constexpr (std::is_same_v<T, DatacenterBuild> ||
                              std::is_same_v<T, BuildInfrastructure>) {
            put_u32(out, spec.site);
            put_double(out, spec.investment_M);
        } else if constexpr (std::is_same_v<T, CapitalAllocation> ||
                              std::is_same_v<T, MonetaryIssuance>) {
            put_double(out, spec.amount_M);
        } else if constexpr (std::is_same_v<T, AIDeploymentReallocation>) {
            put_double(out, spec.new_deployment_fraction_A);
        } else if constexpr (std::is_same_v<T, Sanction>) {
            put_u32(out, spec.target);
        } else if constexpr (std::is_same_v<T, ManipulateObservation>) {
            put_u32(out, spec.target_observer);
            put_u32(out, static_cast<uint32_t>(spec.field));
            put_double(out, spec.investment_M);
            put_double(out, spec.investment_S);
            put_double(out, spec.distortion_magnitude);
            put_u64(out, spec.duration);
        } else if constexpr (std::is_same_v<T, AttributeDistortion>) {
            put_u32(out, spec.accused);
            put_u32(out, static_cast<uint32_t>(spec.field));
            put_double(out, spec.investment_M);
            put_u64(out, spec.duration);
        } else {
            // Pass / CurrencySwap / VirtualizationPush / MobilizeMilitary /
            // CeaseFire — payload-less; just the variant_index.
            (void)spec;
        }
    }, action);

    return out;
}

Action deserialize_canonical(std::span<const uint8_t> bytes) noexcept {
    std::size_t off = 0;
    uint32_t idx = read_u32(bytes, off);

    // Index MUST match the std::variant declaration in action.hpp. If action.hpp
    // is reordered, schema_major must bump per §10.1a.
    switch (idx) {
        case 0: { SeizeTerritory v; v.target = read_u32(bytes, off); return v; }
        case 1: { Blockade v;       v.target = read_u32(bytes, off); return v; }
        case 2: { ExportControl v;  v.target = read_u32(bytes, off); return v; }
        case 3: {
            DatacenterBuild v;
            v.site = read_u32(bytes, off);
            v.investment_M = read_double(bytes, off);
            return v;
        }
        case 4: {
            BuildInfrastructure v;
            v.site = read_u32(bytes, off);
            v.investment_M = read_double(bytes, off);
            return v;
        }
        case 5: { CapitalAllocation v; v.amount_M = read_double(bytes, off); return v; }
        case 6: { MonetaryIssuance v;  v.amount_M = read_double(bytes, off); return v; }
        case 7:  return CurrencySwap{};
        case 8:  return VirtualizationPush{};
        case 9: {
            AIDeploymentReallocation v;
            v.new_deployment_fraction_A = read_double(bytes, off);
            return v;
        }
        case 10: { SovereignExpropriation v; v.target = read_u32(bytes, off); return v; }
        case 11: return MobilizeMilitary{};
        case 12: return CeaseFire{};
        case 13: { Sanction v; v.target = read_u32(bytes, off); return v; }
        case 14: {
            ManipulateObservation v;
            v.target_observer      = read_u32(bytes, off);
            v.field                = static_cast<observer::ObservationField>(read_u32(bytes, off));
            v.investment_M         = read_double(bytes, off);
            v.investment_S         = read_double(bytes, off);
            v.distortion_magnitude = read_double(bytes, off);
            v.duration             = read_u64(bytes, off);
            return v;
        }
        case 15: {
            AttributeDistortion v;
            v.accused      = read_u32(bytes, off);
            v.field        = static_cast<observer::ObservationField>(read_u32(bytes, off));
            v.investment_M = read_double(bytes, off);
            v.duration     = read_u64(bytes, off);
            return v;
        }
        case 16: return Pass{};
        default: halt_unknown_variant(idx);
    }
}

bool roundtrip_bit_identical(const Action& action) noexcept {
    auto bytes1 = serialize_canonical(action);
    Action again = deserialize_canonical(std::span<const uint8_t>(bytes1.data(), bytes1.size()));
    auto bytes2 = serialize_canonical(again);
    return bytes1 == bytes2;
}

}  // namespace rc::sim::action
