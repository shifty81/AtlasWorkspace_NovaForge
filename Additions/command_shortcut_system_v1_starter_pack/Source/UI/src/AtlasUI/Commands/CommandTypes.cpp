#include "NF/UI/AtlasUI/Commands/CommandTypes.h"

namespace NF::UI::AtlasUI
{
    std::string KeyChord::ToString() const
    {
        std::string out;
        if (Ctrl) out += "Ctrl+";
        if (Alt) out += "Alt+";
        if (Shift) out += "Shift+";
        if (Win) out += "Win+";
        if (Key >= 'A' && Key <= 'Z')
        {
            out.push_back(static_cast<char>(Key));
        }
        else
        {
            out += std::to_string(Key);
        }
        return out;
    }
}
