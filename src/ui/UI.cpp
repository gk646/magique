// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>
#include "internal/globals/UIData.h"

namespace magique
{
    Point GetUIAnchor(const Anchor anchor, const float width, const float height, const Point inset)
    {
        Point point{};
        const auto res = global::UI_DATA.targetRes;
        switch (anchor)
        {
        case Anchor::TOP_LEFT:
            point = {inset.x, inset.y};
            break;

        case Anchor::TOP_CENTER:
            point = {(res.x - width) / 2.0F, inset.y};
            break;

        case Anchor::TOP_RIGHT:
            point = {res.x - width - inset.x, inset.y};
            break;

        case Anchor::MID_LEFT:
            point = {inset.x, res.y / 2 - height / 2};
            break;

        case Anchor::MID_CENTER:
            point = {res.x / 2 - width / 2, res.y / 2 - height / 2};
            break;

        case Anchor::MID_RIGHT:
            point = {res.x - width - inset.x, res.y / 2 - height / 2};
            break;

        case Anchor::BOTTOM_LEFT:
            point = {inset.x, res.y - height - inset.y};
            break;

        case Anchor::BOTTOM_CENTER:
            point = {res.x / 2 - width / 2, res.y - height - inset.y};
            break;

        case Anchor::BOTTOM_RIGHT:
            point = {res.x - width - inset.x, res.y - height - inset.y};
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
        return Point{GetMousePosition()}.floor();
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

    Rectangle GetDynamicRectAtMouse(const Point& offset, float width, float height)
    {
        const auto& mouse = GetMousePos();
        Rectangle rect = {mouse.x + offset.x, mouse.y + offset.y, width, height};

        // Outside horizontally
        if (rect.x + rect.width > global::UI_DATA.targetRes.x)
        {
            rect.x -= rect.width + offset.x;
        }
        else if (rect.x < 0)
        {
            rect.x += rect.width + offset.x;
        }

        // Outside vertically
        if (rect.y + rect.height > global::UI_DATA.targetRes.y)
        {
            rect.y -= rect.height + offset.y;
        }
        else if (rect.y < 0)
        {
            rect.y += rect.height - (rect.height + offset.y);
        }
        return rect;
    }

} // namespace magique