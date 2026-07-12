// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>
#include <magique/ui/Menu.h>
#include <magique/core/Camera.h>

#include "internal/globals/UIData.h"
#include "external/raylib-compat/rcore_compat.h"

namespace magique
{

    Point UIGetAnchor(Anchor anchor, const UIObject& relative, const Point size, const Point inset)
    {
        return UIGetAnchor(anchor, relative.getBounds(), size, inset);
    }

    Point UIGetAnchor(Anchor anchor, const Rect& relative, Point size, Point inset)
    {
        Point pos = relative.pos();
        switch (anchor)
        {
        case Anchor::TOP_LEFT:
            pos += inset;
            break;
        case Anchor::TOP_CENTER:
            pos.x += (relative.width - size.x) / 2.0F + inset.x;
            pos.y += inset.y;
            break;
        case Anchor::TOP_RIGHT:
            pos.x += (relative.width - size.x) - inset.x;
            pos.y += inset.y;
            break;
        case Anchor::MID_LEFT:
            pos.x += inset.x;
            pos.y += (relative.height - size.y) / 2.0F + inset.y;
            break;
        case Anchor::MID_CENTER:
            pos.x += (relative.width - size.x) / 2.0F + inset.x;
            pos.y += (relative.height - size.y) / 2.0F + inset.y;
            break;
        case Anchor::MID_RIGHT:
            pos.x += (relative.width - size.x) - inset.x;
            pos.y += (relative.height - size.y) / 2.0F + inset.y;
            break;
        case Anchor::BOTTOM_LEFT:
            pos.x += inset.x;
            pos.y += relative.height - size.y - inset.y;
            break;
        case Anchor::BOTTOM_CENTER:
            pos.x += (relative.width - size.x) / 2.0F + inset.x;
            pos.y += relative.height - size.y - inset.y;
            break;
        case Anchor::BOTTOM_RIGHT:
            pos.x += (relative.width - size.x) - inset.x;
            pos.y += relative.height - size.y - inset.y;
            break;
        case Anchor::NONE:
            break;
        }
        return pos;
    }

    Point UIGetAnchor(const Anchor anchor, const Point size, const Point inset)
    {
        return UIGetAnchor(anchor, Rect{global::UI_DATA.targetRes}, size, inset);
    }

    float UIGetScaled(const float val) { return global::UI_DATA.scaling.y * val; }

    Point UIGetScaled(Point p) { return p * global::UI_DATA.scaling; }

    Point UIGetScaling() { return global::UI_DATA.scaling; }

    Point UIGetDragStart() { return global::UI_DATA.dragStart; }

    void UISetSourceResolution(Point resolution) { global::UI_DATA.sourceRes = resolution; }

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

    void UIDrawPopups()
    {
        if (!global::UI_DATA.popups.empty())
        {
            auto& popup = *global::UI_DATA.popups.front();
            popup.draw();
        }
    }

    void UIAddPopup(Popup& popup)
    {
        auto& popups = global::UI_DATA.popups;
        auto it = std::ranges::find(popups, &popup);
        if (it != popups.end())
        {
            popups.erase(it);
            popups.insert(popups.begin(), &popup);
        }
        else
        {
            popups.insert(popups.begin(), &popup);
        }
        global::UI_DATA.popups.push_back(&popup);
    }

    bool UIRemovePopup(Popup& popup) { return std::erase(global::UI_DATA.popups, &popup) > 0; }

    const std::vector<Popup*>& UIGetPopups() { return global::UI_DATA.popups; }

    Point GetWorldMousePos() { return GetScreenToWorld2D(GetMousePosition(), CameraGet()); }

    void UIShowHitboxes(const bool value) { global::UI_DATA.showHitboxes = value; }

    bool UIUsingGamepad() { return global::UI_DATA.usingGamepad; }

    Rectangle UIGetRectOnScreen(const Point& offset, float width, float height, Point base)
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

    bool LayeredInput::IsKeyPressed(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyPressed(key); }

