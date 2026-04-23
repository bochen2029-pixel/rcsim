#pragma once

// §7.1: PoliticalRegime enum with explicit uint8_t underlying and explicit enumerator values.
// DESIGN_v1.3.md §7.1, §10.1a enum rules: values frozen at schema_major; additive-only within schema_minor.

#include <cstdint>

namespace rc::sim::political {

// §7.1: PoliticalRegime — explicit-underlying uint8_t, explicit values.
enum class PoliticalRegime : uint8_t {
    Democracy             = 0,
    Autocracy             = 1,
    TheocraticAutocracy   = 2,
    Petrostate            = 3,
    FailedState           = 4,
    TransitionalState     = 5
    // Values frozen at schema_major; additive-only within schema_minor.
};

// §10.1a: enum-rule helpers — conversion for action_wire + hard-halt on unknown.
inline constexpr uint32_t regime_to_wire(PoliticalRegime r) noexcept {
    return static_cast<uint32_t>(r);
}

// TODO(phase 4, §10.1a): implement hard-halt on unknown enum value with
//   "unknown enum value N at schema_minor M" diagnostic.
PoliticalRegime regime_from_wire(uint32_t v, uint32_t schema_minor);

}  // namespace rc::sim::political
