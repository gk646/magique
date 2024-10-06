#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"

namespace magique
{
    Point GetUIAnchor(const AnchorPosition anchor, const float width, const float height, const float inset)
    {
        Point point{};
        const auto res = global::UI_DATA.getScreenDims();
        switch (anchor)
        {
        case AnchorPosition::TOP_LEFT:
            point = {inset, inset};
            break;

        case AnchorPosition::TOP_CENTER:
            point = {(res.x - width) / 2.0F, inset};
            break;

        case AnchorPosition::TOP_RIGHT:
            point = {res.x - width - inset, inset};
            break;

        case AnchorPosition::MID_LEFT:
            point = {inset, res.y / 2 - height / 2};
            break;

        case AnchorPosition::MID_CENTER:
            point = {res.x / 2 - width / 2, res.y / 2 - height / 2};
            break;

        case AnchorPosition::MID_RIGHT:
            point = {res.x - width - inset, res.y / 2 - height / 2};
            break;

        case AnchorPosition::BOTTOM_LEFT:
            point = {inset, res.y - height - inset};
            break;

        case AnchorPosition::BOTTOM_CENTER:
            point = {res.x / 2 - width / 2, res.y - height - inset};
            break;

        case AnchorPosition::BOTTOM_RIGHT:
            point = {res.x - width - inset, res.y - height - inset};
            break;
        }
        return point;
    }

    float GetScaled(const float val) { return global::UI_DATA.scaleX * val; }

    Point GetUIScaling() { return global::UI_DATA.getScaling(); }

    Point GetMousePos()
    {
        return global::UI_DATA.getMousePos();
    }

    bool UIInput::IsKeyPressed(const int key) { return !global::UI_DATA.inputConsumed && ::IsKeyPressed(key); }

    bool UIInput::IsKeyDown(const int key) { return !global::UI_DATA.inputConsumed && ::IsKeyDown(key); }

    bool UIInput::IsMouseButtonPressed(const int key)
    {
        return !global::UI_DATA.inputConsumed && ::IsMouseButtonPressed(key);
    }

    bool UIInput::IsMouseButtonDown(const int key) { return !global::UI_DATA.inputConsumed && ::IsMouseButtonDown(key); }

    void UIInput::Consume() { global::UI_DATA.inputConsumed = true; }

    bool UIInput::IsConsumed() { return global::UI_DATA.inputConsumed; }

} // namespace magique