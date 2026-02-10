// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>
#include <magique/core/Animations.h>
#include <magique/core/Camera.h>

#include "internal/globals/ECSData.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/EngineConfig.h"
#include "internal/globals/DynamicCollisionData.h"
#include "internal/globals/PathFindingData.h"
#include "internal/globals/ScriptData.h"

namespace magique
{
    bool EntityRegister(const EntityType type, const CreateFunc& createFunc)
    {
        MAGIQUE_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        auto& map = global::ECS_DATA.typeMap;
        if (map.contains(type))
            LOG_WARNING("Overriding existing create function for entity: %d (enum value)", static_cast<int>(type));
        map[type] = createFunc;

        global::SCRIPT_DATA.padUpToEntity(type); // This assures it's always valid to index with type
        // Iterates all entities
        for (auto entity : internal::REGISTRY.view<entt::entity>())
        {
            volatile int b = static_cast<int>(entity); // Try to instantiate all storage types - even in release mode
            (void)b;                                   // Suppress unused variable
        }
        return true;
    }

    bool EntityUnregister(const EntityType type)
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

    void EntitySetCreateCallback(const EntityCallback& callback) { global::ENGINE_DATA.createCallback = callback; }

    bool EntityExists(const entt::entity e) { return internal::REGISTRY.valid(e); }

    bool EntityIsActor(const entt::entity entity) { return internal::REGISTRY.all_of<ActorC>(entity); }

    entt::entity EntityFirstOf(const EntityType type)
    {
        for (const auto entity : internal::REGISTRY.view<entt::entity>())
        {
            const auto& pos = internal::REGISTRY.get<const PositionC>(entity);
            if (pos.type == type)
            {
                return entity;
            }
        }
        return entt::null;
    }

    static entt::entity CreateEntityInternal(const entt::entity id, EntityType type, const Point& pos, const MapID map,
                                             const float rotation, const bool withFunc)
    {
        MAGIQUE_ASSERT(type < static_cast<EntityType>(UINT16_MAX), "Max value is reserved!");
        const auto& config = global::ENGINE_CONFIG;
        auto& ecs = global::ECS_DATA;
        auto& data = global::ENGINE_DATA;
        auto& registry = internal::REGISTRY;

        const auto entity = registry.create(id != entt::null ? id : entt::entity{ecs.entityID++});
        registry.emplace<PositionC>(entity, pos, map, type, rotation); // PositionC is default

        if (withFunc) [[likely]]
        {
            const auto it = ecs.typeMap.find(type);
            if (it == ecs.typeMap.end())
            {
                LOG_ERROR("No method create method registered for that entity type!");
                return entt::null; // EntityType not registered
            }
            it->second(entity, type);
        }

        if (data.createCallback)
            data.createCallback(entity);

        if (!config.isClientMode && data.isEntityScripted(entity)) [[likely]]
        {
            ScriptingInvokeEvent<onCreate>(entity);
        }
        if (registry.all_of<CameraC>(entity)) [[unlikely]]
        {
            data.cameraEntity = entity;
            data.cameraMap = map;
        }
        return entity;
    }

    entt::entity EntityCreate(const EntityType type, Point pos, const MapID map, float rotation, const bool withFunc)
    {
        return CreateEntityInternal(entt::null, type, pos, map, rotation, withFunc);
    }

    entt::entity EntityCreateEx(const entt::entity id, const EntityType type, Point pos, const MapID map,
                                const float rot, const bool withFunc)
    {
        MAGIQUE_ASSERT(!EntityExists(id), "Entity already exists!");
        return CreateEntityInternal(id, type, pos, map, rot, withFunc);
    }

    void EntitySetDestroyCallback(const EntityCallback& callback) { global::ENGINE_DATA.destroyCallback = callback; }

