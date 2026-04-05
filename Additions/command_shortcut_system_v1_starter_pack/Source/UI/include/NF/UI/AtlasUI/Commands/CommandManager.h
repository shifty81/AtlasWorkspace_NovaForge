#pragma once

#include "ShortcutRouter.h"

namespace NF::UI::AtlasUI
{
    class CommandManager
    {
    public:
        static CommandManager& Get();

        CommandRegistry& Registry() noexcept;
        const CommandRegistry& Registry() const noexcept;

        ShortcutRouter& Router() noexcept;
        const ShortcutRouter& Router() const noexcept;

        void RebuildRouter();

    private:
        CommandManager();

        CommandRegistry m_registry;
        ShortcutRouter m_router;
    };
}
