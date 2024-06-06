#include "ScriptEngine.h"

#include <cassert>
#include <magique/ecs/ExternalScripting.h>
#include <magique/ecs/InternalScripting.h>


namespace magique
{
    void SetScript(EntityID entity, EntityScript* script)
    {
        if (SCRIPT_ENGINE.scripts.size() < entity+1)
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
        assert(SCRIPT_ENGINE.scripts.size() > entity);
        assert(SCRIPT_ENGINE.scripts[entity] != nullptr && "No script exists! Did you call SetScriptForEntity?");
        return SCRIPT_ENGINE.scripts[entity];
    }

} // namespace magique::ecs