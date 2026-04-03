#pragma once
// NF::UI — Custom 2D renderer (quad batching + stb_easy_font). No ImGui.
#include "NF/Core/Core.h"

namespace NF {

struct UIVertex {
    Vec2 position;
    Vec2 uv;
    uint32_t color = 0xFFFFFFFF;
};

class UIRenderer {
public:
    void init() { NF_LOG_INFO("UI", "UI renderer initialized"); }
    void shutdown() { NF_LOG_INFO("UI", "UI renderer shutdown"); }

    void beginFrame(float viewportW, float viewportH) {
        m_viewportW = viewportW;
        m_viewportH = viewportH;
        m_vertices.clear();
    }

    void drawRect(const Rect& r, uint32_t color) {
        (void)r; (void)color;
        // Quad batching placeholder
    }

    void drawText(float x, float y, std::string_view text, uint32_t color = 0xFFFFFFFF) {
        (void)x; (void)y; (void)text; (void)color;
        // stb_easy_font placeholder
    }

    void endFrame() {}

private:
    float m_viewportW = 0.f, m_viewportH = 0.f;
    std::vector<UIVertex> m_vertices;
};

} // namespace NF
