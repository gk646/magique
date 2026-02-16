#include <magique/ui/Scene.h>
#include <magique/util/Datastructures.h>

#include "magique/ui/UIObject.h"

namespace magique
{
    inline SceneManager SCENE_MANAGER{};

    SceneManager& SceneGetManager() { return SCENE_MANAGER; }

    void Scene::draw() const
    {
        const auto size = (int)objects.size() - 1;
        for (int i = size; i >= 0; --i)
        {
            objects[i]->draw();
        }
    }

    UIObject* Scene::addObjImpl(UIObject* obj, const char* name)
    {
        MAGIQUE_ASSERT(obj != nullptr, "Passed nullptr");
        objects.push_back(obj);
        if (name != nullptr)
        {
            mappings.emplace_back(name, obj);
        }
        return obj;
    }

    bool Scene::removeObject(UIObject* obj) { return std::erase(objects, obj) > 0; }

    UIObject* Scene::getObjectImpl(const char* name) const
    {
        for (const auto& mapping : mappings)
        {
            if (mapping.name == name)
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

    inline SceneManagerData SCENE_DATA{};

    Scene& SceneManager::getScene(const char* name) { return SCENE_DATA.nameMap[name]; }

    Scene& SceneManager::getScene(const GameState state) { return SCENE_DATA.stateMap[state]; }


} // namespace magique
