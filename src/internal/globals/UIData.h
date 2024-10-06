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
        HashSet<const UIObject*> renderSet;
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
                auto& container = *objects[i];
                container.onUpdate(container.getBounds(), renderSet.contains(&container));
            }

            for (int i = 0; i < objects.size(); ++i)
            {
                auto& obj = *objects[i];
                obj.onUpdate(obj.getBounds(), renderSet.contains(&obj));
            }
            renderSet.clear();
        }

        // All objects are registered in their ctor
        void registerObject(UIObject* object, const bool isContainer = false)
        {
            if (isContainer)
            {
                containers.push_back(object);
                objects.pop_back(); // Is added as an object as well
            }
            else
                objects.push_back(object);
        }

        // All objects are un-registered in the dtor
        void unregisterObject(const UIObject* object)
        {
            UnorderedDelete(objects, object);
            renderSet.erase(object);
        }

        void registerDrawCall(const UIObject* object) { renderSet.insert(object); }

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