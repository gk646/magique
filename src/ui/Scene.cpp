#include <magique/ui/Scene.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{
    bool RegisterScene(Scene* scene, const char* name)
    {
        auto& data = global::UI_DATA;
        auto it = data.sceneMap.find(name);
        if (it != data.sceneMap.end())
        {
            LOG_ERROR("Scene with name already exists: %s", name);
            return nullptr;
        }
        data.sceneMap.insert({name, scene});
        return scene;
    }

    Scene* GetSceneImpl(const char* name)
    {
        const auto& data = global::UI_DATA;
        const auto it = data.sceneMap.find(name);
        if (it != data.sceneMap.end())
            return it->second;
        return nullptr;
    }

    SceneData& GetSceneData(const int idx) { return global::UI_DATA.sceneData[idx]; }

    Scene::Scene() : dataIdx(global::UI_DATA.dataIndex++) { global::UI_DATA.sceneData.push_back({}); }

    void Scene::addObjetctImpl(UIObject* object) const { GetSceneData(dataIdx).objects.push_back(object); }

    void Scene::draw() const
    {
        auto& scData = GetSceneData(dataIdx);
        auto& data = global::UI_DATA;
        for (const auto obj : scData.objects)
        {
            data.renderObjects.push_back(obj);
        }

    }


} // namespace magique