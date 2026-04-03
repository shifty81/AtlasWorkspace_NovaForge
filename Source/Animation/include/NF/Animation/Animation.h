#pragma once
// NF::Animation — Skeleton, blend tree, state machine, IK
#include "NF/Core/Core.h"

namespace NF {

class AnimationSystem {
public:
    void init() { NF_LOG_INFO("Animation", "Animation system initialized"); }
    void shutdown() { NF_LOG_INFO("Animation", "Animation system shutdown"); }
    void update(float dt) { (void)dt; }
};

} // namespace NF
