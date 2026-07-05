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

    void ScriptingSetScript(const EntityType type, EntityScript* script)
    {
        auto& scData = global::SCRIPT_DATA;
        // Dont delete the default script
        if (scData.scripts[type] != ScriptData::defaultScript)
            delete scData.scripts[type];
        scData.scripts[type] = script;
    }

    void ScriptingSetScript(std::initializer_list<EntityType> types, EntityScript* script)
    {
        for (auto type : types)
        {
            ScriptingSetScript(type, script);
        }
    }

    namespace internal
    {
        EntityScript* GetScriptInternal(const Entity entity)
        {
            const auto& scriptData = global::SCRIPT_DATA;
            const auto type = ComponentGet<PositionC>(entity).type;
            const auto it = scriptData.scripts.find(type);
            MAGIQUE_ASSERT(it != scriptData.scripts.end(), "No script registered for this type! ");
            if (it == scriptData.scripts.end())
            {
                return scriptData.defaultScript;
            }
            return it->second;
        }
    } // namespace internal


    void ScriptingSetScripted(const Entity entity, const bool val)
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

    bool ScriptingGetIsScripted(const Entity entity) { return global::ENGINE_DATA.entityNScriptedSet.contains(entity); }

    void EntityScript::AccumulateCollision(CollisionInfo& collision) { SetIsAccumulated(collision); }


} // namespace magique
