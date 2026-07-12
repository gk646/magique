// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UI_DATA_H
#define MAGIQUE_UI_DATA_H

#include <cmath>
#include <stack>
#include <algorithm>
#include <raylib/raylib.h>
#include <raylib/config.h>

#include <magique/ui/UIObject.h>
#include <magique/ui/UI.h>
#include <magique/util/Datastructures.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/controls/Popup.h>

#include "external/raylib-compat/rcore_compat.h"


namespace magique
{
    struct UIData final
    {
        std::vector<UIObject*> objects;
        std::vector<Popup*> popups;
        RenderTexture alphaTexture;
        UIObject* mouseConsumedAfter = nullptr; // Object after which the mouse was consumed
        HashSet<UIObject*> objectsSet;
        Point dragStart{-1, -1};
        Point targetRes{1920, 1080};
        Point sourceRes{1920, 1080};
        Point scaling{1.0F, 1.0F};
        GamepadMapping* currentMapping = nullptr;
        std::stack<GamepadMapping*> mappings;

        bool keyConsumed = false;
        bool mouseConsumed = false;
        bool customTargetRes = false;
        bool showHitboxes = false;
        bool usingGamepad = false;

        // Consumed status
        // Reset each tick (before render and update)
        // Consumes status is only set in onUpdate() (as its has nothign to do with drawing)
        // We save when it is set (at which object) to emulate the ordering for drawing
        //      => All objects drawn up to the object that consumed should be drawn with the consumed state
        // So we consume before drawing and clear it when the object is drawn that consumed in the first place
        // This is done for both onDraw() and onDrawUpdate()

        void resetConsumed()
        {
            keyConsumed = false;
            mouseConsumed = false;
        }

