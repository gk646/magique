// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SCRIPTENGINE_H
#define MAGIQUE_SCRIPTENGINE_H

#include <magique/ecs/Scripting.h>


namespace magique
{
    struct ScriptData final
    {
        inline static auto* defaultScript = new EntityScript();
        std::vector<EntityScript*> scripts;

        void padUpToEntity(const EntityType entity)
        {
            const auto newSize = (size_t)entity + 1U;
            if (scripts.size() < newSize)
            {
                scripts.resize(newSize, defaultScript);
            }
        }
    };

    namespace global
    {
        inline ScriptData SCRIPT_DATA{};
    }
} // namespace magique


#endif //MAGIQUE_SCRIPTENGINE_H
