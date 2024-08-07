#include <magique/ecs/Scripting.h>

#include "internal/globals/ScriptData.h"

namespace magique
{
    void SetScript(const EntityID entity, EntityScript* script)
    {
        if (global::SCRIPT_DATA.scripts.size() < entity + 1)
        {
            M_ASSERT(entity < 1000, "Sanity check! If you have more than 1000 entity types kudos!");
            global::SCRIPT_DATA.scripts.resize(entity + 1, global::SCRIPT_DATA.defaultScript);
        }

        // Dont delete the default script
        if (global::SCRIPT_DATA.scripts[entity] && global::SCRIPT_DATA.scripts[entity] != global::SCRIPT_DATA.defaultScript)
            delete global::SCRIPT_DATA.scripts[entity];
        global::SCRIPT_DATA.scripts[entity] = script;
    }

    EntityScript* GetScript(const EntityID entity)
    {
        M_ASSERT(global::SCRIPT_DATA.scripts.size() > entity, "No script registered for this type! Did you call SetScript()?");
        M_ASSERT(global::SCRIPT_DATA.scripts[entity] != global::SCRIPT_DATA.defaultScript,
                 "No valid script exists for this entity! Did you call SetScript() and pass a new Instance of your "
                 "ScriptClass?");
        return global::SCRIPT_DATA.scripts[entity];
    }

} // namespace magique