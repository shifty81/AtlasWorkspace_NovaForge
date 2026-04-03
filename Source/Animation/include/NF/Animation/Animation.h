#pragma once
// NF::Animation — Skeleton, blend tree, state machine, IK
#include "NF/Core/Core.h"
#include <map>

namespace NF {

// ── Bone / Joint ─────────────────────────────────────────────────

struct Bone {
    std::string name;
    int32_t parentIndex = -1; // -1 = root
    Transform localBindPose;
};

// ── Skeleton ─────────────────────────────────────────────────────

class Skeleton {
public:
    Skeleton() = default;
    explicit Skeleton(std::string name) : m_name(std::move(name)) {}

    int32_t addBone(const std::string& boneName, int32_t parentIndex,
                     const Transform& bindPose = {}) {
        int32_t index = static_cast<int32_t>(m_bones.size());
        m_bones.push_back({boneName, parentIndex, bindPose});
        m_boneNames[boneName] = index;
        return index;
    }

    [[nodiscard]] int32_t findBone(const std::string& boneName) const {
        auto it = m_boneNames.find(boneName);
        return (it != m_boneNames.end()) ? it->second : -1;
    }

    [[nodiscard]] const Bone& bone(int32_t index) const { return m_bones[index]; }
    [[nodiscard]] size_t boneCount() const { return m_bones.size(); }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<Bone>& bones() const { return m_bones; }

private:
    std::string m_name;
    std::vector<Bone> m_bones;
    std::unordered_map<std::string, int32_t> m_boneNames;
};

// ── Animation Keyframe ───────────────────────────────────────────

struct TransformKey {
    float time = 0.f;
    Transform value;
};

struct AnimationChannel {
    int32_t boneIndex = -1;
    std::vector<TransformKey> keys;

    Transform sample(float time) const {
        if (keys.empty()) return {};
        if (keys.size() == 1 || time <= keys.front().time) return keys.front().value;
        if (time >= keys.back().time) return keys.back().value;

        // Find surrounding keyframes
        for (size_t i = 0; i + 1 < keys.size(); ++i) {
            if (time >= keys[i].time && time < keys[i + 1].time) {
                float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
                Transform result;
                result.position = Vec3::lerp(keys[i].value.position,
                                              keys[i + 1].value.position, t);
                result.rotation = Quat::slerp(keys[i].value.rotation,
                                               keys[i + 1].value.rotation, t);
                result.scale = Vec3::lerp(keys[i].value.scale,
                                           keys[i + 1].value.scale, t);
                return result;
            }
        }
        return keys.back().value;
    }
};

// ── Animation Clip ───────────────────────────────────────────────

class AnimationClip {
public:
    AnimationClip() = default;
    AnimationClip(std::string name, float duration)
        : m_name(std::move(name)), m_duration(duration) {}

    AnimationChannel& addChannel(int32_t boneIndex) {
        m_channels.push_back({boneIndex, {}});
        return m_channels.back();
    }

    void sample(float time, std::vector<Transform>& outPose) const {
        for (auto& channel : m_channels) {
            if (channel.boneIndex >= 0 &&
                channel.boneIndex < static_cast<int32_t>(outPose.size())) {
                outPose[channel.boneIndex] = channel.sample(time);
            }
        }
    }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] float duration() const { return m_duration; }
    [[nodiscard]] const std::vector<AnimationChannel>& channels() const { return m_channels; }
    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

private:
    std::string m_name;
    float m_duration = 0.f;
    std::vector<AnimationChannel> m_channels;
};

// ── Animation State Machine ──────────────────────────────────────

struct AnimationTransition {
    std::string targetState;
    std::function<bool()> condition;
    float blendDuration = 0.2f;
};

struct AnimationStateNode {
    std::string name;
    const AnimationClip* clip = nullptr;
    bool looping = true;
    float speed = 1.f;
    std::vector<AnimationTransition> transitions;
};

class AnimationStateMachine {
public:
    void addState(const std::string& name, const AnimationClip* clip,
                   bool looping = true, float speed = 1.f) {
        m_states[name] = {name, clip, looping, speed, {}};
        if (m_currentState.empty()) m_currentState = name;
    }

    void addTransition(const std::string& from, const std::string& to,
                        std::function<bool()> condition, float blendDuration = 0.2f) {
        auto it = m_states.find(from);
        if (it != m_states.end()) {
            it->second.transitions.push_back({to, std::move(condition), blendDuration});
        }
    }

    void update(float dt) {
        auto it = m_states.find(m_currentState);
        if (it == m_states.end()) return;

        auto& state = it->second;
        m_stateTime += dt * state.speed;

        if (state.clip && m_stateTime >= state.clip->duration()) {
            if (state.looping) {
                m_stateTime = 0.f;
            }
        }

        // Check transitions
        for (auto& transition : state.transitions) {
            if (transition.condition && transition.condition()) {
                m_currentState = transition.targetState;
                m_stateTime = 0.f;
                break;
            }
        }
    }

    [[nodiscard]] const std::string& currentState() const { return m_currentState; }
    [[nodiscard]] float stateTime() const { return m_stateTime; }
    [[nodiscard]] size_t stateCount() const { return m_states.size(); }

    [[nodiscard]] const AnimationClip* currentClip() const {
        auto it = m_states.find(m_currentState);
        return (it != m_states.end()) ? it->second.clip : nullptr;
    }

    void sampleCurrentPose(std::vector<Transform>& outPose) const {
        auto it = m_states.find(m_currentState);
        if (it != m_states.end() && it->second.clip) {
            it->second.clip->sample(m_stateTime, outPose);
        }
    }

private:
    std::unordered_map<std::string, AnimationStateNode> m_states;
    std::string m_currentState;
    float m_stateTime = 0.f;
};

// ── Animation System ─────────────────────────────────────────────

class AnimationSystem {
public:
    void init() { NF_LOG_INFO("Animation", "Animation system initialized"); }

    void shutdown() {
        NF_LOG_INFO("Animation", "Animation system shutdown");
    }

    void update(float dt) {
        (void)dt;
        // In a real implementation, this would iterate registered
        // state machines and update animations for all entities
    }
};

} // namespace NF
