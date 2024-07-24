#include <raylib/raylib.h>

#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/ecs/Components.h>
#include <magique/util/Defines.h>

#include "internal/globals/EntityTypeMap.h"
#include "internal/globals/LogicTickData.h"
#include "internal/globals/LogicThread.h"
#include "internal/globals/ScriptEngine.h"

namespace magique
{
    bool RegisterEntity(const EntityID type, const std::function<void(entt::registry&, entt::entity)>& createFunc)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ENT_TYPE_MAP;
        if (type == static_cast<EntityID>(UINT16_MAX) || map.contains(type))
        {
            return false; // Invalid ID or already registered
        }

        map.insert({type, createFunc});
        global::SCRIPT_ENGINE.padUpToEntity(type); // This assures its always valid to index with type
        for (auto entity : internal::REGISTRY.view<entt::entity>())
        {
            volatile int b = static_cast<int>(entity); // Try to instantiate all storage types
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

    bool IsEntityExisting(const entt::entity e) { return internal::REGISTRY.valid(e); }

    entt::entity CreateEntity(const EntityID type, float x, float y, MapID mapID)
    {
        auto& tickData = global::LOGIC_TICK_DATA;
        M_ASSERT(std::this_thread::get_id() == global::LOGIC_THREAD.get_id() || !tickData.flag.test(),
                 "Has to be called from the logic thread OR without active lock on the draw thread");
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");

        auto& map = global::ENT_TYPE_MAP;

        const auto it = map.find(type);
        if (it == map.end())
        {
            return entt::null; // EntityID not registered
        }

        tickData.lock();
        const auto entity = internal::REGISTRY.create();
        {
            internal::REGISTRY.emplace<PositionC>(entity, x, y, mapID, type); // PositionC is default
            it->second(internal::REGISTRY, entity);
        }
        tickData.unlock();
        if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
        {
            InvokeEvent<onCreate>(entity);
        }
        return entity;
    }

    bool DestroyEntity(const entt::entity entity)
    {
        // M_ASSERT(std::this_thread::get_id() == global::LOGIC_THREAD.get_id(), "Has to be called from the logic thread");
        auto& tickData = global::LOGIC_TICK_DATA;
        if (internal::REGISTRY.valid(entity))
        {
            if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
            {
                InvokeEvent<onDestroy>(entity);
            }
            tickData.lock();
            internal::REGISTRY.destroy(entity);
            tickData.entityUpdateCache.erase(entity);
            std::erase(tickData.drawVec, entity);
            std::erase(tickData.entityUpdateVec, entity);
            tickData.unlock();
            return true;
        }
        return false;
    }

    void GiveCamera(const entt::entity entity) { internal::REGISTRY.emplace<CameraC>(entity); }

    OccluderC& GiveOccluder(const entt::entity entity, const int width, const int height, Shape shape)
    {
        return internal::REGISTRY.emplace<OccluderC>(entity, static_cast<int16_t>(width), static_cast<int16_t>(height),
                                                     shape);
    }

    EmitterC& GiveEmitter(const entt::entity entity, Color color, const int intensity, LightStyle style)
    {
        return internal::REGISTRY.emplace<EmitterC>(entity, color.r, color.g, color.b, color.a, (uint16_t)intensity,
                                                    style);
    }

    void GiveActor(const entt::entity e) { return internal::REGISTRY.emplace<ActorC>(e); }

    auto GiveCollision(const entt::entity e, Shape shape, const int width, const int height, const int anchorX,
                       const int anchorY) -> CollisionC&
    {
        return internal::REGISTRY.emplace<CollisionC>(e, (float)width, (float)height, static_cast<int16_t>(anchorX),
                                                      static_cast<int16_t>(anchorY), (uint8_t)1, shape);
    }

    void GiveScript(const entt::entity e) { internal::REGISTRY.emplace<ScriptC>(e); }

    void GiveDebugVisuals(const entt::entity e)
    {
#ifdef MAGIQUE_DEBUG_ENTITIES
        internal::REGISTRY.emplace<DebugVisualsC>(e);
#else
        LOG_WARNING("Using debug function but not in debug mode!");
#endif
    }

    void GiveDebugController(entt::entity e)
    {
#ifdef MAGIQUE_DEBUG_ENTITIES
        internal::REGISTRY.emplace<DebugControllerC>(e);
#else
        LOG_WARNING("Using debug function but not in debug mode!");
#endif
    }


} // namespace magique