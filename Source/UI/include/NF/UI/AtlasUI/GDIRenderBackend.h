#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"
#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/UI.h"
#include <algorithm>
#include <variant>
#include <vector>

namespace NF::UI::AtlasUI {

/// Bridges the AtlasUI DrawList to the UIRenderer pipeline.
///
/// AtlasUI stores colours as 0xAARRGGBB (alpha in the high byte).
/// UIRenderer/GDIBackend expects 0xRRGGBBAA (red in the high byte).
/// This class converts the format and dispatches each DrawCommand variant
/// to the appropriate UIRenderer method.
///
/// PushClipCmd / PopClipCmd maintain a clip-rect stack.  DrawTextCmd is
/// clipped against the active rect: text whose origin falls outside is
/// skipped, and text that starts inside but would overflow is truncated
/// to the available width (using ~8 px per character, matching the
/// Consolas font configured in GDIBackend).
class GDIRenderBackend final : public IRenderBackend {
public:
    void setUIRenderer(UIRenderer* renderer) { m_uiRenderer = renderer; }
    [[nodiscard]] UIRenderer* uiRenderer() const { return m_uiRenderer; }

    // IRenderBackend
    void beginFrame() override { m_clipStack.clear(); }
    void endFrame()   override { m_clipStack.clear(); }

    void drawCommandBuffer(const DrawList& drawList) override {
        if (!m_uiRenderer) return;
        for (const auto& cmd : drawList.commands()) {
            std::visit([this](const auto& c) { dispatch(c); }, cmd);
        }
    }

private:
    // AtlasUI: 0xAARRGGBB  →  UIRenderer: 0xRRGGBBAA
    static uint32_t toRGBA(Color argb) {
        const uint8_t a =  static_cast<uint8_t>((argb >> 24) & 0xFF);
        const uint8_t r =  static_cast<uint8_t>((argb >> 16) & 0xFF);
        const uint8_t g =  static_cast<uint8_t>((argb >>  8) & 0xFF);
        const uint8_t b =  static_cast<uint8_t>( argb        & 0xFF);
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) <<  8) | a;
    }

    void dispatch(const FillRectCmd& c) {
        m_uiRenderer->drawRect(c.rect, toRGBA(c.color));
    }
    void dispatch(const DrawRectCmd& c) {
        m_uiRenderer->drawRectOutline(c.rect, toRGBA(c.color), 1.f);
    }
    void dispatch(const DrawTextCmd& c) {
        if (!m_clipStack.empty()) {
            const NF::Rect& clip = m_clipStack.back();
            // Skip text whose origin is entirely outside the clip rect
            if (c.rect.y >= clip.y + clip.h) return;
            if (c.rect.y + c.rect.h <= clip.y) return;
            if (c.rect.x >= clip.x + clip.w) return;

            // Truncate text that overflows the right edge of the clip rect
            const float availW = std::min(c.rect.w, (clip.x + clip.w) - c.rect.x);
            if (availW <= 0.f) return;

            constexpr float kApproxCharW = 8.f;
            const auto maxChars = static_cast<size_t>(availW / kApproxCharW);
            if (maxChars < c.text.size()) {
                std::string truncated;
                if (maxChars > 3) {
                    truncated = c.text.substr(0, maxChars - 3) + "...";
                } else {
                    truncated = c.text.substr(0, maxChars);
                }
                m_uiRenderer->drawText(c.rect.x, c.rect.y, truncated, toRGBA(c.color));
                return;
            }
        }
        m_uiRenderer->drawText(c.rect.x, c.rect.y, c.text, toRGBA(c.color));
    }
    void dispatch(const PushClipCmd& c) { m_clipStack.push_back(c.rect); }
    void dispatch(const PopClipCmd&)    {
        if (!m_clipStack.empty()) m_clipStack.pop_back();
    }

    UIRenderer* m_uiRenderer = nullptr;
    std::vector<NF::Rect> m_clipStack;
};

} // namespace NF::UI::AtlasUI
