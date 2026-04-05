#include "NF/UI/AtlasUI/Commands/CommandHelpers.h"

namespace NF::UI::AtlasUI
{
    KeyChord MakeChord(int key, bool ctrl, bool alt, bool shift, bool win) noexcept
    {
        return KeyChord{ ctrl, alt, shift, win, key };
    }
}
