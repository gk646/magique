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

    void ScriptingSetScript(const EntityType entity, EntityScript* script)
    {
        auto& scData = global::SCRIPT_DATA;
        // Dont delete the default script
        if (scData.scripts[entity] != ScriptData::defaultScript)
            delete scData.scripts[entity];
        scData.scripts[entity] = script;
    }

    EntityScript* ScriptingGetScript(const EntityType entity)
    {
        const auto& scriptData = global::SCRIPT_DATA;
        MAGIQUE_ASSERT(scriptData.scripts.width() > (size_t)entity,
                       "No script registered for this type! Did you call SetScript()?");
        auto* script = scriptData.scripts[entity];
        if (script == nullptr)
        {
            return scriptData.defaultScript;
        }
        return script;
    }

    void ScriptingSetScripted(const entt::entity entity, const bool val)
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

    bool ScriptingGetIsScripted(const entt::entity entity)
    {
        return global::ENGINE_DATA.entityNScriptedSet.contains(entity);
    }

    void EntityScript::AccumulateCollision(CollisionInfo& collisionInfo) { SetIsAccumulated(collisionInfo); }

} // namespace magique
