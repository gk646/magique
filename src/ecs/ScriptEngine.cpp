#include <magique/ecs/ExternalScripting.h>
#include <magique/ecs/InternalScripting.h>

#include "ScriptEngine.h"

namespace magique
{
    void SetScript(EntityID entity, EntityScript* script)
    {
        if (SCRIPT_ENGINE.scripts.size() < entity + 1)
        {
            assert(entity < 250 && "Sanity check");
            SCRIPT_ENGINE.scripts.resize(entity + 1, nullptr);
        }

        if (SCRIPT_ENGINE.scripts[entity])
            delete SCRIPT_ENGINE.scripts[entity];
        SCRIPT_ENGINE.scripts[entity] = script;
    }


    EntityScript* GetScript(const EntityID entity)
    {
        M_ASSERT(SCRIPT_ENGINE.scripts.size() > entity,
                 "No script registered for this type! Did you call SetScriptForEntity?");
        M_ASSERT(SCRIPT_ENGINE.scripts[entity] != nullptr,
                 "No valid script exists! Did you pass a new Instance of your ScriptClass?");
        return SCRIPT_ENGINE.scripts[entity];
    }

} // namespace magique