#ifndef MAGIQUE_SCRIPTENGINE_H
#define MAGIQUE_SCRIPTENGINE_H

#include <vector>
#include <magique/ecs/Scripting.h>

namespace magique
{
    struct ScriptEngine
    {
        inline static auto* defaultScript = new EntityScript();
        std::vector<EntityScript*> scripts;

        void padUpToEntity(const EntityID entity)
        {
            if (scripts.size() < entity + 1)
            {
                scripts.resize(entity + 1, defaultScript);
            }
        }
    };

    namespace global
    {

        inline ScriptEngine SCRIPT_ENGINE;
    }
} // namespace magique


#endif //MAGIQUE_SCRIPTENGINE_H