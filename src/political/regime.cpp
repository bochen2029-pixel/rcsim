#include "rcsim/political/regime.hpp"

// §7.1, §10.1a: regime enum values frozen at schema_major; additive within schema_minor.

namespace rc::sim::political {

PoliticalRegime regime_from_wire(uint32_t /*v*/, uint32_t /*schema_minor*/) {
    // TODO(phase 4, §10.1a): hard-halt on unknown enum value with
    //   "unknown enum value N at schema_minor M" diagnostic.
    return PoliticalRegime::Democracy;
}

}  // namespace rc::sim::political
