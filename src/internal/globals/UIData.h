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
        vector<UIObject*> allObjects;
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
            renderSet.clear();
            for (int i = 0; i < allObjects.size(); ++i) // Using fori to support deletions in the update methods
            {
                auto& obj = *allObjects[i];
                obj.onUpdate(obj.getBounds(), renderSet.contains(&obj));
            }
        }

        // All objects are registered in their ctor
        void registerObject(UIObject& object) { allObjects.push_back(&object); }

        // All objects are un-registered in the dtor
        void unregisterObject(const UIObject* object)
        {
            UnorderedDelete(allObjects, object);
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