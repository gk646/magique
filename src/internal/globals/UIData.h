#ifndef MAGIQUE_UIDATA_H
#define MAGIQUE_UIDATA_H

#include <raylib/raylib.h>

#include <magique/util/Defines.h>
#include <magique/ui/types/UIRoot.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/StringHashMap.h"
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
        uint16_t nextID = 1;                // unique ids
        vector<UIObject*> sortedObjects;    // Sorted after z-index
        StringHashMap<UIObject*> objectMap; // Stores by name
        UIObject* hoveredObject;            // Currently hovered object
        CursorAttachment cursorAttachment;  // Current object attached to the cursor

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
            auto** it = sortedObjects.begin();
            const auto* const end = sortedObjects.end();
            while (it != end)
            {
                if ((*it)->getZIndex() <= object->getZIndex())
                {
                    break;
                }
                ++it;
            }
            sortedObjects.insert(it, object);
        }

        void unregisterObject(UIObject* object)
        {
            if (sortedObjects.empty())
                return;
            sortedObjects.erase(object);
        }

        uint16_t getNextID() { return nextID++; }
    };

    namespace global
    {
        inline UIData UI_DATA;
    }
} // namespace magique

#endif //UIDATA_H