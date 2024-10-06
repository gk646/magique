#ifndef MAGIQUE_UI_DATA_H
#define MAGIQUE_UI_DATA_H

#include <raylib/raylib.h>

#include <magique/ui/UIObject.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"
#include "external/raylib/src/coredata.h"
#include "internal/utils/STLUtil.h"

namespace magique
{
    struct UIData final
    {
        vector<UIObject*> objects;
        vector<UIObject*> containers;
        HashSet<UIObject*> objectsSet;
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        float mouseX = 0.0F;
        float mouseY = 0.0F;
        int dataIndex = 0;
        bool inputConsumed = false;

        // Called at the end of the update tick
        void update()
        {
            scaleX = MAGIQUE_UI_RESOLUTION_X / static_cast<float>(CORE.Window.screen.width);
            scaleY = MAGIQUE_UI_RESOLUTION_Y / static_cast<float>(CORE.Window.screen.height);
            const auto [mx, my] = GetMousePosition();
            mouseX = mx;
            mouseY = my;
            inputConsumed = false;

            // Using fori to support deletions in the update methods
            for (int i = 0; i < containers.size(); ++i)
            {
                auto& container = *containers[i];
                container.onUpdate(container.getBounds(), container.wasDrawn);
                container.wasDrawn = false;
            }

            for (int i = 0; i < objects.size(); ++i)
            {
                auto& obj = *objects[i];
                obj.onUpdate(obj.getBounds(), obj.wasDrawn);
                obj.wasDrawn = false;
            }
        }

        // All objects are registered in their ctor
        void registerObject(UIObject* object, const bool isContainer = false)
        {
            objectsSet.insert(object);
            if (isContainer)
            {
                containers.push_back(object);
                objects.pop_back(); // Is added as an object as well
            }
            else
                objects.push_back(object);
        }

        // All objects are un-registered in the dtor
        void unregisterObject(const UIObject* object) { UnorderedDelete(objects, object); }

        void registerDrawCall(UIObject* object, const bool isContainer)
        {
            object->wasDrawn = true;
            if (!objectsSet.contains(object)) [[unlikely]]
                registerObject(object, object->isContainer);

            // Moving 3 to the front
            // [1][2][3][4][5]
            // [3]              -> assign temporary
            // [0][1][2][4][5]  -> copy everything before 1 back
            // [3][1][2][4][5]
            // or using std:: iterators ... :(

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

        //----------------- UTIL -----------------//

        [[nodiscard]] Point getScaling() const { return {scaleX, scaleY}; }

        [[nodiscard]] static Point getScreenDims()
        {
            return {static_cast<float>(CORE.Window.screen.width), static_cast<float>(CORE.Window.screen.height)};
        }

        [[nodiscard]] Point getMousePos() const { return {mouseX, mouseY}; }
    };

    namespace global
    {
        inline UIData UI_DATA{};
    }
} // namespace magique

#endif //MAGIQUE_UI_DATA_H