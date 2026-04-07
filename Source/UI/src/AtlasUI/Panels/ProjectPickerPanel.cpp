#include "NF/UI/AtlasUI/Panels/ProjectPickerPanel.h"

#include <algorithm>
#include <system_error>

namespace NF::UI::AtlasUI {

// ── paint ─────────────────────────────────────────────────────────

void ProjectPickerPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    const float width  = m_bounds.w;
    const float height = m_bounds.h;

    // Dim full-screen overlay
    context.fillRect(m_bounds, 0xAA000000u);

    // Dialog box (centred)
    constexpr float dw = 520.f, dh = 360.f;
    const float dx = m_bounds.x + (width  - dw) * 0.5f;
    const float dy = m_bounds.y + (height - dh) * 0.5f;
    const NF::Rect dlg{dx, dy, dw, dh};

    context.fillRect(dlg,  0xFF2B2B2Bu);
    context.drawRect(dlg,  0xFF555555u);

    // Title bar
    context.fillRect({dx, dy, dw, 28.f}, 0xFF3C3C3Cu);
    context.drawText({dx + 12.f, dy + 6.f, dw - 24.f, 16.f},
                     "Open Workspace", 0, 0xFFDCDCDCu);

    // Project list header
    context.drawText({dx + 12.f, dy + 38.f, dw - 24.f, 14.f},
                     "Available Projects:", 0, 0xFF9CDCFEu);

    // Cache hit-test geometry
    m_firstRowY = dy + 60.f;
    m_rowX      = dx + 8.f;
    m_rowW      = dw - 16.f;

    float ly = m_firstRowY;
    for (int i = 0; i < static_cast<int>(m_projects.size()); ++i) {
        const auto& p = m_projects[static_cast<size_t>(i)];
        const NF::Rect row{m_rowX, ly, m_rowW, 26.f};
        context.fillRect(row, (i == m_selectedIndex) ? 0xFF2255AAu : 0xFF2A2A2Au);
        std::string label = p.name;
        if (!p.version.empty()) label += "  v" + p.version;
        context.drawText({m_rowX + 8.f, ly + 5.f, m_rowW - 16.f, 14.f},
                         label, 0, 0xFFDCDCDCu);
        ly += 30.f;
    }

    if (m_projects.empty()) {
        context.drawText({dx + 12.f, ly + 4.f, dw - 24.f, 14.f},
                         "No projects found in Project/ directory.", 0, 0xFF9CDCFEu);
        ly += 28.f;
    }
    (void)ly;

    // Buttons
    const float bby = dy + dh - 40.f;
    m_openBtnRect   = {dx + dw - 200.f, bby, 88.f, 26.f};
    m_cancelBtnRect = {dx + dw - 104.f, bby, 88.f, 26.f};

    context.fillRect(m_openBtnRect,   m_selectedIndex >= 0 ? 0xFF1A7A1Au : 0xFF333333u);
    context.fillRect(m_cancelBtnRect, 0xFF5A2222u);
    context.drawRect(m_openBtnRect,   0xFF555555u);
    context.drawRect(m_cancelBtnRect, 0xFF555555u);
    context.drawText({m_openBtnRect.x   + 20.f, bby + 5.f, 48.f, 14.f},
                     "Open",   0, 0xFFDCDCDCu);
    context.drawText({m_cancelBtnRect.x + 12.f, bby + 5.f, 64.f, 14.f},
                     "Cancel", 0, 0xFFDCDCDCu);
}

// ── handleInput ───────────────────────────────────────────────────

bool ProjectPickerPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;

    const bool curDown  = context.primaryDown();
    const bool clicked  = curDown && !m_prevPrimaryDown; // leading edge (press)
    m_prevPrimaryDown   = curDown;

    if (!clicked) return false;

    const NF::Vec2 mouse = context.mousePosition();

    // Row selection
    float ly = m_firstRowY;
    for (int i = 0; i < static_cast<int>(m_projects.size()); ++i) {
        const NF::Rect row{m_rowX, ly, m_rowW, 26.f};
        if (rectContains(row, mouse)) {
            m_selectedIndex = i;
            return true;
        }
        ly += 30.f;
    }

    // Open button
    if (rectContains(m_openBtnRect, mouse) && m_selectedIndex >= 0) {
        loadSelected();
        return true;
    }

    // Cancel button
    if (rectContains(m_cancelBtnRect, mouse)) {
        hide();
        return true;
    }

    return false;
}

// ── scanDirectory ─────────────────────────────────────────────────

size_t ProjectPickerPanel::scanDirectory(const std::string& dir) {
    m_projects.clear();
    m_selectedIndex = -1;
    if (dir.empty()) return 0;

    std::error_code ec;
    if (!std::filesystem::exists(dir, ec)) return 0;

    for (auto& entry : std::filesystem::directory_iterator(dir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec)) continue;
        const auto p = entry.path();
        const std::string filename = p.filename().string();

        // Match files ending with .atlas.json (11 chars minimum: "x.atlas.json")
        if (filename.size() < 11) continue;
        if (filename.substr(filename.size() - 11) != kExtension) continue;

        ProjectEntry pe;
        pe.path = p.string();
        pe.name = p.stem().string();
        // Strip ".atlas" second extension if present (e.g. "NovaForge.atlas" → "NovaForge")
        if (pe.name.size() > 6 && pe.name.substr(pe.name.size() - 6) == ".atlas")
            pe.name = pe.name.substr(0, pe.name.size() - 6);

        // Best-effort JSON parse for name/version/description
        std::ifstream f(pe.path);
        if (f.is_open()) {
            std::string content((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
            pe.name        = extractJsonString(content, "name",        pe.name);
            pe.version     = extractJsonString(content, "version",     "");
            pe.description = extractJsonString(content, "description", "");
        }

        m_projects.push_back(std::move(pe));
        if (m_projects.size() >= kMaxProjects) break;
    }
    return m_projects.size();
}

// ── extractJsonString (private helper) ───────────────────────────

std::string ProjectPickerPanel::extractJsonString(const std::string& json,
                                                   const std::string& key,
                                                   const std::string& fallback) {
    const std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return fallback;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return fallback;
    const auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return fallback;
    return json.substr(pos + 1, end - pos - 1);
}

} // namespace NF::UI::AtlasUI
