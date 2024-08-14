#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"

namespace magique
{
    Point GetUIAnchor(const AnchorPosition anchor, const float width, const float height, const float inset)
    {
        Point point{};
        switch (anchor)
        {
        case AnchorPosition::TOP_LEFT:
            point = {inset, inset};
            break;
        case AnchorPosition::TOP_CENTER:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, inset};
            break;
        case AnchorPosition::TOP_RIGHT:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, inset};
            break;
        case AnchorPosition::MID_LEFT:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::MID_CENTER:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::MID_RIGHT:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::BOTTOM_LEFT:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::BOTTOM_CENTER:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::BOTTOM_RIGHT:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        }
        const auto [sx, sy] = global::UI_DATA.getScaling();
        point.x *= sx;
        point.y *= sy;
        return point;
    }

    float GetScaled(const float val) { return global::UI_DATA.scaleX * val; }

    Point GetUIScaling() { return global::UI_DATA.getScaling(); }

    bool UIInput::IsKeyPressed(const int key) { return global::UI_DATA.inputConsumed && IsKeyPressed(key); }
    bool UIInput::IsKeyDown(const int key) { return global::UI_DATA.inputConsumed && IsKeyDown(key); }
    bool UIInput::IsKeyUp(const int key) { return global::UI_DATA.inputConsumed && IsKeyUp(key); }

} // namespace magique