    bool EntityDestroy(const entt::entity entity)
    {
        const auto& config = global::ENGINE_CONFIG;
        auto& data = global::ENGINE_DATA;
        auto& dynamic = global::DY_COLL_DATA;
        auto& registry = internal::REGISTRY;
        if (registry.valid(entity)) [[likely]]
        {
            const auto& pos = internal::POSITION_GROUP.get<const PositionC>(entity);
            if (data.destroyCallback)
            {
                data.destroyCallback(entity);
            }
            if (!config.isClientMode && data.isEntityScripted(entity)) [[likely]]
            {
                ScriptingInvokeEventDirect<onDestroy>(global::SCRIPT_DATA.scripts[pos.type], entity);
            }

            data.entityUpdateCache.erase(entity);
            std::erase(data.drawVec, entity);
            std::erase(data.entityUpdateVec, entity);
            std::erase(data.collisionVec, entity);
            data.entityNScriptedSet.erase(entity);
            dynamic.mapEntityGrids[pos.map].removeWithHoles(entity);
            global::PATH_DATA.solidEntities.erase(entity);
            if (entity == CameraGetEntity())
            {
                data.cameraEntity = entt::null;
            }
            registry.destroy(entity);
            return true;
        }
        return false;
    }

    void EntityDestroy(const std::initializer_list<EntityType>& ids)
    {
        const auto& config = global::ENGINE_CONFIG;
        auto& data = global::ENGINE_DATA;
        auto& dyCollData = global::DY_COLL_DATA;

        const auto& group = internal::POSITION_GROUP; // Get all entities
        if (ids.size() == 0)
        {
            for (const auto e : group)
            {
                const auto& pos = group.get<PositionC>(e);
                if (data.destroyCallback)
                {
                    data.destroyCallback(e);
                }
                if (!config.isClientMode && data.isEntityScripted(e)) [[likely]]
                {
                    ScriptingInvokeEventDirect<onDestroy>(global::SCRIPT_DATA.scripts[pos.type], e);
                }
            }

            data.entityUpdateCache.clear();
            data.drawVec.clear();
            data.entityUpdateVec.clear();
            data.collisionVec.clear();
            data.entityNScriptedSet.clear();
            dyCollData.mapEntityGrids.clear();
            internal::REGISTRY.clear();
            global::PATH_DATA.solidEntities.clear();
            data.cameraEntity = entt::null;
            return;
        }

        for (const auto e : internal::REGISTRY.view<entt::entity>())
        {
            const auto& pos = group.get<PositionC>(e);
            for (const auto id : ids)
            {
                if (pos.type == id)
                {
                    EntityDestroy(e);
                    break;
                }
            }
        }
        // Don't need to patch as its cleared each tick
    }

    void EntityDestroy(const std::function<bool(entt::entity)>& func)
    {
        for (const auto e : internal::REGISTRY.view<entt::entity>())
        {
            if (func(e))
            {
                EntityDestroy(e);
            }
        }
    }

    CollisionC& GiveCollisionRect(entt::entity entity, float width, float height)
    {
        const auto size = Point{width, height};
        return GiveCollisionRect(entity, Rect{{0, 0}, size}, size / 2);
    }

    CollisionC& GiveCollisionRect(entt::entity entity, Point dims, Point anchor)
    {
        return GiveCollisionRect(entity, Rect{{0, 0}, dims}, anchor);
    }

    CollisionC& GiveCollisionRect(entt::entity entity, Rect rect, Point anchor)
    {
        return internal::REGISTRY.emplace<CollisionC>(entity, rect.w, rect.h, 0.0F, 0.0F, rect.pos(), anchor,
                                                      Shape::RECT);
    }

