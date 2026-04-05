#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include <utility>

namespace NF::AtlasUI {

class PanelBase : public IPanel, public WidgetBase {
public:
    PanelBase(std::string panelId, std::string title)
        : m_panelId(std::move(panelId)), m_title(std::move(title)) {}

    [[nodiscard]] const char* panelId() const override { return m_panelId.c_str(); }
    [[nodiscard]] const char* title() const override { return m_title.c_str(); }

    void initialize() override { m_initialized = true; }
    void activate() override { m_active = true; }
    void deactivate() override { m_active = false; }

    [[nodiscard]] PanelState saveState() const override {
        PanelState state;
        state.panelId = m_panelId;
        state.title = m_title;
        state.visible = m_visible;
        state.active = m_active;
        state.bounds = m_bounds;
        return state;
    }

    void loadState(const PanelState& state) override {
        m_visible = state.visible;
        m_active = state.active;
        m_bounds = state.bounds;
        if (!state.title.empty()) {
            m_title = state.title;
        }
    }

    using WidgetBase::arrange;
    using WidgetBase::bounds;
    using WidgetBase::handleInput;
    using WidgetBase::isVisible;
    using WidgetBase::measure;
    using WidgetBase::paint;
    using WidgetBase::setVisible;

protected:
    std::string m_panelId;
    std::string m_title;
    bool m_initialized = false;
    bool m_active = false;
};

} // namespace NF::AtlasUI
