#include "NF/UI/AtlasUI/Panels/HierarchyPanel.h"
#include <cstdio>

namespace NF::UI::AtlasUI {

void HierarchyPanel::paint(IPaintContext& context) {
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

    // Search filter
    if (!m_searchFilter.empty()) {
        context.drawText({left, y, contentW, 14.f},
                         "Filter: " + m_searchFilter, 0, Theme::ColorToken::TextMuted);
        y += 18.f;
    }

    // Rebuild hit-test row list each frame
    m_rowRects.clear();

    // Entity list
    for (const auto& entity : m_entities) {
        if (y > m_bounds.y + m_bounds.h - 4.f) break;

        // Filter check
        if (!m_searchFilter.empty() &&
            entity.name.find(m_searchFilter) == std::string::npos) {
            continue;
        }

        float indent = static_cast<float>(entity.depth) * 16.f;

        // Selection highlight
        if (entity.selected) {
            context.fillRect({m_bounds.x, y, m_bounds.w, 18.f}, Theme::ColorToken::Selection);
        }

        // Store hit-test rect for this row
        m_rowRects.push_back({entity.id, {m_bounds.x, y, m_bounds.w, 18.f}});

        // Entity label
        char buf[64];
        if (entity.name.empty()) {
            std::snprintf(buf, sizeof(buf), "Entity #%d", entity.id);
        } else {
            std::snprintf(buf, sizeof(buf), "%s (#%d)", entity.name.c_str(), entity.id);
        }
        context.drawText({left + indent, y + 2.f, contentW - indent, 14.f},
                         buf, 0,
                         entity.selected ? Theme::ColorToken::Accent : Theme::ColorToken::Text);
        y += 18.f;
    }

    context.popClip();
}

bool HierarchyPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;

    const bool curDown = context.primaryDown();
    const bool clicked = curDown && !m_prevPrimaryDown; // leading edge
    m_prevPrimaryDown  = curDown;

    if (!clicked) return false;

    const NF::Vec2 mouse = context.mousePosition();

    for (const auto& row : m_rowRects) {
        if (rectContains(row.rect, mouse)) {
            m_clickedEntityId = row.id;
            // Update selection state in the entity list
            for (auto& entity : m_entities) {
                entity.selected = (entity.id == row.id);
            }
            return true;
        }
    }

    return false;
}

} // namespace NF::UI::AtlasUI