    CollisionC& GiveCollisionCircle(const entt::entity e, const float radius)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, radius, radius, 0.0F, 0.0F, Point{}, Point{}, Shape::CIRCLE);
    }

    CollisionC& GiveCollisionTri(const entt::entity e, const Point p2, const Point p3, Point anchor)
    {
        return internal::REGISTRY.emplace<CollisionC>(e, p2.x, p2.y, p3.x, p3.y, Point{}, anchor, Shape::TRIANGLE);
    }

    EmitterC& GiveEmitter(const entt::entity entity, const Color color, const int intensity, LightStyle style)
    {
        return internal::REGISTRY.emplace<EmitterC>(entity, static_cast<uint16_t>(intensity), color.r, color.g, color.b,
                                                    color.a, style, true);
    }

    OccluderC& GiveOccluder(const entt::entity entity, const int width, const int height, const Shape shape)
    {
        return internal::REGISTRY.emplace<OccluderC>(entity, static_cast<int16_t>(width), static_cast<int16_t>(height),
                                                     shape);
    }

    void GiveCamera(const entt::entity entity)
    {
        internal::REGISTRY.emplace<CameraC>(entity);
        global::ENGINE_DATA.cameraMap = ComponentGet<const PositionC>(entity).map;
    }

    void GiveActor(const entt::entity e) { internal::REGISTRY.emplace<ActorC>(e); }

    //----------------- CORE -----------------//

    void CameraSetEntity(const entt::entity target)
    {
        auto& reg = internal::REGISTRY;
        if (!reg.valid(target))
        {
            LOG_WARNING("Trying to assign camera to invalid entity: %d", static_cast<int>(target));
            return;
        }

        if (EntityHasComponents<CameraC>(target))
        {
            LOG_WARNING("Target entity is already the camera holder!");
            return;
        }

        ComponentRemove<CameraC>(CameraGetEntity());
        reg.emplace<CameraC>(target);
    }

    static HashSet<entt::entity>& QueryLoadedEntitiesRectIMPL(MapID map, Point origin, Point size)
    {
        auto& dynamicData = global::DY_COLL_DATA;
        auto& set = global::ENGINE_DATA.queryCache;
        set.clear();
        dynamicData.mapEntityGrids[map].query(set, origin.x, origin.y, size.x, size.y);
        return set;
    }

    const std::vector<entt::entity>& QueryLoadedEntitiesCircle(MapID map, Point origin, float size,
                                                               const FilterFunc& filter)
    {
        QueryLoadedEntitiesRectIMPL(map, origin - size, Point{size * 2});
        auto& set = global::ENGINE_DATA.queryCache;
        for (auto it = set.begin(); it != set.end();)
        {
            if (CollisionC::GetMiddle(*it).euclidean(origin) > size || (filter && !filter(*it)))
            {
                it = set.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return set.values();
    }

    const std::vector<entt::entity>& QueryLoadedEntitiesRect(MapID map, Point origin, Point size,
                                                             const FilterFunc& filter)
    {
        auto& set = global::ENGINE_DATA.queryCache;
        QueryLoadedEntitiesRectIMPL(map, origin, size);
        const Rect bounds{origin, size};
        for (auto it = set.begin(); it != set.end();)
        {
            if (!bounds.contains(CollisionC::GetMiddle(*it)) || (filter && !filter(*it)))
            {
                it = set.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return set.values();
    }

    const std::vector<entt::entity>& QueryEntitiesCircle(MapID map, Point origin, float size, const FilterFunc& filter)
    {
        auto& set = global::ENGINE_DATA.queryCache;
        set.clear();
        for (const auto e : ComponentGetView<PositionC>())
        {
            const auto& pos = ComponentGet<PositionC>(e);
            if (pos.map != map || pos.pos.euclidean(origin) > size || (filter && !filter(e))) [[likely]]
            {
                continue;
            }
            set.insert(e);
        }
        return set.values();
    }

    const std::vector<entt::entity>& QueryEntitiesRect(MapID map, Point origin, Point size, const FilterFunc& filter)
    {
        auto& set = global::ENGINE_DATA.queryCache;
        set.clear();
        const Rect bounds{origin, size};
        for (const auto e : ComponentGetView<PositionC>())
        {
            const auto& pos = ComponentGet<PositionC>(e);
            if (pos.map != map || !bounds.contains(pos.pos) || (filter && !filter(e))) [[likely]]
            {
                continue;
            }
            set.insert(e);
        }
        return set.values();
    }

} // namespace magique
