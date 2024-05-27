#include "core/Core.h"

#include <entt/entity/registry.hpp>
#include <magique/ecs/Registry.h>

namespace magique::ecs
{
    // Internal registry
    inline entt::registry ENTT_REGISTRY;

    entt::registry& GetRegistry() { return ENTT_REGISTRY; }

    // Type map
    HashMap<EntityType, const std::function<void(entt::registry&, entt::entity)>> typeMap;


    bool RegisterEntity(const EntityType type, const std::function<void(entt::registry&, entt::entity)>& createFunc)
    {
        CX_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        if (type == static_cast<EntityType>(UINT16_MAX) || typeMap.contains(type))
        {
            return false; // Invalid ID or already registered
        }

        typeMap.insert({type, createFunc});
        return true;
    }

    bool UnRegisterEntity(const EntityType type)
    {
        CX_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");

        if (type == static_cast<EntityType>(UINT16_MAX) || !typeMap.contains(type))
        {
            return false; // Invalid ID or not registered
        }

        typeMap.erase(type);
        return true;
    }

    entt::entity CreateEntity(const EntityType id)
    {
        CX_ASSERT(id < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");

        const auto it = typeMap.find(id);
        if (it == typeMap.end())
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