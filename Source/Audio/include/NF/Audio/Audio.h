#pragma once
// NF::Audio — Device, spatial audio, mixer, cues
#include "NF/Core/Core.h"

namespace NF {

class AudioDevice {
public:
    bool init() { NF_LOG_INFO("Audio", "Audio device initialized"); return true; }
    void shutdown() { NF_LOG_INFO("Audio", "Audio device shutdown"); }
    void update(float dt) { (void)dt; }
};

} // namespace NF
