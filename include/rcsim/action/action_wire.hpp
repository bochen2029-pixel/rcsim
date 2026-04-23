#pragma once

// §10.1a: Action payload canonical encoding — with enum rules (AEGIS §2.3).
// DESIGN_v1.3.md §10.1a:
//   - Scalars: explicit little-endian byte-order (not host-order)
//   - Strings: uint32 length prefix + UTF-8 bytes, no null terminator
//   - Enums: explicit uint32 little-endian; values frozen at schema_major;
//            additive-only within schema_minor; unknown values in replay → hard halt
//   - Field order: matches struct declaration; reflection-free
//   - Padding: none (explicit padding fields where needed, but serialized bytes have none)
//   - Versioning: wire format frozen at schema_major bump
// Test 08_action_wire_roundtrip.cpp verifies round-trip across compilers.

#include <cstdint>
#include <span>
#include <vector>

#include "rcsim/action/action.hpp"

namespace rc::sim::action {

// §10.1a: Schema version. Bumped via schema_major when enum values or field layout changes;
// schema_minor is additive-only.
struct SchemaVersion {
    uint32_t schema_major;
    uint32_t schema_minor;
};

inline constexpr SchemaVersion kCurrentSchemaVersion{1, 0};

// §10.1a: serialize_canonical — canonical byte encoding for an Action.
// TODO(phase 4, §10.1a): implement per DESIGN_v1.3.md §10.1a
std::vector<uint8_t> serialize_canonical(const Action& action) noexcept;

// §10.1a: deserialize_canonical — inverse of serialize_canonical. Hard-halts on
// unknown enum values with "unknown enum value N at schema_minor M" diagnostic.
// Returns Action on success; std::nullopt-style failure is a halt (not a return).
// TODO(phase 4, §10.1a): implement per DESIGN_v1.3.md §10.1a
Action deserialize_canonical(std::span<const uint8_t> bytes) noexcept;

// §10.1a: round-trip helper used by 08_action_wire_roundtrip.cpp.
// TODO(phase 4, §10.1a, §16.1): implement
bool roundtrip_bit_identical(const Action& action) noexcept;

}  // namespace rc::sim::action
