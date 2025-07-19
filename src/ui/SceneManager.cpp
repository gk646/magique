#include <magique/ui/SceneManager.h>

#include "internal/datastructures/StringHashMap.h"
#include "magique/ui/UIObject.h"

namespace magique
{
    SceneManager& GetSceneManager()
    {
        static SceneManager SCENE_MANAGER{};
        return SCENE_MANAGER;
    }

    void Scene::draw()
    {
        const auto size = (int)objects.size() - 1;
        for (int i = size; i >= 0; --i)
        {
            objects[i]->draw();
        }
    }

    UIObject* Scene::addObject(UIObject* obj, const char* name)
    {
        MAGIQUE_ASSERT(obj != nullptr, "Passed nullptr");
        objects.push_back(obj);
        if (name != nullptr && strlen(name) < MAGIQUE_MAX_NAMES_LENGTH)
        {
            auto& mapping = mappings.emplace_back();
            auto len = strlen(name);
            std::memcpy(mapping.name, name, len + 1);
            mapping.name[len + 1] = '\0';
            mapping.object = obj;
        }
        return obj;
    }

    bool Scene::removeObject(UIObject* obj) { return std::erase(objects, obj) > 0; }

    UIObject* Scene::getObject(const char* name) const
    {
        for (const auto& mapping : mappings)
        {
            if (strcmp(mapping.name, name) == 0)
            {
                return mapping.object;
            }
        }
        return nullptr;
    }

    const std::vector<UIObject*>& Scene::getObjects() { return objects; }

    struct SceneManagerData final
    {
        StringHashMap<Scene> nameMap;
        HashMap<GameState, Scene> stateMap;
    };

    static SceneManagerData SCENE_DATA{};

    Scene& SceneManager::getScene(const char* name) { return SCENE_DATA.nameMap[name]; }

    Scene& SceneManager::getScene(const GameState state) { return SCENE_DATA.stateMap[state]; }


} // namespace magique