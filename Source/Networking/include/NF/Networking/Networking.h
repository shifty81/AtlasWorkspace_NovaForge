#pragma once
// NF::Networking — Sockets, replication, sessions, lockstep/rollback
#include "NF/Core/Core.h"

namespace NF {

enum class NetRole : uint8_t {
    None,
    Client,
    Server,
    ListenServer
};

class NetworkManager {
public:
    void init(NetRole role) {
        m_role = role;
        NF_LOG_INFO("Networking", "Network manager initialized");
    }
    void shutdown() { NF_LOG_INFO("Networking", "Network manager shutdown"); }
    void update(float dt) { (void)dt; }

    [[nodiscard]] NetRole role() const { return m_role; }

private:
    NetRole m_role = NetRole::None;
};

} // namespace NF
