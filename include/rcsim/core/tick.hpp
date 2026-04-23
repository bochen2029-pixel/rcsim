#pragma once

// §2.5: Ticks are uint64_t; no wall-clock time in core.
// DESIGN_v1.3.md §2.5: No wall-clock time; no std::chrono::system_clock; no timestamps in canonical state.

#include <cstdint>

namespace rc::sim::core {

// §2.5: Tick number type. Monotonically increasing across simulation.
using TickNumber = uint64_t;

// Alias surfaced for call sites that read as Tick in spec text (§5.2, §5.3, §6.1).
using Tick = TickNumber;

// §8.1: Default dt_years = 1.0 / 52.0 (1 tick = 1 week).
// Duration in years expressed as double (tick-width, not TickNumber).
using DurationYears = double;

}  // namespace rc::sim::core
