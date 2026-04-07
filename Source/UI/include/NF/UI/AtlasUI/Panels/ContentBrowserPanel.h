#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI ContentBrowserPanel — displays project file system for asset browsing.
/// Replaces the legacy NF::Editor::ContentBrowserPanel for the AtlasUI framework.
class ContentBrowserPanel final : public PanelBase {
public:
    ContentBrowserPanel()
        : PanelBase("atlas.content_browser", "Content Browser") {}

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    void setCurrentPath(const std::string& path) { m_currentPath = path; }
    [[nodiscard]] const std::string& currentPath() const { return m_currentPath; }

    struct FileEntry {
        std::string name;
        bool isDirectory = false;
    };

    void clearEntries() { m_entries.clear(); m_selectedIndex = -1; }
    void addEntry(const std::string& name, bool isDirectory) {
        m_entries.push_back({name, isDirectory});
    }
    [[nodiscard]] const std::vector<FileEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

    [[nodiscard]] int  selectedIndex() const { return m_selectedIndex; }
    [[nodiscard]] bool hasSelection()  const { return m_selectedIndex >= 0; }
    [[nodiscard]] const FileEntry* selectedEntry() const {
        if (!hasSelection()) return nullptr;
        return &m_entries[static_cast<size_t>(m_selectedIndex)];
    }

private:
    std::string m_currentPath = "/";
    std::vector<FileEntry> m_entries;

    // Entry selection
    int m_selectedIndex = -1;

    // Hit-test geometry written during paint(), read during handleInput()
    float m_firstEntryY = 0.f;
    float m_entryX      = 0.f;
    float m_entryW      = 0.f;

    // Leading-edge click detection (was-up → now-down)
    bool m_prevPrimaryDown = false;
};

} // namespace NF::UI::AtlasUI
