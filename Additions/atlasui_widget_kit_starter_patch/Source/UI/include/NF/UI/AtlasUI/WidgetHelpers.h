#pragma once

#include "NF/Core/Core.h"
#include <algorithm>

namespace NF::AtlasUI {

inline Rect insetRect(const Rect& rect, float amount) {
    return {rect.x + amount, rect.y + amount, std::max(0.f, rect.w - amount * 2.f), std::max(0.f, rect.h - amount * 2.f)};
}

inline Rect insetRect(const Rect& rect, float horizontal, float vertical) {
    return {rect.x + horizontal, rect.y + vertical,
            std::max(0.f, rect.w - horizontal * 2.f), std::max(0.f, rect.h - vertical * 2.f)};
}

inline Rect makeRect(float x, float y, float w, float h) {
    return {x, y, w, h};
}

inline bool rectContains(const Rect& rect, Vec2 point) {
    return rect.contains(point.x, point.y);
}

} // namespace NF::AtlasUI
