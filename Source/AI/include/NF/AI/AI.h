#pragma once
// NF::AI — Behavior graphs, memory, NPC logic, faction relationships
#include "NF/Core/Core.h"

namespace NF {

enum class AIBehavior : uint8_t {
    Idle,
    Patrol,
    Attack,
    Flee,
    Mine,
    Trade,
    Haul,
    Guard,
    Explore
};

struct AIMemory {
    float decayRate = 0.01f;
    float threat = 0.f;
    float friendliness = 0.5f;
};

class AISystem {
public:
    void init() { NF_LOG_INFO("AI", "AI system initialized"); }
    void shutdown() { NF_LOG_INFO("AI", "AI system shutdown"); }
    void update(float dt) { (void)dt; }
};

} // namespace NF
