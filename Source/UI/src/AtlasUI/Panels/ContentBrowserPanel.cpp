#include "NF/UI/AtlasUI/Panels/ContentBrowserPanel.h"

namespace NF::UI::AtlasUI {

void ContentBrowserPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar (outside clip so the header is never truncated)
    NF::Rect hdr = m_bounds;
    hdr.h = 22.f;
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    // Clip content to panel bounds to prevent text overflow
    context.pushClip(m_bounds);

    float y = m_bounds.y + 26.f;
    const float left = m_bounds.x + 8.f;
    const float contentW = m_bounds.w - 16.f;

    // Current path
    context.drawText({left, y, contentW, 14.f}, m_currentPath, 0, Theme::ColorToken::TextMuted);
    y += 20.f;

    // Separator
    context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
    y += Theme::Spacing::Small;

    // Cache hit-test geometry
    m_firstEntryY = y;
    m_entryX      = left;
    m_entryW      = contentW;

    // File entries
    int idx = 0;
    for (const auto& entry : m_entries) {
        if (y > m_bounds.y + m_bounds.h - 4.f) break;

        // Selection highlight
        if (idx == m_selectedIndex) {
            context.fillRect({m_bounds.x, y, m_bounds.w, 18.f}, Theme::ColorToken::Selection);
        }

        std::string icon = entry.isDirectory ? "[D] " : "[F] ";
        Color textColor = entry.isDirectory ? Theme::ColorToken::Accent : Theme::ColorToken::Text;
        context.drawText({left + 8.f, y, contentW - 8.f, 14.f},
                         icon + entry.name, 0, textColor);
        y += 18.f;
        ++idx;
    }

    context.popClip();
}

bool ContentBrowserPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;

    const bool curDown = context.primaryDown();
    const bool clicked = curDown && !m_prevPrimaryDown; // leading edge (press)
    m_prevPrimaryDown  = curDown;

    if (!clicked) return false;

    const NF::Vec2 mouse = context.mousePosition();

    // Hit-test each visible entry row
    float y = m_firstEntryY;
    for (int i = 0; i < static_cast<int>(m_entries.size()); ++i) {
        if (y > m_bounds.y + m_bounds.h - 4.f) break;
        const NF::Rect row{m_entryX, y, m_entryW, 18.f};
        if (rectContains(row, mouse)) {
            m_selectedIndex = i;
            return true;
        }
        y += 18.f;
    }

    return false;
}

} // namespace NF::UI::AtlasUI
