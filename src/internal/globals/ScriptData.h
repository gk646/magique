#ifndef MAGIQUE_SCRIPTENGINE_H
#define MAGIQUE_SCRIPTENGINE_H

#include <magique/ecs/Scripting.h>

#include "internal/datastructures/VectorType.h"

namespace magique
{
    struct ScriptData final
    {
        inline static auto* defaultScript = new EntityScript();
        vector<EntityScript*> scripts;

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

        inline ScriptData SCRIPT_DATA;
    }
} // namespace magique


#endif //MAGIQUE_SCRIPTENGINE_H