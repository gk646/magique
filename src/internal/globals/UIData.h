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
        StringHashMap<UIObject*> objectMap;              // Stores by name
        HashMap<GameState, vector<UIObject*>> stateData; // Different states - hashmap cause we dont know the user enum
        UIObject* hoveredObject = nullptr;               // Currently hovered object
        CursorAttachment cursorAttachment{};             // Current object attached to the cursor
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

            auto vec = getStateObjects(GetGameState());
            if (vec != nullptr) [[likely]]
            {
                for (int i = vec->size() - 1; i > -1; --i)
                {
                    vec->operator[](i)->update();
                }
            }
        }

        void draw()
        {
            auto vec = getStateObjects(GetGameState());
            if (vec != nullptr) [[likely]]
            {
                for (const auto obj : *vec)
                {
                    obj->draw();
                }
            }
        }

        vector<UIObject*>* getStateObjects(const GameState gameState) // Only valid till next deletion!
        {
            const auto it = stateData.find(gameState);
            if (it == stateData.end())
                return nullptr;
            return &it->second;
        }

        void registerObject(const char* name, const GameState gameState, UIObject* object, const UILayer layer)
        {
            object->setLayer(layer);
            const auto it = objectMap.find(name);
            if (it == objectMap.end())
            {
                objectMap.insert({name, object});
                object->instances++;
            }
            else if (it->second != object)
            {
                delete object;
                it->second = object;
            }

            auto& vec = stateData[gameState];
            auto** vecIt = vec.begin();
            const auto* const vecEnd = vec.end();
            while (vecIt != vecEnd)
            {
                if ((*vecIt)->getLayer() < object->getLayer())
                {
                    break;
                }
                ++vecIt;
            }
            vec.insert(vecIt, object);
        }

        bool unregisterObject(UIObject* object, const GameState gameState)
        {
            const auto eraseObject = [](UIObject* object, StringHashMap<UIObject*>& objectMap)
            {
                --object->instances;
                if (object->instances == 0)
                {
                    for (auto it = objectMap.begin(); it != objectMap.end(); ++it)

                        if (it->second == object)
                        {
                            objectMap.erase(it);
                            return true;
                        }
                }
                return false;
            };

            if (gameState == GameState(INT32_MAX))
            {
                bool val = false;
                for (auto& [state, vec] : stateData)
                {
                    if (vec.empty())
                        continue;
                    if (vec.erase(object))
                    {
                        if (eraseObject(object, objectMap))
                        {
                            return true;
                        }
                        val = true;
                    }
                }
                return val;
            }

            const auto it = stateData.find(gameState);
            if (it == stateData.end())
                return false;
            auto& vec = it->second;
            if (vec.empty())
                return false;
            if (vec.erase(object))
            {
                eraseObject(object, objectMap);
                return true;
            }
            return false;
        }

        [[nodiscard]] Point getScaling() const { return {scaleX, scaleY}; }

        [[nodiscard]] static Point getScreenDims()
        {
            return {static_cast<float>(CORE.Window.screen.width), static_cast<float>(CORE.Window.screen.height)};
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