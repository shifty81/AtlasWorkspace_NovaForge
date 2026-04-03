#pragma once
// NF::Input — Keyboard, mouse, gamepad, action mappings
#include "NF/Core/Core.h"

namespace NF {

enum class KeyCode : uint16_t {
    Unknown = 0,
    W, A, S, D, Space, Escape, Enter, Tab, Shift, Ctrl, Alt,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Left, Right, Up, Down,
    Mouse1, Mouse2, Mouse3
};

struct InputState {
    bool keys[512] = {};
    float mouseX = 0.f, mouseY = 0.f;
    float mouseDeltaX = 0.f, mouseDeltaY = 0.f;
    float scrollDelta = 0.f;
};

class InputSystem {
public:
    void init() { NF_LOG_INFO("Input", "Input system initialized"); }
    void shutdown() { NF_LOG_INFO("Input", "Input system shutdown"); }
    void update() {}

    [[nodiscard]] const InputState& state() const { return m_state; }
    InputState& mutableState() { return m_state; }

private:
    InputState m_state;
};

} // namespace NF