        void updateGamePadMapping() const
        {
            if (currentMapping == nullptr || !UIUsingGamepad())
                return;

            if (LayeredInput::IsKeyReleased(KEY_ENTER) ||
                LayeredInput::IsGamepadButtonReleased(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
            {
                TriggerMouseRelease(MOUSE_BUTTON_LEFT);
            }

            if (LayeredInput::IsKeyPressed(KEY_ENTER) ||
                LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::Submit);
            }
            else if (LayeredInput::IsKeyPressed(KEY_ESCAPE) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::Back);
            }
            else if (LayeredInput::IsKeyPressed(KEY_DOWN) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::Down);
            }
            else if (LayeredInput::IsKeyPressed(KEY_UP) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::Up);
            }
            else if (LayeredInput::IsKeyPressed(KEY_LEFT) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::Left);
            }
            else if (LayeredInput::IsKeyPressed(KEY_RIGHT) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::Right);
            }
            else if (LayeredInput::IsKeyPressed(KEY_Q) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::SwitchLeft);
            }
            else if (LayeredInput::IsKeyPressed(KEY_E) ||
                     LayeredInput::IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
            {
                currentMapping->triggerEvent(GamepadMappingEvent::SwitchRight);
            }
            else
            {
                for (int i = 0; i < MAX_GAMEPAD_BUTTONS; i++)
                {
                    if (LayeredInput::IsGamepadButtonPressed(0, i))
                    {
                        currentMapping->triggerEvent(GamepadMappingEvent::Button, static_cast<GamepadButton>(i));
                    }
                }
            }
        }

        // Before each draw and update tick
        void onEachTick()
        {
            if (!customTargetRes)
            {
                targetRes = GetScreenDims();
            }
            scaling = targetRes / sourceRes;
            const auto [mx, my] = GetMousePos();

            if (dragStart == -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                dragStart = {mx, my};
            }

            // Here we are doing the updates for next tick (instead of right at the end of the draw tick)
            // Using fori to support deletions in the update methods
            for (size_t i = 0; i < objects.size(); ++i)
            {
                auto& obj = *objects[i];
                if (obj.wasDrawnLastTick && !obj.drawnThisTick)
                    obj.onHide(obj.getBounds());

                obj.wasDrawnLastTick = obj.drawnThisTick;
                obj.drawnThisTick = false;
            }

            resetConsumed();
        }

        void detectGamepad()
        {
            const bool prevStat = usingGamepad;
            if (usingGamepad)
            {
                //  GetKeyPressedQueueCount() == 0 &&
                usingGamepad = Point{GetMouseDelta()} == 0;
            }
            else
            {
                bool anyButton = std::memcmp(GetCurrentGamepadState(), GetPreviousGamepadState(),
                                             MAX_GAMEPADS * MAX_GAMEPAD_BUTTONS) != 0;
                bool anyAxis = GetGamePadLeftStick(0, 0.3) != 0 || GetGamePadRightStick(0, 0.3) != 0;
                usingGamepad = anyButton || anyAxis;
            }

            if (prevStat != usingGamepad)
            {
                keyConsumed = false;
                mouseConsumed = false;
                if (usingGamepad)
                {
                    HideCursor();
                }
                else
                {
                    EnableCursor();
                }
            }
        }

        // Only before each update tick (if it happens)
        void onUpdateTick()
        {
            mouseConsumedAfter = nullptr;

            detectGamepad();

            // Using fori to support deletions in the update methods
            if (currentMapping != nullptr)
                updateGamePadMapping();


            for (size_t i = 0; i < objects.size(); ++i)
            {
                auto& obj = *objects[i];

                obj.onUpdate(obj.getBounds(), obj.wasDrawnLastTick);
                if (mouseConsumedAfter == nullptr && LayeredInput::GetIsMouseConsumed())
                    mouseConsumedAfter = &obj;
            }

            // After the tick
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                dragStart = {-1, -1};
            }
        }

        void onRenderTick()
        {
            if (mouseConsumedAfter != nullptr)
                mouseConsumed = true;
        }

        // All objects are registered in their ctor
        void registerObject(UIObject* object)
        {
            objectsSet.insert(object);
            objects.push_back(object);
        }

        // All objects are un-registered in the dtor
        void unregisterObject(UIObject* object) { std::erase(objects, object); }

        void registerDrawCall(UIObject* object)
        {
            if (object == mouseConsumedAfter)
                mouseConsumed = false;

            if (!object->wasDrawnLastTick)
            {
                object->onShown(object->getBounds());
            }

            object->drawnThisTick = true;

            if (!objectsSet.contains(object)) [[unlikely]]
            {
                registerObject(object);
            }

            const auto it = std::ranges::find(objects, object);
            if (it != objects.end())
            {
                objects.erase(it);
                objects.insert(objects.begin(), object);
            }
        }

        void scaleBounds(Rect& bounds, const ScalingMode scaleMode, Point inset, Anchor anchor) const
        {
            const auto& [sx, sy] = targetRes;
            switch (scaleMode)
            {
            case ScalingMode::FULL:
                bounds.x *= sx;
                bounds.y *= sy;
                bounds.width *= sx;
                bounds.height *= sy;
                break;
            case ScalingMode::KEEP_RATIO:
                bounds.x *= sx;
                bounds.y *= sy;
                bounds.width = bounds.width * sourceRes.x / sourceRes.y * sy;
                bounds.height *= sy;
                break;
            case ScalingMode::ABSOLUTE:
                break;
            }
            if (anchor != Anchor::NONE)
            {
                const auto scaledInset = UIGetScaled(inset);
                const auto pos = UIGetAnchor(anchor, bounds.size(), scaledInset);
                bounds.x = pos.x;
                bounds.y = pos.y;
            }
            // Floating points...
            bounds.x = std::floor(bounds.x + 0.01F);
            bounds.y = std::floor(bounds.y + 0.01F);
            bounds.width = std::floor(bounds.width + 0.01F);
            bounds.height = std::floor(bounds.height + 0.01F);
        }
    };

    namespace global
    {
        inline UIData UI_DATA{};
    }
} // namespace magique

#endif // MAGIQUE_UI_DATA_H
