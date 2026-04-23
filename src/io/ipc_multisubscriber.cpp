#include "rcsim/io/ipc_multisubscriber.hpp"

#include "rcsim/state/world_state.hpp"

// §14: Local Unix-domain-socket multi-subscriber IPC. Phase 9 implementation.

namespace rc::sim::io {

IpcMultiSubscriberServer::IpcMultiSubscriberServer(const std::string& socket_path)
    : socket_path_(socket_path) {
    // TODO(phase 9, §14)
}

IpcMultiSubscriberServer::~IpcMultiSubscriberServer() = default;

void IpcMultiSubscriberServer::poll_new_subscribers() {
    // TODO(phase 9, §14)
}

void IpcMultiSubscriberServer::broadcast_tick(const state::WorldState& /*s*/) {
    // TODO(phase 9, §14)
}

void IpcMultiSubscriberServer::reap_disconnected() {
    // TODO(phase 9, §14)
}

}  // namespace rc::sim::io
