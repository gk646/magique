#include <raylib/raylib.h>

#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>
#include <magique/core/Animations.h>

#include "internal/globals/ECSData.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/DynamicCollisionData.h"
#include "internal/globals/ScriptData.h"
#include "internal/utils/STLUtil.h"

namespace magique
{
    bool RegisterEntity(const EntityType type, const CreateFunc& createFunc)
    {
        MAGIQUE_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ECS_DATA.typeMap;
        if (map.contains(type))
            LOG_WARNING("Overriding existing create function for entity: %d (enum value)", (int)type);
        map[type] = createFunc;

        global::SCRIPT_DATA.padUpToEntity(type); // This assures it's always valid to index with type
        for (auto entity : internal::REGISTRY.view<entt::entity>())
        {
            volatile int b = static_cast<int>(entity); // Try to instantiate all storage types - even in release mode
            b = 5;                                     // Suppress unused variable
        }
        return true;
    }

    bool UnRegisterEntity(const EntityType type)
    {
        MAGIQUE_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ECS_DATA.typeMap;
        if (type == static_cast<EntityType>(UINT16_MAX) || !map.contains(type))
        {
            return false; // Invalid ID or not registered
        }
        map.erase(type);
        return true;
    }

    bool EntityExists(const entt::entity e) { return internal::REGISTRY.valid(e); }

    bool EntityIsActor(const entt::entity entity) { return internal::REGISTRY.all_of<ActorC>(entity); }

    entt::entity CreateEntity(const EntityType type, const float x, const float y, const MapID map)
    {
        MAGIQUE_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        auto& ecs = global::ECS_DATA;
        auto& data = global::ENGINE_DATA;
        const auto it = ecs.typeMap.find(type);
        if (it == ecs.typeMap.end())
        {
            return entt::null; // EntityType not registered
        }
        const auto entity = internal::REGISTRY.create(static_cast<entt::entity>(ecs.entityID++));
        {
            internal::REGISTRY.emplace<PositionC>(entity, x, y, map, type); // PositionC is default
            it->second(entity, type);
        }
        if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
        {
            InvokeEvent<onCreate>(entity);
        }
        if (internal::REGISTRY.all_of<CameraC>(entity)) [[unlikely]]
        {
            data.cameraEntity = entity;
            data.cameraMap = map;
        }
        return entity;
    }

    entt::entity CreateEntityNetwork(entt::entity id, EntityType type, const float x, const float y, MapID map)
    {
        MAGIQUE_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        auto& ecs = global::ECS_DATA;
        const auto it = ecs.typeMap.find(type);
        if (it == ecs.typeMap.end())
        {
            return entt::null; // EntityType not registered
        }
        const auto entity = internal::REGISTRY.create(id);
        {
            internal::REGISTRY.emplace<PositionC>(entity, x, y, map, type); // PositionC is default
            it->second(entity, type);
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
        auto& dynamic = global::DY_COLL_DATA;
        if (internal::REGISTRY.valid(entity)) [[likely]]
        {
            const auto& pos = internal::POSITION_GROUP.get<const PositionC>(entity);
            if (internal::REGISTRY.all_of<ScriptC>(entity)) [[likely]]
            {
                InvokeEvent<onDestroy>(entity);
            }
            if (internal::REGISTRY.all_of<CollisionC>(entity)) [[likely]]
            {
                UnorderedDelete(tickData.collisionVec, entity);
            }
            internal::REGISTRY.destroy(entity);
            tickData.entityUpdateCache.erase(entity);
            if (dynamic.mapEntityGrids.contains(pos.map)) [[likely]]
                dynamic.mapEntityGrids[pos.map].removeWithHoles(entity);
            UnorderedDelete(tickData.drawVec, entity);
            UnorderedDelete(tickData.entityUpdateVec, entity);
            return true;
        }
        return false;
    }

    void DestroyEntities(const std::initializer_list<EntityType>& ids)
    {
        auto& reg = internal::REGISTRY;
        auto& tickData = global::ENGINE_DATA;
        auto& dyCollData = global::DY_COLL_DATA;
        const auto view = reg.view<PositionC>(); // Get all entities
        if (ids.size() == 0)
        {
            for (const auto e : view)
            {
                if (reg.all_of<ScriptC>(e)) [[likely]]
                {
                    InvokeEventDirect<onDestroy>(global::SCRIPT_DATA.scripts[view.get<PositionC>(e).type], e);
                }
            }
            tickData.entityUpdateCache.clear();
            tickData.drawVec.clear();
            tickData.entityUpdateVec.clear();
            tickData.collisionVec.clear();
            dyCollData.mapEntityGrids.clear();
            internal::REGISTRY.clear();
            return;
        }

        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            for (const auto id : ids)
            {
                if (pos.type == id)
                {
                    if (reg.all_of<ScriptC>(e)) [[likely]]
                    {
                        InvokeEventDirect<onDestroy>(global::SCRIPT_DATA.scripts[pos.type], e);
                    }
                    if (reg.all_of<CollisionC>(e)) [[likely]]
                    {
                        UnorderedDelete(tickData.collisionVec, e);
                    }
                    internal::REGISTRY.destroy(e);
                    tickData.entityUpdateCache.erase(e);
                    UnorderedDelete(tickData.drawVec, e);
                    UnorderedDelete(tickData.entityUpdateVec, e);
                    if (dyCollData.mapEntityGrids.contains(pos.map)) [[likely]]
                    {
                        dyCollData.mapEntityGrids[pos.map].removeWithHoles(e);
                    }
                    break;
                }
            }
        }
    }

