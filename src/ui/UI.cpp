#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"

namespace magique
{
    Point GetUIAnchor(const AnchorPosition anchor, const float width, const float height, const float inset)
    {
        Point point{};
        switch (anchor)
        {
        case AnchorPosition::LEFT_TOP:
            point = {inset, inset};
            break;
        case AnchorPosition::CENTER_TOP:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, inset};
            break;
        case AnchorPosition::RIGHT_TOP:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, inset};
            break;
        case AnchorPosition::LEFT_MID:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::CENTER_MID:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::RIGHT_MID:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::LEFT_BOTTOM:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::CENTER_BOTTOM:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::RIGHT_BOTTOM:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        }
        const auto [sx, sy] = global::UI_DATA.getScaling();
        point.x *= sx;
        point.y *= sy;
        return point;
    }

    float GetScaled(const float val)
    {
        return global::UI_DATA.scaleX * val;
    }

    UIStateRoot& GetUIRoot() { return global::UI_DATA.uiRoot; }

    Point GetUIScaling() { return global::UI_DATA.getScaling(); }


} // namespace magique