#pragma once

#include "CommandRegistry.h"

namespace NF::UI::AtlasUI
{
    class ShortcutRouter
    {
    public:
        explicit ShortcutRouter(const CommandRegistry& registry);

        [[nodiscard]] bool TryExecute(const KeyChord& chord, const CommandContext& context) const;

    private:
        const CommandRegistry& m_registry;
    };
}
