#pragma once

#include "NF/UI/AtlasUI/Theme.h"
#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <functional>
#include <string>
#include <utility>

namespace NF::AtlasUI {

class Button final : public WidgetBase {
public:
    using ClickHandler = std::function<void()>;

    explicit Button(std::string text, ClickHandler onClick = {})
        : m_text(std::move(text)), m_onClick(std::move(onClick)) {}

    void setOnClick(ClickHandler onClick) { m_onClick = std::move(onClick); }
    void setText(std::string text) { m_text = std::move(text); }

    void measure(ILayoutContext& context) override {
        auto textSize = context.measureText(m_text, 14.f);
        m_desiredSize = {textSize.x + 20.f, textSize.y + 12.f};
    }

    void paint(IPaintContext& context) override {
        if (!m_visible) return;
        Color fill = Theme::ColorToken::SurfaceAlt;
        if (m_pressed) fill = Theme::ColorToken::Pressed;
        else if (m_hovered) fill = Theme::ColorToken::AccentHover;
        context.fillRect(m_bounds, fill);
        context.drawRect(m_bounds, Theme::ColorToken::Border);
        context.drawText(insetRect(m_bounds, Theme::Spacing::Medium, Theme::Spacing::Small), m_text, 0, Theme::ColorToken::Text);
    }

    bool handleInput(IInputContext& context) override {
        if (!m_visible) return false;
        const bool inside = rectContains(m_bounds, context.mousePosition());
        m_hovered = inside;
        if (inside && context.primaryDown() && !m_pressed) {
            m_pressed = true;
            context.capturePointer(this);
            return true;
        }
        if (m_pressed && !context.primaryDown()) {
            context.releasePointer(this);
            bool trigger = inside;
            m_pressed = false;
            if (trigger && m_onClick) m_onClick();
            return trigger;
        }
        return inside;
    }

    [[nodiscard]] Vec2 desiredSize() const { return m_desiredSize; }

private:
    std::string m_text;
    ClickHandler m_onClick;
    Vec2 m_desiredSize{};
    bool m_hovered = false;
    bool m_pressed = false;
};

} // namespace NF::AtlasUI
