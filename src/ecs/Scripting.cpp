#include <magique/ecs/Scripting.h>

#include "internal/globals/ScriptData.h"
#include "internal/globals/EngineData.h"

namespace magique
{
    void EntityScript::AccumulateCollision(const entt::entity entity, const CollisionInfo& collisionInfo)
    {
        auto& data = global::ENGINE_DATA;
        auto& [pos, normal] = data.infoMap[entity];
        normal.x += collisionInfo.normalVector.x * collisionInfo.penDepth;
        normal.y += collisionInfo.normalVector.y * collisionInfo.penDepth;
        if (pos == nullptr)
            pos = &internal::POSITION_GROUP.get<PositionC>(entity);
    }

    void SetScript(const EntityType entity, EntityScript* script)
    {
        auto& scData = global::SCRIPT_DATA;
        if (scData.scripts.size() < entity + 1)
        {
            MAGIQUE_ASSERT(entity < 1000, "Sanity check! If you have more than 1000 entity types kudos!");
            scData.scripts.resize(entity + 1, ScriptData::defaultScript);
        }
        // Dont delete the default script
        if (scData.scripts[entity] && scData.scripts[entity] != ScriptData::defaultScript)
            delete scData.scripts[entity];
        scData.scripts[entity] = script;
    }

    EntityScript* GetScript(const EntityType entity)
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