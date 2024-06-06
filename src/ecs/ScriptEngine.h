#ifndef MAGIQUE_SCRIPTENGINE_H
#define MAGIQUE_SCRIPTENGINE_H

#include <vector>
#include <magique/ecs/InternalScripting.h>

namespace magique
{
    struct ScriptEngine
    {
        std::vector<EntityScript*> scripts;
    };

    inline ScriptEngine SCRIPT_ENGINE;

} // namespace magique


#endif //MAGIQUE_SCRIPTENGINE_H