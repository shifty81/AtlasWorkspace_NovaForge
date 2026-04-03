#pragma once
// NF::Renderer — RHI (OpenGL), forward pipeline, mesh, materials
#include "NF/Core/Core.h"

namespace NF {

class Renderer {
public:
    bool init(int width, int height) {
        m_width = width;
        m_height = height;
        NF_LOG_INFO("Renderer", "Renderer initialized");
        return true;
    }

    void shutdown() {
        NF_LOG_INFO("Renderer", "Renderer shutdown");
    }

    void beginFrame() {}
    void endFrame() {}

    [[nodiscard]] int width() const { return m_width; }
    [[nodiscard]] int height() const { return m_height; }

private:
    int m_width = 0;
    int m_height = 0;
};

} // namespace NF
