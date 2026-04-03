#pragma once
// NF::Engine — ECS, world/level, behavior trees, asset system
#include "NF/Core/Core.h"

namespace NF {

using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;

class EntityManager {
public:
    EntityID createEntity() {
        EntityID id = m_nextID++;
        m_alive.push_back(id);
        return id;
    }

    void destroyEntity(EntityID id) {
        std::erase(m_alive, id);
    }

    [[nodiscard]] const std::vector<EntityID>& alive() const { return m_alive; }
    [[nodiscard]] size_t count() const { return m_alive.size(); }

private:
    EntityID m_nextID = 1;
    std::vector<EntityID> m_alive;
};

} // namespace NF
