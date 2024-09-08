#include <magique/ecs/Scripting.h>

#include "internal/globals/ScriptData.h"
#include "internal/globals/EngineData.h"

namespace magique
{
    void SetIsAccumulated(CollisionInfo& info) // Needed to cleanly define it as friend method
    {
        info.isAccumulated = true;
    }
    void EntityScript::AccumulateCollision(CollisionInfo& collisionInfo)
    {
        SetIsAccumulated(collisionInfo);
    }

    void SetEntityScript(const EntityType entity, EntityScript* script)
    {
        auto& scData = global::SCRIPT_DATA;
        if (scData.scripts.size() < entity + 1)
        {
            MAGIQUE_ASSERT(entity < UINT16_MAX, "Sanity check");
            scData.scripts.resize(entity + 1, ScriptData::defaultScript);
        }
        // Dont delete the default script
        if (scData.scripts[entity] && scData.scripts[entity] != ScriptData::defaultScript)
            delete scData.scripts[entity];
        scData.scripts[entity] = script;
    }

    EntityScript* GetEntityScript(const EntityType entity)
    {
        auto& scData = global::SCRIPT_DATA;
        MAGIQUE_ASSERT(scData.scripts.size() > entity, "No script registered for this type! Did you call SetScript()?");
        MAGIQUE_ASSERT(
            scData.scripts[entity] != scData.defaultScript,
            "No valid script exists for this entity! Did you call SetScript() and pass a new Instance of your "
            "ScriptClass?");
        return scData.scripts[entity];
    }

} // namespace magique