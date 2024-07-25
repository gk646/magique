#ifndef UIDATA_H
#define UIDATA_H

#include <vector>
#include <algorithm>
#include <raylib/raylib.h>

#include <magique/util/Defines.h>
#include <magique/ui/types/UIRoot.h>

#include "external/raylib/src/coredata.h"

namespace magique
{
    struct UIData final
    {
        UIRoot uiRoot;
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        float mouseX = 0.0F;
        float mouseY = 0.0F;
        uint64_t nextID = 1;
        std::vector<UIObject*> sortedObjects; // Sorted after z-index

        void update()
        {
            scaleX = MAGIQUE_UI_RESOLUTION_X / static_cast<float>(CORE.Window.display.width);
            scaleY = MAGIQUE_UI_RESOLUTION_Y / static_cast<float>(CORE.Window.display.height);
            const auto [mx, my] = GetMousePosition();
            mouseX = mx;
            mouseY = my;
        }

        [[nodiscard]] Point getScaling() const { return {scaleX, scaleY}; }


        [[nodiscard]] static Point getScreenDims()
        {
            return {static_cast<float>(CORE.Window.display.width), static_cast<float>(CORE.Window.display.height)};
        }

        [[nodiscard]] Point getMousePos() const { return {mouseX, mouseY}; }

        void registerObject(UIObject* object)
        {
            if (sortedObjects.empty())
            {
                sortedObjects.push_back(object);
                return;
            }
            const auto pred = [](const UIObject* ob1, const UIObject* ob2)
            { return ob1->getZIndex() > ob2->getZIndex(); };
            const auto it = std::ranges::lower_bound(sortedObjects, object, pred);
            sortedObjects.insert(it, object);
        }

        void unregisterObject(UIObject* object)
        {
            if (sortedObjects.empty())
                return;
            std::erase(sortedObjects, object);
        }

        uint16_t getNextID() { return nextID++; }
    };

    namespace global
    {
        inline UIData UI_DATA;
    }
} // namespace magique

#endif //UIDATA_H