    bool LayeredInput::IsKeyDown(const int key) { return !global::UI_DATA.keyConsumed && ::IsKeyDown(key); }

    bool LayeredInput::IsKeyReleased(const int key) { return global::UI_DATA.keyConsumed || ::IsKeyReleased(key); }

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
        return !global::UI_DATA.keyConsumed && ::IsGamepadButtonPressed(gamepad, key);
    }

    bool LayeredInput::IsGamepadButtonDown(int gamepad, int key)
    {
        return !global::UI_DATA.keyConsumed && ::IsGamepadButtonDown(gamepad, key);
    }

    bool LayeredInput::IsGamepadButtonReleased(int gamepad, int key)
    {
        return global::UI_DATA.keyConsumed || ::IsGamepadButtonReleased(gamepad, key);
    }

    void LayeredInput::ConsumeKey() { global::UI_DATA.keyConsumed = true; }

    void LayeredInput::ConsumeMouse() { global::UI_DATA.mouseConsumed = true; }

    bool LayeredInput::GetIsKeyConsumed() { return global::UI_DATA.keyConsumed; }

    bool LayeredInput::GetIsMouseConsumed() { return global::UI_DATA.mouseConsumed; }

    void UIEmitMousePress(MouseButton button, Point mouse)
    {
        if (mouse != -1)
        {
            mouse *= Point{1} / GetMouseScale();
            SetMousePosition(mouse.x, mouse.y);
        }
        TriggerMousePress(button);
    }

    GamepadMapping::GamepadMapping(UIObject& object, const GamepadMappingFunc& func) : object(&object), func(func)
    {
        state_ = {};
    }

    void GamepadMapping::reset() { state_ = {}; }

    void GamepadMapping::setOnEvent(const GamepadMappingFunc& newFunc) { func = newFunc; }

    const GamepadMappingState& GamepadMapping::getState() const { return state_; }

    void GamepadMapping::triggerEvent(GamepadMappingEvent event, GamepadButton button)
    {
        if (event == GamepadMappingEvent::Start)
            reset();

        state_.event = event;
        Point pos = -1;
        state_.backConsumed = false;
        state_.submitConsumed = false;

        if (func)
            pos = func(state_, button);

        if (event == GamepadMappingEvent::Submit && !state_.submitConsumed)
            UIEmitMousePress(MOUSE_BUTTON_LEFT, pos);
        if (event == GamepadMappingEvent::Back && !state_.backConsumed)
        {
            UISetPreviousGamepadMap();
        }
        setMouse(pos);
    }

    UIObject& GamepadMapping::getObject() { return *object; }

    void GamepadMapping::setMouse(Point pos)
    {
        if (pos != -1)
        {
            pos = pos * (Point{1} / GetMouseScale());
            SetMousePosition(pos.x, pos.y);
        }
    }

    void UISetGamepadMap(GamepadMapping* map, bool resetStack)
    {
        if (map == nullptr)
            return;
        auto& data = global::UI_DATA;

        if (resetStack)
            while (!data.mappings.empty())
                data.mappings.pop();

        data.mappings.push(map);
        data.currentMapping = map;
        data.currentMapping->triggerEvent(GamepadMappingEvent::Start);
    }

    void UISetPreviousGamepadMap()
    {
        auto& data = global::UI_DATA;
        if (data.mappings.size() <= 1)
            return;

        data.mappings.pop();
        data.currentMapping = data.mappings.top();
    }

    GamepadMapping* UIGetGamepadMap() { return global::UI_DATA.currentMapping; }

    MouseToWorld::MouseToWorld()
    {
        prev = GetMousePosition();
        auto worldMouse = GetScreenToWorld2D(prev, CameraGet());
        SetMousePositionDirect(worldMouse.x, worldMouse.y);
    }

    MouseToWorld::~MouseToWorld() { SetMousePositionDirect(prev.x, prev.y); }

} // namespace magique
