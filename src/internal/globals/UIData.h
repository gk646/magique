#ifndef MAGIQUE_UIDATA_H
#define MAGIQUE_UIDATA_H

#include <magique/util/Defines.h>
#include <magique/ui/types/UIObject.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/StringHashMap.h"
#include "external/raylib/src/coredata.h"

namespace magique
{
    struct StateData final
    {
        vector<UIObject*> sortedObjects[(int)UILayer::ONTOP + 1]; // Sorted after z-index

        void registerObject(UIObject* object, const int layer)
        {
            auto** it = sortedObjects[layer].begin();
            const auto* const end = sortedObjects[layer].end();
            while (it != end)
            {
                if ((*it)->getZIndex() <= object->getZIndex())
                {
                    break;
                }
                ++it;
            }
            sortedObjects[layer].insert(it, object);
        }

        void unregisterObject(UIObject* object, const int layer)
        {
            if (sortedObjects[layer].empty())
                return;
            sortedObjects[layer].erase(object);
        }
    };

    struct UIData final
    {
        StringHashMap<UIObject*> objectMap;      // Stores by name
        HashMap<GameState, StateData> stateData; // Different states - hashmap cause we dont know the user enum
        UIObject* hoveredObject = nullptr;       // Currently hovered object
        CursorAttachment cursorAttachment{};     // Current object attached to the cursor
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        float mouseX = 0.0F;
        float mouseY = 0.0F;
        uint16_t nextID = 1; // unique ids

        void update()
        {
            scaleX = MAGIQUE_UI_RESOLUTION_X / static_cast<float>(CORE.Window.display.width);
            scaleY = MAGIQUE_UI_RESOLUTION_Y / static_cast<float>(CORE.Window.display.height);
            const auto [mx, my] = GetMousePosition();
            mouseX = mx;
            mouseY = my;
        }

        void draw()
        {
            auto it = stateData.find(GetGameState());
            if (it == stateData.end())
                return;

            auto& data = it->second;

            for ()
        }

        StateData* getData()
        {
            const auto it = stateData.find(GetGameState());
            if (it == stateData.end())
                return nullptr;
            return &it->second;
        }

        [[nodiscard]] Point getScaling() const { return {scaleX, scaleY}; }

        [[nodiscard]] static Point getScreenDims()
        {
            return {static_cast<float>(CORE.Window.display.width), static_cast<float>(CORE.Window.display.height)};
        }

        [[nodiscard]] Point getMousePos() const { return {mouseX, mouseY}; }

        uint16_t getNextID() { return nextID++; }
    };

    namespace global
    {
        inline UIData UI_DATA;
    }
} // namespace magique

#endif //UIDATA_H