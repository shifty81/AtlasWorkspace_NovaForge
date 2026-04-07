#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// AtlasUI HierarchyPanel — displays entity tree with search filter.
/// Replaces the legacy NF::Editor::HierarchyPanel for the AtlasUI framework.
class HierarchyPanel final : public PanelBase {
public:
    HierarchyPanel()
        : PanelBase("atlas.hierarchy", "Hierarchy") {}

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    void setSearchFilter(const std::string& filter) { m_searchFilter = filter; }
    [[nodiscard]] const std::string& searchFilter() const { return m_searchFilter; }

    struct EntityEntry {
        int id = 0;
        std::string name;
        bool selected = false;
        int depth = 0;
    };

    void clearEntities() { m_entities.clear(); m_clickedEntityId = -1; }
    void addEntity(int id, const std::string& name, bool selected = false, int depth = 0) {
        m_entities.push_back({id, name, selected, depth});
    }
    [[nodiscard]] const std::vector<EntityEntry>& entities() const { return m_entities; }
    [[nodiscard]] size_t entityCount() const { return m_entities.size(); }

    /// Returns the entity id that was most recently clicked, or -1 if none.
    [[nodiscard]] int clickedEntityId() const { return m_clickedEntityId; }

private:
    std::string m_searchFilter;
    std::vector<EntityEntry> m_entities;

    // Most recently clicked entity id (-1 = none)
    int m_clickedEntityId = -1;

    // Hit-test geometry written during paint(), read during handleInput()
    struct EntityRowRect { int id; NF::Rect rect; };
    std::vector<EntityRowRect> m_rowRects;

    // Leading-edge click detection
    bool m_prevPrimaryDown = false;
};

} // namespace NF::UI::AtlasUI
