#include <entt/entity/registry.hpp>
#include <magique/ecs/Registry.h>

#include <cxutil/cxassert.h>


#include "core/CoreData.h"

namespace magique::ecs
{

    entt::registry& GetRegistry() { return ENTT_REGISTRY; }

    bool RegisterEntity(const EntityType type, const std::function<void(entt::registry&, entt::entity)>& createFunc)
    {
        CX_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        if (type == static_cast<EntityType>(UINT16_MAX) || ENT_TYPE_MAP.contains(type))
        {
            return false; // Invalid ID or already registered
        }

        ENT_TYPE_MAP.insert({type, createFunc});
        return true;
    }

    bool UnRegisterEntity(const EntityType type)
    {
        CX_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");

        if (type == static_cast<EntityType>(UINT16_MAX) || !ENT_TYPE_MAP.contains(type))
        {
            return false; // Invalid ID or not registered
        }

        ENT_TYPE_MAP.erase(type);
        return true;
    }

    entt::entity CreateEntity(const EntityType id)
    {
        CX_ASSERT(id < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");

        const auto it = ENT_TYPE_MAP.find(id);
        if (it == ENT_TYPE_MAP.end())
        {
            return entt::null; // EntityID not registered
        }
        const auto entity = ENTT_REGISTRY.create();
        ENTT_REGISTRY.emplace<PositionC>(entity); // PositionC is default
        it->second(ENTT_REGISTRY, entity);
        return entity;
    }

    bool DestroyEntity(const entt::entity entity)
    {
        if (ENTT_REGISTRY.valid(entity))
        {
            ENTT_REGISTRY.destroy(entity);
            return true;
        }
        return false;
    }

    void MakeActor(entt::entity e) { ENTT_REGISTRY.emplace<ActorC>(e); }


    void MakeCollision(entt::entity e, Shape shape, int anchorX, int anchorY)
    {
        ENTT_REGISTRY.emplace<CollisionC>(e, shape, static_cast<int16_t>(anchorX), static_cast<int16_t>(anchorY));
    }

} // namespace magique::ecs