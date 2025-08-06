// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"

namespace magique
{
    Point GetUIAnchor(const Anchor anchor, const float width, const float height, const float inset)
    {
        Point point{};
        const auto res = global::UI_DATA.targetRes;
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

    float GetScaled(const float val) { return global::UI_DATA.scaling.y * val; }

    Point GetUIScaling() { return global::UI_DATA.scaling; }

    Point GetDragStartPosition() { return global::UI_DATA.dragStart; }

    Point GetMousePos()
    {
        const auto mouse = GetMousePosition();
        return Point{std::round(mouse.x), std::round(mouse.y)};
    }

    void SetUITargetResolution(float width, float height) { global::UI_DATA.targetRes = {width, height}; }

    bool LayeredInput::IsKeyPressed(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyPressed(key); }

    bool LayeredInput::IsKeyDown(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyDown(key); }

    bool LayeredInput::IsKeyReleased(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyReleased(key); }

    bool LayeredInput::IsMouseButtonPressed(const int key)
    {
        return !global::UI_DATA.mouseConsumed && ::IsMouseButtonPressed(key);
    }

    bool LayeredInput::IsMouseButtonDown(const int key)
    {
        return !global::UI_DATA.mouseConsumed && ::IsMouseButtonDown(key);
    }

    bool LayeredInput::IsMouseButtonReleased(const int key)
    {
        return !global::UI_DATA.mouseConsumed && ::IsMouseButtonReleased(key);
    }

    void LayeredInput::ConsumeKey() { global::UI_DATA.keyConsumed = true; }

    void LayeredInput::ConsumeMouse() { global::UI_DATA.mouseConsumed = true; }

    bool LayeredInput::GetIsKeyConsumed() { return global::UI_DATA.keyConsumed; }

    bool LayeredInput::GetIsMouseConsumed() { return global::UI_DATA.mouseConsumed; }

    void SetUISourceResolution(float width, float height) { global::UI_DATA.sourceRes = {width, height}; }
} // namespace magique