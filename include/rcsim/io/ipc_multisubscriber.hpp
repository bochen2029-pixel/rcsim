#pragma once

// §14: Multi-subscriber IPC (new — v1.3).
// DESIGN_v1.3.md §14: Local Unix-domain-socket IPC supports multiple renderer subscribers
// simultaneously (globe + dashboard). Per-subscriber sequence-number tracking; each
// subscriber reads from its own cursor; broadcast tick updates fan out in subscription order.
// No backpressure — slow subscribers drop frames rather than block core. Subscriber disconnect
// detected via socket EOF and cleaned up without core interruption.
// Phase 9 implementation dependency. Not blocking for phase 0-8.

#include <cstdint>
#include <memory>
#include <string>

// Forward decl — WorldState authored in state/world_state.hpp.
namespace rc::sim::state { struct WorldState; }

namespace rc::sim::io {

// §14: Subscriber handle. Opaque to core; tracked by MultiSubscriberServer.
struct SubscriberId {
    uint64_t value;
};

// §14: IpcMultiSubscriberServer — core-side broadcast surface.
class IpcMultiSubscriberServer {
public:
    // TODO(phase 9, §14): implement constructor that binds the local socket.
    explicit IpcMultiSubscriberServer(const std::string& socket_path);
    ~IpcMultiSubscriberServer();

    // §14: accept any pending connections; non-blocking.
    // TODO(phase 9, §14): implement
    void poll_new_subscribers();

    // §14: fan-out a tick update to all live subscribers. Drops frames on slow subscribers.
    // TODO(phase 9, §14): implement per DESIGN_v1.3.md §14
    void broadcast_tick(const state::WorldState& s);

    // §14: clean up disconnected subscribers (detected via socket EOF).
    // TODO(phase 9, §14): implement
    void reap_disconnected();

    // §14: SPEC_AMBIGUOUS — Windows platform: Unix-domain-sockets are available on
    //   Windows 10+ (AF_UNIX). Decision on whether to support named pipes as a fallback
    //   deferred to phase 9. Scaffold assumes AF_UNIX on all platforms (§15.1 CI matrix).
private:
    std::string socket_path_;
};

}  // namespace rc::sim::io
