#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// Lightweight project descriptor loaded from a .atlas.json file.
/// Mirrors the layout of NF::ProjectEntry so that callers requiring only
/// name/path/version/description can use either interchangeably.
struct ProjectEntry {
    std::string name;        ///< Display name (from JSON "name" field, else filename stem)
    std::string path;        ///< Absolute path to the .atlas.json file
    std::string description; ///< Optional description from JSON
    std::string version;     ///< Optional version string from JSON

    [[nodiscard]] bool isValid() const { return !path.empty(); }
};

/// AtlasUI ProjectPickerPanel — modal "Open Workspace" dialog.
///
/// Replaces the legacy NF::ProjectPickerPanel rendering path.  All input
/// handling (row selection, Open, Cancel) is performed via handleInput()
/// instead of going through the UIContext, so clicks now actually work.
///
/// The public API is a strict superset of the legacy class so that
/// EditorApp callers (tests included) require no changes.
class ProjectPickerPanel final : public PanelBase {
public:
    static constexpr const char* kExtension = ".atlas.json";
    static constexpr size_t      kMaxProjects = 64;

    ProjectPickerPanel() : PanelBase("atlas.project_picker", "Open Workspace") {
        setVisible(false); // modal picker is hidden until explicitly show()-n
    }

    // ── IWidget overrides ────────────────────────────────────────
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    // ── Visibility (mirrors legacy show/hide API) ─────────────
    void show() { setVisible(true); }
    void hide() { setVisible(false); }

    // ── Project management ────────────────────────────────────
    /// Scan a directory for .atlas.json files. Returns the number found.
    size_t scanDirectory(const std::string& dir);

    void addProject(const ProjectEntry& entry) {
        if (m_projects.size() < kMaxProjects)
            m_projects.push_back(entry);
    }

    void clearProjects() {
        m_projects.clear();
        m_selectedIndex = -1;
    }

    /// Select a project by 0-based index. Returns false if out of range.
    bool selectProject(int index) {
        if (index < 0 || index >= static_cast<int>(m_projects.size())) return false;
        m_selectedIndex = index;
        return true;
    }

    /// Load the currently selected project. Hides the picker on success.
    bool loadSelected() {
        if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_projects.size()))
            return false;
        m_loadedPath = m_projects[static_cast<size_t>(m_selectedIndex)].path;
        m_loaded     = true;
        hide();
        return true;
    }

    /// Directly load a project by path, adding it to the list if absent.
    bool loadProject(const std::string& path) {
        if (path.empty()) return false;
        int idx = -1;
        for (int i = 0; i < static_cast<int>(m_projects.size()); ++i) {
            if (m_projects[static_cast<size_t>(i)].path == path) { idx = i; break; }
        }
        if (idx < 0) {
            ProjectEntry pe;
            pe.path = path;
            auto stem = std::filesystem::path(path).stem().string();
            if (stem.size() > 6 && stem.substr(stem.size() - 6) == ".atlas")
                stem = stem.substr(0, stem.size() - 6);
            pe.name = stem;
            m_projects.push_back(std::move(pe));
            idx = static_cast<int>(m_projects.size()) - 1;
        }
        m_selectedIndex = idx;
        m_loadedPath    = path;
        m_loaded        = true;
        hide();
        return true;
    }

    // ── Accessors ─────────────────────────────────────────────
    [[nodiscard]] const std::vector<ProjectEntry>& projects() const { return m_projects; }
    [[nodiscard]] size_t                            projectCount()  const { return m_projects.size(); }
    [[nodiscard]] int                               selectedIndex() const { return m_selectedIndex; }
    [[nodiscard]] bool                              hasSelection()  const { return m_selectedIndex >= 0; }

    [[nodiscard]] const ProjectEntry* selectedProject() const {
        if (!hasSelection()) return nullptr;
        return &m_projects[static_cast<size_t>(m_selectedIndex)];
    }

    [[nodiscard]] bool               isLoaded()           const { return m_loaded; }
    [[nodiscard]] const std::string& loadedProjectPath()  const { return m_loadedPath; }

private:
    static std::string extractJsonString(const std::string& json,
                                         const std::string& key,
                                         const std::string& fallback);

    std::vector<ProjectEntry> m_projects;
    int         m_selectedIndex   = -1;
    bool        m_loaded          = false;
    std::string m_loadedPath;

    // Hit-test geometry written during paint(), read during handleInput().
    NF::Rect m_openBtnRect{};
    NF::Rect m_cancelBtnRect{};
    float    m_firstRowY  = 0.f;
    float    m_rowX       = 0.f;
    float    m_rowW       = 0.f;

    // Leading-edge click detection (was-up → now-down).
    bool m_prevPrimaryDown = false;
};

} // namespace NF::UI::AtlasUI
