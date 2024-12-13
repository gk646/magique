// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ecs/Scripting.h>

#include "internal/globals/ScriptData.h"
#include "internal/globals/EngineData.h"

namespace magique
{
    void SetIsAccumulated(CollisionInfo& info) // Needed to cleanly define it as friend method
    {
        info.isAccumulated = true;
    }

    void EntityScript::AccumulateCollision(CollisionInfo& collisionInfo) { SetIsAccumulated(collisionInfo); }

    void SetEntityScript(const EntityType entity, EntityScript* script)
    {
        auto& scData = global::SCRIPT_DATA;
        if (scData.scripts.size() < entity + 1)
        {
            MAGIQUE_ASSERT(entity < UINT16_MAX, "Sanity check");
            scData.scripts.resize(entity + 1, ScriptData::defaultScript);
        }
        // Dont delete the default script
        IGNORE_WARNING_GCC("-Wdelete-non-virtual-dtor")
        if (scData.scripts[entity] != ScriptData::defaultScript)
            delete scData.scripts[entity];
        UNIGNORE_WARNING_GCC();
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

    void SetIsEntityScripted(const entt::entity entity, const bool val)
    {
        if (val)
        {
            global::ENGINE_DATA.entityNScriptedSet.erase(entity);
        }
        else
        {
            global::ENGINE_DATA.entityNScriptedSet.insert(entity);
        }
    }
    bool GetIsEntityScripted(const entt::entity entity)
    {
        return global::ENGINE_DATA.entityNScriptedSet.contains(entity);
    }

} // namespace magique