// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UI_DATA_H
#define MAGIQUE_UI_DATA_H

#include <cmath>
#include <algorithm>
#include <raylib/raylib.h>

#include <magique/ui/UIObject.h>
#include <magique/ui/UI.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/utils/STLUtil.h"

inline bool initialized = false;

namespace magique
{
    struct UIData final
    {
        vector<UIObject*> objects;
        vector<UIObject*> containers;
        HashSet<UIObject*> objectsSet;
        Point dragStart{-1, -1};
        Point targetRes{};
        Point sourceRes{1920, 1080};
        Point scaling{1.0F, 1.0F};
        bool keyConsumed = false;
        bool mouseConsumed = false;
        bool customTargetRes = false;
        bool showHitboxes = false;

        UIData() { initialized = true; }

        // Before each draw and update tick
        void updateBeginTick()
        {
            if (!customTargetRes)
            {
                targetRes.x = static_cast<float>(GetScreenWidth());
                targetRes.y = static_cast<float>(GetScreenHeight());
            }
            scaling = targetRes / sourceRes;
            const auto [mx, my] = GetMousePos();

            if (dragStart.x == -1 && dragStart.y == -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                dragStart = {mx, my};
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                dragStart = {-1, -1};
            }

            // Here we are doing the updates for next tick (instead of right at the end of the draw tick)
            // Using fori to support deletions in the update methods
            for (int i = 0; i < containers.size(); ++i)
            {
                auto& container = *containers[i];
                if (container.wasDrawnLastTick && !container.drawnThisTick)
                {
                    container.onHide(container.getBounds());
                }
                container.wasDrawnLastTick = container.drawnThisTick;
                container.drawnThisTick = false;
                container.onDrawUpdate(container.getBounds(), container.wasDrawnLastTick);
            }
            for (int i = 0; i < objects.size(); ++i)
            {
                auto& obj = *objects[i];
                if (obj.wasDrawnLastTick && !obj.drawnThisTick)
                {
                    obj.onHide(obj.getBounds());
                }
                obj.wasDrawnLastTick = obj.drawnThisTick;
                obj.drawnThisTick = false;
                obj.onDrawUpdate(obj.getBounds(), obj.wasDrawnLastTick);
            }
        }


        // Only before each update tick (if it happens)
        void update()
        {
            keyConsumed = false;
            mouseConsumed = false;

            // Using fori to support deletions in the update methods
            for (int i = 0; i < containers.size(); ++i)
            {
                auto& container = *containers[i];
                container.onUpdate(container.getBounds(), container.wasDrawnLastTick);
            }

            for (int i = 0; i < objects.size(); ++i)
            {
                auto& obj = *objects[i];
                obj.onUpdate(obj.getBounds(), obj.wasDrawnLastTick);
            }
        }

        // All objects are registered in their ctor
        void registerObject(UIObject* object, const bool isContainer = false)
        {
            if (!initialized)
            {
                *this = {};
                initialized = true;
            }

            objectsSet.insert(object);
            if (isContainer)
            {
                object->isContainer = true;
                containers.push_back(object);
                objects.pop_back(); // Is added as an object as well
            }
            else
            {
                objects.push_back(object);
            }
        }

        // All objects are un-registered in the dtor
        void unregisterObject(const UIObject* object) { UnorderedDelete(objects, object); }

        void registerDrawCall(UIObject* object, const bool isContainer)
        {
            if (!object->wasDrawnLastTick)
            {
                object->onShown(object->getBounds());
            }
            object->drawnThisTick = true;
            if (!objectsSet.contains(object)) [[unlikely]]
            {
                registerObject(object, object->isContainer);
            }

            // Moving 3 to the front
            // [1][2][3][4][5]
            // [3]              -> assign temporary
            // [0][1][2][4][5]  -> copy everything before 1 back
            // [3][1][2][4][5]
            // or using std::iterators ... :(

            auto sortUpfront = [](vector<UIObject*>& objects, UIObject* obj)
            {
                auto* it = std::ranges::find(objects, obj);
                objects.erase(it);
                objects.insert(objects.begin(), obj);
            };

            if (isContainer)
            {
                sortUpfront(containers, object);
            }
            else
            {
                sortUpfront(objects, object);
            }
        }

        void scaleBounds(Rectangle& bounds, const ScalingMode scaleMode, Point inset, Anchor anchor) const
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
            case ScalingMode::NONE:
                bounds.x *= sourceRes.x;
                bounds.y *= sourceRes.y;
                bounds.width *= sourceRes.x;
                bounds.height *= sourceRes.y;
                break;
            }
            if (anchor != Anchor::NONE)
            {
                auto scaledInset = UIGetScaled(inset);
                const auto pos = UIGetAnchor(anchor, Point{bounds.width, bounds.height}, scaledInset);
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

#endif //MAGIQUE_UI_DATA_H
