#pragma once
// NF::Physics — Rigid bodies, collision detection, character controller
#include "NF/Core/Core.h"

namespace NF {

struct AABB {
    Vec3 min, max;
    bool intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
};

class PhysicsWorld {
public:
    void init() { NF_LOG_INFO("Physics", "Physics world initialized"); }
    void shutdown() { NF_LOG_INFO("Physics", "Physics world shutdown"); }
    void step(float dt) { (void)dt; }

private:
};

} // namespace NF
