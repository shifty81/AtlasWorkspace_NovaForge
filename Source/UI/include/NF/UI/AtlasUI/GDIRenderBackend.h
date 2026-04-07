#pragma once

#include "NF/UI/AtlasUI/Interfaces.h"
#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/UI.h"
#include <variant>

namespace NF::UI::AtlasUI {

/// Bridges the AtlasUI DrawList to the UIRenderer pipeline.
///
/// AtlasUI stores colours as 0xAARRGGBB (alpha in the high byte).
/// UIRenderer/GDIBackend expects 0xRRGGBBAA (red in the high byte).
/// This class converts the format and dispatches each DrawCommand variant
/// to the appropriate UIRenderer method.
///
/// PushClipCmd and PopClipCmd are no-ops here: GDI does not expose a
/// per-frame clip-region API compatible with our batched draw path.
class GDIRenderBackend final : public IRenderBackend {
public:
    void setUIRenderer(UIRenderer* renderer) { m_uiRenderer = renderer; }
    [[nodiscard]] UIRenderer* uiRenderer() const { return m_uiRenderer; }

    // IRenderBackend
    void beginFrame() override {}
    void endFrame()   override {}

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
        m_uiRenderer->drawText(c.rect.x, c.rect.y, c.text, toRGBA(c.color));
    }
    void dispatch(const PushClipCmd&) { /* GDI clip not supported — no-op */ }
    void dispatch(const PopClipCmd&)  { /* GDI clip not supported — no-op */ }

    UIRenderer* m_uiRenderer = nullptr;
};

} // namespace NF::UI::AtlasUI
