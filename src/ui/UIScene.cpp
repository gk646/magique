#include <magique/ui/UIScene.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{
     UIScene::UIScene() {  }

    bool RegisterScene(UIScene* scene, const char* name)
    {
        auto& data = global::UI_DATA;
        auto it = data.sceneMap.find(name);
        if (it != data.sceneMap.end())
        {
            LOG_ERROR("Scene with name already exists: %s", name);
            return false;
        }
        data.sceneMap.insert({name, scene});
        return scene;
    }

    UIScene* GetSceneImpl(const char* name)
    {
        const auto& data = global::UI_DATA;
        const auto it = data.sceneMap.find(name);
        if (it != data.sceneMap.end())
            return it->second;
        return nullptr;
    }


} // namespace magique