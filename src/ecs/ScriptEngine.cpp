#include <magique/ecs/Scripting.h>

#include "internal/globals/ScriptEngine.h"

namespace magique
{
    void SetScript(EntityID entity, EntityScript* script)
    {
        if (global::SCRIPT_ENGINE.scripts.size() < entity + 1)
        {
            M_ASSERT(entity < 1000, "Sanity check! If you have more than 1000 entity types kudos!");
            global::SCRIPT_ENGINE.scripts.resize(entity + 1, global::SCRIPT_ENGINE.defaultScript);
        }

        // Dont delete the default script
        if (global::SCRIPT_ENGINE.scripts[entity] && global::SCRIPT_ENGINE.scripts[entity] != global::SCRIPT_ENGINE.defaultScript)
            delete global::SCRIPT_ENGINE.scripts[entity];
        global::SCRIPT_ENGINE.scripts[entity] = script;
    }

    EntityScript* GetScript(const EntityID entity)
    {
        M_ASSERT(global::SCRIPT_ENGINE.scripts.size() > entity, "No script registered for this type! Did you call SetScript()?");
        M_ASSERT(global::SCRIPT_ENGINE.scripts[entity] != global::SCRIPT_ENGINE.defaultScript,
                 "No valid script exists for this entity! Did you call SetScript() and pass a new Instance of your "
                 "ScriptClass?");
        return global::SCRIPT_ENGINE.scripts[entity];
    }

} // namespace magique