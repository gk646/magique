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

    entt::entity CreateEntity(const EntityType type, float x, float y, MapID map)
    {
        CX_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        const auto it = ENT_TYPE_MAP.find(type);
        if (it == ENT_TYPE_MAP.end())
        {
            return entt::null; // EntityID not registered
        }
        const auto entity = ENTT_REGISTRY.create();
        ENTT_REGISTRY.emplace<PositionC>(entity, x, y,type,  map); // PositionC is default
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

    void GiveActor(entt::entity e) { ENTT_REGISTRY.emplace<ActorC>(e); }

    void GiveCollision(entt::entity e, Shape shape, int width, int height, int anchorX, int anchorY)
    {
        ENTT_REGISTRY.emplace<CollisionC>(e, shape, (uint16_t)width, (uint16_t)height, static_cast<int16_t>(anchorX),
                                          static_cast<int16_t>(anchorY));
    }

    void GiveDebugVisuals(entt::entity e)
    {
#if MAGIQUE_DEBUG == 1
        ENTT_REGISTRY.emplace<DebugVisualsC>(e);
#else
        LOG_WARNING("Using debug function but not in debug mode!");
#endif
    }

void GiveDebugController(entt::entity e)
    {
#if MAGIQUE_DEBUG == 1
        ENTT_REGISTRY.emplace<DebugControllerC>(e);
#else
        LOG_WARNING("Using debug function but not in debug mode!");
#endif
    }



} // namespace magique::ecs