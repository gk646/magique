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
    struct SceneData final
    {
        vector<UIObject*> objects; // Sorted objects
        float workBuildup = 0.0F;
    };

    struct UIData final
    {
        vector<SceneData> sceneData;
        vector<UIObject*> renderObjects;
        StringHashMap<Scene*> sceneMap;      // Stores by name
        UIObject* hoveredObject = nullptr;   // Currently hovered object
        CursorAttachment cursorAttachment{}; // Current object attached to the cursor
        float scaleX = 1.0F;
        float scaleY = 1.0F;
        float mouseX = 0.0F;
        float mouseY = 0.0F;
        int dataIndex = 0;

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
            return {static_cast<float>(CORE.Window.screen.width), static_cast<float>(CORE.Window.screen.height)};
        }

        [[nodiscard]] Point getMousePos() const { return {mouseX, mouseY}; }

        //----------------- SCENES -----------------//

        void renderScenes()
        {
            const auto start = renderObjects.size() - 1;
            for (int i = start; i > -1; --i)
            {
                renderObjects[i]->draw();
            }
        }

        void updateScenes()
        {

        }
    };

    namespace global
    {
        inline UIData UI_DATA;
    }
} // namespace magique

#endif //UIDATA_H