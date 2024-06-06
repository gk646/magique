#include <magique/ecs/ECS.h>
#include <magique/ecs/InternalScripting.h>
#include <magique/ecs/Components.h>

#include "core/CoreData.h"
#include "core/CoreConfig.h"
#include "ScriptEngine.h"

namespace magique
{
    bool RegisterEntity(const EntityID type, const std::function<void(entt::registry&, entt::entity)>& createFunc)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        if (type == static_cast<EntityID>(UINT16_MAX) || ENT_TYPE_MAP.contains(type))
        {
            return false; // Invalid ID or already registered
        }

        ENT_TYPE_MAP.insert({type, createFunc});

        for (auto entity : REGISTRY.view<entt::entity>())
        {
            volatile int b = (int)entity; // Try to instantiate all storage types
        }


        return true;
    }

    bool UnRegisterEntity(const EntityID type)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");

        if (type == static_cast<EntityID>(UINT16_MAX) || !ENT_TYPE_MAP.contains(type))
        {
            return false; // Invalid ID or not registered
        }

        ENT_TYPE_MAP.erase(type);
        return true;
    }

    entt::entity CreateEntity(const EntityID type, float x, float y, MapID map)
    {
        M_ASSERT(std::this_thread::get_id() == LOGIC_THREAD.get_id(), "Has to be called from the logic thread");
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");

        const auto it = ENT_TYPE_MAP.find(type);
        if (it == ENT_TYPE_MAP.end())
        {
            return entt::null; // EntityID not registered
        }
        LOGIC_TICK_DATA.lock();
        const auto entity = REGISTRY.create();
        REGISTRY.emplace<PositionC>(entity, x, y, type, map); // PositionC is default
        it->second(REGISTRY, entity);
        LOGIC_TICK_DATA.unlock();
        SCRIPT_ENGINE.padUpToEntity(type); // This assures its always valid to index with type
        return entity;
    }

    bool DestroyEntity(const entt::entity entity)
    {
        M_ASSERT(std::this_thread::get_id() == LOGIC_THREAD.get_id(), "Has to be called from the logic thread");
        if (REGISTRY.valid(entity))
        {
            LOGIC_TICK_DATA.lock();
            REGISTRY.destroy(entity);
            LOGIC_TICK_DATA.entityUpdateCache.erase(entity);
            LOGIC_TICK_DATA.unlock();
            return true;
        }
        return false;
    }

    void GiveCamera(entt::entity entity)
    {
        REGISTRY.emplace<CameraC>(entity);
    }

    void GiveActor(entt::entity e) { REGISTRY.emplace<ActorC>(e); }

    void GiveCollision(entt::entity e, Shape shape, int width, int height, int anchorX, int anchorY)
    {
        REGISTRY.emplace<CollisionC>(e, shape, (uint16_t)width, (uint16_t)height, static_cast<int16_t>(anchorX),
                                     static_cast<int16_t>(anchorY));
    }

    void GiveScript(const entt::entity e) { REGISTRY.emplace<ScriptC>(e); }

    void GiveDebugVisuals(entt::entity e)
    {
#ifdef MAGIQUE_DEBUG_ENTITIES
        REGISTRY.emplace<DebugVisualsC>(e);
#else
        LOG_WARNING("Using debug function but not in debug mode!");
#endif
    }

    void GiveDebugController(entt::entity e)
    {
#ifdef MAGIQUE_DEBUG_ENTITIES
        REGISTRY.emplace<DebugControllerC>(e);
#else
        LOG_WARNING("Using debug function but not in debug mode!");
#endif
    }


} // namespace magique