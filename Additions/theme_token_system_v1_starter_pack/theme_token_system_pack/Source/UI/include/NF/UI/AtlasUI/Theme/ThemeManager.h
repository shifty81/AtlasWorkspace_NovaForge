#pragma once
#include "Theme.h"

namespace NF::UI::AtlasUI
{
    class ThemeManager
    {
    public:
        static ThemeManager& Get();

        const AtlasTheme& Current() const;
        void SetTheme(const AtlasTheme& theme);

    private:
        ThemeManager();
        AtlasTheme m_current;
    };
}
