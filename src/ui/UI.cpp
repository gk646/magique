// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>

#include "internal/globals/UIData.h"
#include "magique/core/Camera.h"
#include "external/raylib-compat/rcore_compat.h"

namespace magique
{

    static Point UIGetAnchor(const Anchor anchor, Point topLeft, Point bounds, Point size, Point inset)
    {
        Point point = topLeft;
        switch (anchor)
        {
        case Anchor::TOP_LEFT:
            point = {topLeft.x + inset.x, topLeft.y + inset.y};
            break;
        case Anchor::TOP_CENTER:
            point = {topLeft.x + (bounds.x - size.x) / 2, topLeft.y + inset.y};
            break;
        case Anchor::TOP_RIGHT:
            point = {topLeft.x + bounds.x - size.x - inset.x, topLeft.y + inset.y};
            break;
        case Anchor::MID_LEFT:
            point = {topLeft.x + inset.x, topLeft.y + (bounds.y - size.y) / 2};
            break;
        case Anchor::MID_CENTER:
            point = {topLeft.x + (bounds.x - size.x) / 2, topLeft.y + (bounds.y - size.y) / 2};
            break;
        case Anchor::MID_RIGHT:
            point = {topLeft.x + bounds.x - size.x - inset.x, topLeft.y + (bounds.y - size.y) / 2};
            break;
        case Anchor::BOTTOM_LEFT:
            point = {topLeft.x + inset.x, topLeft.y + bounds.y - size.y - inset.y};
            break;
        case Anchor::BOTTOM_CENTER:
            point = {topLeft.x + (bounds.x - size.x) / 2, topLeft.y + bounds.y - size.y - inset.y};
            break;
        case Anchor::BOTTOM_RIGHT:
            point = {topLeft.x + bounds.x - size.x - inset.x, topLeft.y + bounds.y - size.y - inset.y};
            break;
        case Anchor::NONE:
            MAGIQUE_ASSERT(false, "Invalid anchor");
            break;
        }
        return point;
    }


    Point UIGetAnchor(const Anchor anchor, const Point size, const Point inset)
    {
        return UIGetAnchor(anchor, {}, global::UI_DATA.targetRes, size, inset);
    }

    Point UIGetAnchor(Anchor anchor, const UIObject& relative, const Point size, const Point inset)
    {
        const auto bounds = relative.getBounds();
        return UIGetAnchor(anchor, {bounds.x, bounds.y}, {bounds.width, bounds.height}, size, inset);
    }

    float UIGetScaled(const float val) { return global::UI_DATA.scaling.y * val; }

    Point UIGetScaled(Point p) { return p * global::UI_DATA.scaling; }

    Point UIGetScaling() { return global::UI_DATA.scaling; }

    Point GetDragStartPosition() { return global::UI_DATA.dragStart; }

    void UISetSourceResolution(Point resolution) { global::UI_DATA.sourceRes = resolution; }

    Point GetMousePos() { return Point{GetMousePosition()}.floor(); }

    Point GetWorldMousePos() { return GetScreenToWorld2D(GetMousePosition(), CameraGet()); }

    void UISetTargetResolution(Point resolution)
    {
        if (resolution == 0)
        {
            global::UI_DATA.customTargetRes = false;
        }
        else
        {
            global::UI_DATA.targetRes = resolution;
            global::UI_DATA.customTargetRes = true;
        }
    }

    Point UIGetTargetResolution() { return global::UI_DATA.targetRes; }

    bool LayeredInput::IsKeyPressed(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyPressed(key); }

    bool LayeredInput::IsKeyDown(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyDown(key); }

    bool LayeredInput::IsKeyReleased(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyReleased(key); }

    bool LayeredInput::IsKeyPressedRepeat(int key) { return !global::UI_DATA.keyConsumed && ::IsKeyPressedRepeat(key); }

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

    bool LayeredInput::IsGamepadButtonPressed(int gamepad, int key)
    {
        return !global::UI_DATA.mouseConsumed && ::IsGamepadButtonPressed(gamepad, key);
    }

    bool LayeredInput::IsGamepadButtonDown(int gamepad, int key)
    {
        return !global::UI_DATA.mouseConsumed && ::IsGamepadButtonDown(gamepad, key);
    }

    bool LayeredInput::IsGamepadButtonReleased(int gamepad, int key)
    {
        return !global::UI_DATA.mouseConsumed && ::IsGamepadButtonReleased(gamepad, key);
    }

    void LayeredInput::ConsumeKey() { global::UI_DATA.keyConsumed = true; }

    void LayeredInput::ConsumeMouse() { global::UI_DATA.mouseConsumed = true; }

    bool LayeredInput::GetIsKeyConsumed() { return global::UI_DATA.keyConsumed; }

    bool LayeredInput::GetIsMouseConsumed() { return global::UI_DATA.mouseConsumed; }

    void UISetShowHitboxes(const bool value) { global::UI_DATA.showHitboxes = value; }

    UIMouseToWorld::UIMouseToWorld()
    {
        prev = GetMousePosition();
        auto worldMouse = GetScreenToWorld2D(prev.v(), magique::CameraGet());
        SetMousePositionDirect(worldMouse.x, worldMouse.y);
    }

    UIMouseToWorld::~UIMouseToWorld() { SetMousePositionDirect(prev.x, prev.y); }

    Rectangle GetRectOnScreen(const Point& offset, float width, float height, Point base)
    {
        Rectangle rect = {base.x + offset.x, base.y + offset.y, width, height};

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
