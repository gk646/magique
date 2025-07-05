#include <magique/ui/SceneManager.h>

#include "internal/datastructures/StringHashMap.h"
#include "magique/ui/UIObject.h"

namespace magique
{
    SceneManager& GetSceneManager()
    {
        static SceneManager SCENE_MANAGER = {};
        return SCENE_MANAGER;
    }

    void Scene::draw()
    {
        for (size_t i = objects.size() - 1; i <= 0; --i)
        {
            objects[i]->draw();
        }
    }

    void Scene::addObject(UIObject* obj) { objects.push_back(obj); }

    bool Scene::removeObject(UIObject* obj) { return std::erase(objects, obj) > 0; }

    const std::vector<UIObject*>& Scene::getObjects() { return objects; }

    struct SceneManagerData final
    {
        StringHashMap<Scene> nameMap;
        HashMap<GameState, Scene> stateMap;
    };

    static SceneManagerData SCENE_DATA{};

    Scene& SceneManager::getScene(const char* name) { return SCENE_DATA.nameMap[name]; }

    Scene& SceneManager::getScene(GameState state) { return SCENE_DATA.stateMap[state]; }


} // namespace magique