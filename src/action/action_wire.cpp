#include "rcsim/action/action_wire.hpp"

// §10.1a: Canonical wire format. Little-endian scalars, uint32-prefixed UTF-8 strings,
// uint32 enums frozen at schema_major, no implicit padding, reflection-free.
// Test 08_action_wire_roundtrip verifies bit-identical round-trip across compilers.

namespace rc::sim::action {

std::vector<uint8_t> serialize_canonical(const Action& /*action*/) noexcept {
    // TODO(phase 4, §10.1a)
    return {};
}

Action deserialize_canonical(std::span<const uint8_t> /*bytes*/) noexcept {
    // TODO(phase 4, §10.1a)
    return Pass{};
}

bool roundtrip_bit_identical(const Action& /*action*/) noexcept {
    // TODO(phase 4, §10.1a, §16.1)
    return false;
}

}  // namespace rc::sim::action
