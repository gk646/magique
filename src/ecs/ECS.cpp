#include <raylib/raylib.h>

#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>

#include "internal/globals/ECSData.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/ScriptData.h"

namespace magique
{
    bool RegisterEntity(const EntityID type, const CreateFunc& createFunc)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ECS_DATA.typeMap;
        if (type == static_cast<EntityID>(UINT16_MAX) || map.contains(type))
        {
            return false; // Invalid ID or already registered
        }

        map.insert({type, createFunc});
        global::SCRIPT_DATA.padUpToEntity(type); // This assures its always valid to index with type
        for (auto entity : internal::REGISTRY.view<entt::entity>())
        {
            volatile int b = static_cast<int>(entity); // Try to instantiate all storage types - even in release mode
            b = 5;                                     // Suppress C4126
        }
        return true;
    }

    bool UnRegisterEntity(const EntityID type)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ECS_DATA.typeMap;
        if (type == static_cast<EntityID>(UINT16_MAX) || !map.contains(type))
        {
            return false; // Invalid ID or not registered
        }
        map.erase(type);
        return true;
    }

    bool IsEntityExisting(const entt::entity e) { return internal::REGISTRY.valid(e); }

    entt::entity CreateEntity(const EntityID type, const float x, const float y, const MapID map)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& ecs = global::ECS_DATA;
        const auto it = ecs.typeMap.find(type);
        if (it == ecs.typeMap.end())
        {
            return entt::null; // EntityID not registered
        }
        const auto entity = internal::REGISTRY.create(static_cast<entt::entity>(ecs.entityID++));
        {
            internal::REGISTRY.emplace<PositionC>(entity, x, y, map, type); // PositionC is default
            it->second(entity);
        }
        if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
        {
            InvokeEvent<onCreate>(entity);
        }
        return entity;
    }

    entt::entity CreateEntityNetwork(uint32_t id, EntityID type, const float x, const float y, MapID map)
    {
        M_ASSERT(type < static_cast<EntityID>(UINT16_MAX), "Max value is reserved!");
        auto& ecs = global::ECS_DATA;
        const auto it = ecs.typeMap.find(type);
        if (it == ecs.typeMap.end())
        {
            return entt::null; // EntityID not registered
        }
        const auto entity = internal::REGISTRY.create(static_cast<entt::entity>(id));
        {
            internal::REGISTRY.emplace<PositionC>(entity, x, y, map, type); // PositionC is default
            it->second(entity);
        }
        if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
        {
            InvokeEvent<onCreate>(entity);
        }
        return entity;
    }

    bool DestroyEntity(const entt::entity entity)
    {
        auto& tickData = global::ENGINE_DATA;
        if (internal::REGISTRY.valid(entity)) [[likely]]
        {
            if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
            {
                InvokeEvent<onDestroy>(entity);
            }
            internal::REGISTRY.destroy(entity);
            tickData.entityUpdateCache.erase(entity);
            std::erase(tickData.drawVec, entity);
            std::erase(tickData.entityUpdateVec, entity);
            return true;
        }
        return false;
    }

    CollisionC& GiveCollisionRect(const entt::entity e, const float width, const float height, const int anchorX,
                                  const int anchorY)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, width, height, 0.0F, 0.0F, static_cast<int16_t>(anchorX),
                                                      static_cast<int16_t>(anchorY), DEFAULT_LAYER, Shape::RECT);
    }

    CollisionC& GiveCollisionCircle(const entt::entity e, const float radius)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, radius, 0.0F, 0.0F, 0.0F, static_cast<int16_t>(0),
                                                      static_cast<int16_t>(0), DEFAULT_LAYER, Shape::CIRCLE);
    }

    CollisionC& GiveCollisionCapsule(const entt::entity e, const float height, const float radius)
    {
        M_ASSERT(height > 2 * radius,
                 "Given capsule is not well defined! Total height has to be greater than 2 * radius");
        return internal::REGISTRY.emplace<CollisionC>(e, radius, height, 0.0F, 0.0F, static_cast<int16_t>(0),
                                                      static_cast<int16_t>(0), DEFAULT_LAYER, Shape::CAPSULE);
    }

    CollisionC& GiveCollisionTri(const entt::entity e, const Point p2, const Point p3, const int anchorX,
                                 const int anchorY)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, p2.x, p2.y, p3.x, p3.y, static_cast<int16_t>(anchorX),
                                                      static_cast<int16_t>(anchorY), DEFAULT_LAYER, Shape::TRIANGLE);
    }

    void GiveCamera(const entt::entity entity) { internal::REGISTRY.emplace<CameraC>(entity); }

    OccluderC& GiveOccluder(const entt::entity entity, const int width, const int height, const Shape shape)
    {
        return internal::REGISTRY.emplace<OccluderC>(entity, static_cast<int16_t>(width), static_cast<int16_t>(height),
                                                     shape);
    }

    EmitterC& GiveEmitter(const entt::entity entity, const Color color, const int intensity, LightStyle style)
    {
        return internal::REGISTRY.emplace<EmitterC>(entity, color.r, color.g, color.b, color.a, (uint16_t)intensity,
                                                    style);
    }

    void GiveActor(const entt::entity e) { return internal::REGISTRY.emplace<ActorC>(e); }

    void GiveScript(const entt::entity e) { internal::REGISTRY.emplace<ScriptC>(e); }

    void SetCameraEntity(const entt::entity e)
    {
        auto& reg = internal::REGISTRY;
        if (!reg.valid(e))
        {
            LOG_WARNING("Trying to assign camera to invalid entity: %d", static_cast<int>(e));
            return;
        }
        const auto view = reg.view<CameraC>();
        bool found = false;
        for (const auto cam : view)
        {
            reg.erase<CameraC>(cam);
            found = true;
        }
        if (found)
            reg.emplace<CameraC>(e);
        else
            LOG_ERROR("No existing entity with a camera component found!");
    }

} // namespace magique