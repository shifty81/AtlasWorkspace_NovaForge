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
        // Quad batching: emit 4 vertices + 6 indices for a filled rectangle
        UIVertex topLeft     = {{r.x, r.y},             {0, 0}, color};
        UIVertex topRight    = {{r.x + r.w, r.y},       {1, 0}, color};
        UIVertex bottomRight = {{r.x + r.w, r.y + r.h}, {1, 1}, color};
        UIVertex bottomLeft  = {{r.x, r.y + r.h},       {0, 1}, color};

        uint32_t base = static_cast<uint32_t>(m_vertices.size());
        m_vertices.push_back(topLeft);
        m_vertices.push_back(topRight);
        m_vertices.push_back(bottomRight);
        m_vertices.push_back(bottomLeft);

        m_indices.push_back(base + 0);
        m_indices.push_back(base + 1);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 0);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 3);

        ++m_quadCount;
    }

    void drawText(float x, float y, std::string_view text, uint32_t color = 0xFFFFFFFF) {
        // Simple bitmap-style text rendering: each character is a small quad
        constexpr float kCharWidth = 8.f;
        constexpr float kCharHeight = 14.f;
        float cx = x;
        for (char ch : text) {
            if (ch == '\n') { cx = x; y += kCharHeight + 2.f; continue; }
            if (ch == ' ')  { cx += kCharWidth; continue; }

            Rect charRect{cx, y, kCharWidth, kCharHeight};
            drawRect(charRect, color);
            cx += kCharWidth;
        }
        ++m_textDrawCount;
    }

    void drawRectOutline(const Rect& r, uint32_t color, float thickness = 1.f) {
        drawRect({r.x, r.y, r.w, thickness}, color);                       // top
        drawRect({r.x, r.y + r.h - thickness, r.w, thickness}, color);     // bottom
        drawRect({r.x, r.y, thickness, r.h}, color);                       // left
        drawRect({r.x + r.w - thickness, r.y, thickness, r.h}, color);     // right
    }

    void endFrame() {
        // In a real renderer this would flush batched quads to GPU
        m_lastFrameQuadCount = m_quadCount;
        m_lastFrameTextCount = m_textDrawCount;
    }

    [[nodiscard]] const std::vector<UIVertex>& vertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<uint32_t>& indices() const { return m_indices; }
    [[nodiscard]] size_t quadCount() const { return m_lastFrameQuadCount; }
    [[nodiscard]] size_t textDrawCount() const { return m_lastFrameTextCount; }
    [[nodiscard]] float viewportWidth() const { return m_viewportW; }
    [[nodiscard]] float viewportHeight() const { return m_viewportH; }

private:
    float m_viewportW = 0.f, m_viewportH = 0.f;
    std::vector<UIVertex> m_vertices;
    std::vector<uint32_t> m_indices;
    size_t m_quadCount = 0;
    size_t m_textDrawCount = 0;
    size_t m_lastFrameQuadCount = 0;
    size_t m_lastFrameTextCount = 0;
};

} // namespace NF
