#ifndef MAGIQUE_UIDATA_H
#define MAGIQUE_UIDATA_H

#include <raylib/raylib.h>

#include <magique/util/Defines.h>
#include <magique/ui/types/UIObject.h>
#include <magique/core/Core.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/StringHashMap.h"
#include "external/raylib/src/coredata.h"

namespace magique
{
    struct UIData final
    {
        vector<UIObject*> renderObjects;
        StringHashMap<UIScene*> sceneMap;  // Stores by name
        UIObject* hoveredObject = nullptr; // Currently hovered object
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        float mouseX = 0.0F;
        float mouseY = 0.0F;
        int dataIndex = 0;
        bool inputConsumed = false;

        void update()
        {
            scaleX = MAGIQUE_UI_RESOLUTION_X / static_cast<float>(CORE.Window.display.width);
            scaleY = MAGIQUE_UI_RESOLUTION_Y / static_cast<float>(CORE.Window.display.height);
            const auto [mx, my] = GetMousePosition();
            mouseX = mx;
            mouseY = my;
            inputConsumed = false;
        }

        [[nodiscard]] Point getScaling() const { return {scaleX, scaleY}; }

        [[nodiscard]] static Point getScreenDims()
        {
            return {static_cast<float>(CORE.Window.screen.width), static_cast<float>(CORE.Window.screen.height)};
        }

        [[nodiscard]] Point getMousePos() const { return {mouseX, mouseY}; }

        //----------------- SCENES -----------------//

        void renderUI()
        {
            const auto start = renderObjects.size() - 1;
            for (int i = start; i > -1; --i)
            {
                auto& obj = *renderObjects[i];
                obj.draw(obj.getBounds());
            }
        }

    };

    namespace global
    {
        inline UIData UI_DATA;
    }
} // namespace magique

#endif //UIDATA_H