    CollisionC& GiveCollisionRect(const entt::entity e, const float width, const float height, const int anchorX,
                                  const int anchorY)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, width, height, 0.0F, 0.0F, static_cast<int16_t>(anchorX),
                                                      static_cast<int16_t>(anchorY), DEFAULT_LAYER, Shape::RECT, Point{},
                                                      Point{});
    }

    CollisionC& GiveCollisionCircle(const entt::entity e, const float radius)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, radius, 0.0F, 0.0F, 0.0F, static_cast<int16_t>(0),
                                                      static_cast<int16_t>(0), DEFAULT_LAYER, Shape::CIRCLE);
    }

    CollisionC& GiveCollisionCapsule(const entt::entity e, const float height, const float radius)
    {
        MAGIQUE_ASSERT(height > 2 * radius,
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

    AnimationC& GiveAnimation(const entt::entity entity, const EntityType type, const AnimationState startState)
    {
        const auto& animation = GetEntityAnimation(type);
        return internal::REGISTRY.emplace<AnimationC>(entity, animation, startState);
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

    //----------------- CORE -----------------//

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
            if (cam == e)
            {
                LOG_WARNING("Target entity is already the camera holder!");
                return;
            }
            reg.erase<CameraC>(cam);
            found = true;
        }
        if (found)
        {
            MAGIQUE_ASSERT(!reg.any_of<CameraC>(e), "Target entity cannot have camera component already!");
            reg.emplace<CameraC>(e);
        }
        else
            LOG_ERROR("No existing entity with a camera component found!");
    }

    const std::vector<entt::entity>& GetNearbyEntities(const MapID map, const Point origin, const float radius)
    {
        // This works because the hashset uses a vector as underlying storage type (stored without holes)
        const auto& dynamicData = global::DY_COLL_DATA;
        auto& data = global::ENGINE_DATA;

        if (!dynamicData.mapEntityGrids.contains(map))
            return {};

        if (data.nearbyQueryData.getIsSimilarParameters(map, origin, radius))
            return data.nearbyQueryData.cache.values();

        data.nearbyQueryData.lastRadius = radius;
        data.nearbyQueryData.lastOrigin = origin;
        data.nearbyQueryData.cache.clear();

        const auto queryX = origin.x - radius;
        const auto queryY = origin.y - radius;
        dynamicData.mapEntityGrids[map].query(data.nearbyQueryData.cache, queryX, queryY, radius * 2.0F, radius * 2.0F);
        return data.nearbyQueryData.cache.values();
    }

    bool NearbyEntitiesContain(const MapID map, const Point origin, const float radius, const entt::entity target)
    {
        auto& data = global::ENGINE_DATA;
        if (data.nearbyQueryData.getIsSimilarParameters(map, origin, radius))
            return data.nearbyQueryData.cache.contains(target);
        GetNearbyEntities(map, origin, radius);
        return data.nearbyQueryData.cache.contains(target);
    }

} // namespace magique