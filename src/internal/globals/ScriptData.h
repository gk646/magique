// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SCRIPTENGINE_H
#define MAGIQUE_SCRIPTENGINE_H

#include <magique/ecs/Scripting.h>

namespace magique
{
    struct ScriptData final
    {
        inline static auto* defaultScript = new EntityScript();
        SparseRangeVector<EntityScript*> scripts;
    };

    namespace global
    {
        inline ScriptData SCRIPT_DATA{};
    }
} // namespace magique


#endif //MAGIQUE_SCRIPTENGINE_H
