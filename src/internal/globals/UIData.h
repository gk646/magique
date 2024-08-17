#ifndef MAGIQUE_UIDATA_H
#define MAGIQUE_UIDATA_H

#include <raylib/raylib.h>

#include <magique/util/Defines.h>
#include <magique/ui/types/UIObject.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/StringHashMap.h"
#include "external/raylib/src/coredata.h"
#include "internal/utils/STLUtil.h"

namespace magique
{
    struct RenderData final
    {
        UIObject* obj;
        float transparency = 1.0F;
        bool scissor = false;
    };

    struct UIData final
    {
        StringHashMap<UIScene*> sceneMap;   // Stores by name
        HashSet<const UIObject*> renderSet; // Stores existence of rendered objects
        vector<RenderData> renderObjects;
        vector<UIObject*> allObjects;

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

            for (int i = 0; i < allObjects.size(); ++i) // Using fori to support deletions in the update methods
            {
                auto& obj = *allObjects[i];
                obj.update(obj.getBounds(), renderSet.contains(&obj));
            }
        }

        void draw()
        {
            const auto start = renderObjects.size() - 1;
            for (int i = start; i > -1; --i)
            {
                auto& data = renderObjects[i];
                const auto bounds = data.obj->getBounds();
                if (data.scissor)
                {
                    BeginScissorMode(bounds.x, bounds.y, bounds.width, bounds.height);
                    data.obj->draw(bounds);
                    EndScissorMode();
                }
                else
                {
                    data.obj->draw(bounds);
                }
            }
        }

        void clearRenderObjects()
        {
            renderObjects.clear();
            renderSet.clear();
        }

        void addRenderObject(UIObject& object, const float t, const bool sc)
        {
            renderSet.insert(&object);
            renderObjects.push_back({&object, t, sc});
        }

        // All objects are registered in their ctor
        void registerObject(UIObject& object) { allObjects.push_back(&object); }

        // All objects are un-registered in the dtor
        void unregisterObject(const UIObject& object)
        {
            UnorderedDelete(allObjects, &object);
            const auto it = renderSet.find(&object);
            if(it != renderSet.end())
            {
                renderSet.erase(it);
                for (auto jt = renderObjects.begin(); jt != renderObjects.end(); ++jt)
                {
                    if (jt->obj == &object)
                    {
                        renderObjects.erase(jt);
                        break;
                    }
                }
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
        inline UIData UI_DATA;
    }
} // namespace magique

#endif //MAGIQUE_UIDATA_H