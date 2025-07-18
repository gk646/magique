// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"

namespace magique
{
    Point GetUIAnchor(const Anchor anchor, const float width, const float height, const float inset)
    {
        Point point{};
        const auto res = global::UI_DATA.getTargetResolution();
        switch (anchor)
        {
        case Anchor::TOP_LEFT:
            point = {inset, inset};
            break;

        case Anchor::TOP_CENTER:
            point = {(res.x - width) / 2.0F, inset};
            break;

        case Anchor::TOP_RIGHT:
            point = {res.x - width - inset, inset};
            break;

        case Anchor::MID_LEFT:
            point = {inset, res.y / 2 - height / 2};
            break;

        case Anchor::MID_CENTER:
            point = {res.x / 2 - width / 2, res.y / 2 - height / 2};
            break;

        case Anchor::MID_RIGHT:
            point = {res.x - width - inset, res.y / 2 - height / 2};
            break;

        case Anchor::BOTTOM_LEFT:
            point = {inset, res.y - height - inset};
            break;

        case Anchor::BOTTOM_CENTER:
            point = {res.x / 2 - width / 2, res.y - height - inset};
            break;

        case Anchor::BOTTOM_RIGHT:
            point = {res.x - width - inset, res.y - height - inset};
            break;
        case Anchor::NONE:
            MAGIQUE_ASSERT(false, "Invalid anchor");
            break;
        }
        return point;
    }

    float GetScaled(const float val) { return global::UI_DATA.scaleY * val; }

    Point GetUIScaling() { return global::UI_DATA.getScaling(); }

    Point GetDragStartPosition() { return global::UI_DATA.dragStart; }

    Point GetMousePos() { return global::UI_DATA.getMousePos(); }

    void SetUITargetResolution(float width, float height) { global::UI_DATA.resolution = {width, height}; }

    bool UIInput::IsKeyPressed(const int key) { return !global::UI_DATA.inputConsumed && ::IsKeyPressed(key); }

    bool UIInput::IsKeyDown(const int key) { return !global::UI_DATA.inputConsumed && ::IsKeyDown(key); }

    bool UIInput::IsKeyReleased(const int key) { return !global::UI_DATA.inputConsumed && ::IsKeyReleased(key); }

    bool UIInput::IsMouseButtonPressed(const int key)
    {
        return !global::UI_DATA.inputConsumed && ::IsMouseButtonPressed(key);
    }

    bool UIInput::IsMouseButtonDown(const int key) { return !global::UI_DATA.inputConsumed && ::IsMouseButtonDown(key); }

    void UIInput::Consume() { global::UI_DATA.inputConsumed = true; }

    bool UIInput::GetIsConsumed() { return global::UI_DATA.inputConsumed; }

} // namespace magique