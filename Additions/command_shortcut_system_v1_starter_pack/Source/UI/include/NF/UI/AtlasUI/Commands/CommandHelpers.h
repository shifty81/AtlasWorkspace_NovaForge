#pragma once

#include "CommandTypes.h"

namespace NF::UI::AtlasUI
{
    [[nodiscard]] KeyChord MakeChord(int key, bool ctrl = false, bool alt = false, bool shift = false, bool win = false) noexcept;
}
