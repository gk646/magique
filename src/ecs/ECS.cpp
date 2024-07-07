#include <magique/ecs/ECS.h>
#include <magique/ecs/InternalScripting.h>
#include <magique/ecs/Components.h>
#include <raylib/raylib.h>

#include "core/globals/EntityTypeMap.h"
#include "core/globals/LogicTickData.h"
#include "core/globals/LogicThread.h"
#include "core/Config.h"
#include "ScriptEngine.h"

namespace magique
{
    bool RegisterEntity(const EntityID type, const std::function<void(entt::registry&, entt::entity)>& createFunc)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ENT_TYPE_MAP;
        if (type == static_cast<EntityID>(UINT16_MAX) || map.contains(type))
        {
            return false; // Invalid ID or already registered
        }

        map.insert({type, createFunc});

        for (auto entity : REGISTRY.view<entt::entity>())
        {
            volatile int b = (int)entity; // Try to instantiate all storage types
        }
        return true;
    }

    bool UnRegisterEntity(const EntityID type)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ENT_TYPE_MAP;

        if (type == static_cast<EntityID>(UINT16_MAX) || !map.contains(type))
        {
            return false; // Invalid ID or not registered
        }

        map.erase(type);
        return true;
    }

    entt::entity CreateEntity(const EntityID type, float x, float y, MapID mapID)
    {
        M_ASSERT(std::this_thread::get_id() == global::LOGIC_THREAD.get_id(), "Has to be called from the logic thread");
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");

        auto& map = global::ENT_TYPE_MAP;

        const auto it = map.find(type);
        if (it == map.end())
        {
            return entt::null; // EntityID not registered
        }

        auto& tickData = global::LOGIC_TICK_DATA;
        tickData.lock();
        const auto entity = REGISTRY.create();
        REGISTRY.emplace<PositionC>(entity, x, y, type, mapID); // PositionC is default
        it->second(REGISTRY, entity);
        tickData.unlock();
        SCRIPT_ENGINE.padUpToEntity(type); // This assures its always valid to index with type
        return entity;
    }

    bool DestroyEntity(const entt::entity entity)
    {
        M_ASSERT(std::this_thread::get_id() == global::LOGIC_THREAD.get_id(), "Has to be called from the logic thread");
        auto& tickData = global::LOGIC_TICK_DATA;
        if (REGISTRY.valid(entity))
        {
            tickData.lock();
            REGISTRY.destroy(entity);
            tickData.entityUpdateCache.erase(entity);
            tickData.unlock();
            return true;
        }
        return false;
    }

   void GiveCamera(entt::entity entity) {
         REGISTRY.emplace<CameraC>(entity);

    }

    OccluderC& GiveOccluder(entt::entity entity, int width, int height, Shape shape)
    {
        return REGISTRY.emplace<OccluderC>(entity, (int16_t)width, (int16_t)height, shape);
    }

    EmitterC& GiveEmitter(entt::entity entity, Color color, int intensity, LightStyle style)
    {
        return REGISTRY.emplace<EmitterC>(entity, color.r, color.g, color.b, color.a, (uint16_t)intensity, style);
    }

    void GiveActor(entt::entity e) { return REGISTRY.emplace<ActorC>(e); }

    CollisionC& GiveCollision(entt::entity e, Shape shape, int width, int height, int anchorX, int anchorY)
    {
        return REGISTRY.emplace<CollisionC>(e,{}, shape, (uint16_t)width, (uint16_t)height, static_cast<int16_t>(anchorX),
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