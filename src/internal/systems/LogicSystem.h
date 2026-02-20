// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOGIC_SYSTEM_H
#define MAGIQUE_LOGIC_SYSTEM_H

namespace magique
{
    // Flattened array
    struct ActorMapDistribution final
    {
        static constexpr int8_t EMPTY_VALUE = -1;
        std::vector<int8_t> dataVec;

        ActorMapDistribution() { dataVec.resize(32 * MAGIQUE_MAX_PLAYERS, EMPTY_VALUE); }

        int8_t getActorNum(const MapID map, const int offset) const
        {
            return dataVec[(static_cast<int>(map) * MAGIQUE_MAX_PLAYERS) + offset];
        }

        void insertActorNum(const MapID map, const int num)
        {
            if ((int)dataVec.size() < static_cast<int>(map) * MAGIQUE_MAX_PLAYERS)
                dataVec.resize(static_cast<int>(map) * MAGIQUE_MAX_PLAYERS, EMPTY_VALUE);
            for (int i = 0; i < MAGIQUE_MAX_PLAYERS; i++)
            {
                if (dataVec[(MAGIQUE_MAX_PLAYERS * static_cast<int>(map)) + i] == EMPTY_VALUE)
                {
                    dataVec[(MAGIQUE_MAX_PLAYERS * static_cast<int>(map)) + i] = static_cast<int8_t>(num);
                    return;
                }
            }
        }
    };

    using ActorRectsTable = std::array<Rect, MAGIQUE_MAX_PLAYERS>;
    using ActorMapsTable = std::array<bool, UINT8_MAX>;

    inline void HandleCollisionEntity(const entt::entity e, const PositionC pos, const CollisionC& col,
                                      EntityHashGrid& grid, std::vector<entt::entity>& cVec)
    {
        auto& pathData = global::PATH_DATA;
        auto& pathGrid = pathData.mapsDynamicGrids[pos.map]; // Must exist - check in loop before
        const auto isPathSolid = pathData.getIsPathSolid(e, pos.type);

        cVec.push_back(e);
        const auto bb = pos.getBounds(col);
        grid.insert(e, bb.x, bb.y, bb.width, bb.height);
        if (isPathSolid) [[unlikely]]
        {
            pathGrid.insert(bb.x, bb.y, bb.width, bb.height);
        }
    }

    inline void AssignCameraData(const entt::registry& registry)
    {
        const auto view = registry.view<const CameraC, const PositionC>();
        auto& data = global::ENGINE_DATA;
#ifdef MAGIQUE_DEBUG
        int count = 0;
#endif
        data.camera.offset = Vector2{(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};
        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            data.cameraMap = pos.map;
            data.cameraEntity = e;
            const auto manualOff = global::ENGINE_CONFIG.cameraViewOff;
            if (manualOff != 0) // Use the custom offset if supplied
            {
                data.camera.offset = manualOff;
            }
#ifdef MAGIQUE_DEBUG
            count++;
#endif
        }
        // Center the camera
#ifdef MAGIQUE_DEBUG
        MAGIQUE_ASSERT(count < 2, "You have multiple cameras? O.O");
#endif
    }

    // Builds the lookup tables so the main iteration is as fast as possible
    inline void BuildCache(ActorRectsTable& actorRects, ActorMapsTable& actorMaps, ActorMapDistribution& actorDist,
                           int& actorCount)
    {
        const auto& registry = internal::REGISTRY;
        const auto updateDist = global::ENGINE_CONFIG.entityUpdateDist;
        const auto view = registry.view<const ActorC, const PositionC>();

        for (const auto actor : view)
        {
            MAGIQUE_ASSERT(actorCount < MAGIQUE_MAX_PLAYERS, "More actors than configured!");
            const auto& pos = view.get<const PositionC>(actor);
            actorMaps[static_cast<int>(pos.map)] = true;
            actorRects[actorCount] = Rect::CenteredOn(pos.pos, updateDist);
            actorDist.insertActorNum(pos.map, actorCount);
            actorCount++;
        }
    }

    inline void IterateEntities()
    {
        const auto& registry = internal::REGISTRY;
        auto& data = global::ENGINE_DATA;
        auto& config = global::ENGINE_CONFIG;
        const auto& group = internal::POSITION_GROUP;
        auto& dynamicData = global::DY_COLL_DATA;

        auto& drawVec = data.drawVec;
        auto& cache = data.entityUpdateCache;
        auto& collisionVec = data.collisionVec;

        // Cache
        const uint16_t cacheDuration = config.entityCacheDuration;
        int actorCount = 0;
        const auto cameraMap = data.cameraMap;
        const Rect camBound = CameraGetBounds();

        // Lookup tables
        ActorMapDistribution actorDist{};
        ActorRectsTable actorRects{};
        ActorMapsTable actorMaps{};
        std::array<bool, UINT8_MAX> loadedMaps{};

        BuildCache(actorRects, actorMaps, actorDist, actorCount);

        // Iterate all entities and insert them into the hashgrids and drawVec/collisionVec
        const auto view = registry.view<const PositionC>();
        for (const auto e : view)
        {
            const auto& pos = group.get<const PositionC>(e);
            const auto map = pos.map;

            loadedMaps[static_cast<int>(map)] = true;
            auto& hashGrid = dynamicData.mapEntityGrids[map];

            if (loadedMaps[static_cast<int>(map)]) [[likely]] // entity is in any map where at least 1 actor is
            {
                // Check if inside the camera bounds already
                if (map == cameraMap && camBound.contains(pos.pos))
                {
                    drawVec.push_back(e); // Should be drawn
                    cache[e] = cacheDuration;
                    if (group.contains(e))
                    {
                        auto& col = group.get<CollisionC>(e);
                        HandleCollisionEntity(e, pos, col, hashGrid, collisionVec);
                    }
                }
                else
                {
                    for (int i = 0; i < actorCount; ++i) // Iterate through the different actors within the map
                    {
                        const int8_t actorNum = actorDist.getActorNum(map, i);
                        if (actorNum == -1) // No more actors in that map
                        {
                            break;
                        }
                        // Check if inside any update rect - rect is an enlarged rectangle
                        if (actorRects[actorNum].contains(pos.pos))
                        {
                            cache[e] = cacheDuration;
                            if (group.contains(e))
                            {
                                auto& col = group.get<CollisionC>(e);
                                HandleCollisionEntity(e, pos, col, hashGrid, collisionVec);
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Generate a dense vector of the loaded maps - map is loaded if it contains at least 1 entity
        for (int i = 0; i < UINT8_MAX; ++i)
        {
            if (loadedMaps[i])
            {
                data.loadedMaps.push_back(static_cast<MapID>(i));
            }
        }
    }

    inline void LogicSystem(const entt::registry& registry)
    {
        auto& data = global::ENGINE_DATA;
        auto& dynamicData = global::DY_COLL_DATA;
        auto& pathData = global::PATH_DATA;
        auto& config = global::ENGINE_CONFIG;

        auto& drawVec = data.drawVec;
        auto& cache = data.entityUpdateCache;
        auto& updateVec = data.entityUpdateVec;
        auto& collisionVec = data.collisionVec;
        auto& loadedMaps = data.loadedMaps;

        AssignCameraData(registry);

        // Clear data
        loadedMaps.clear();                 // Loaded maps vector
        drawVec.clear();                    // Drawn entities
        updateVec.clear();                  // Update entities
        collisionVec.clear();               // Collision entities
        dynamicData.mapEntityGrids.clear(); // Collision entity hashgrid
        pathData.mapsDynamicGrids.clear();  // Pathfinding solid entities hashgrid

        // Iterates all entities
        IterateEntities();

        // Fill the update vec after to avoid adding entities that drop out
        for (auto it = cache.begin(); it != cache.end();)
        {
            if (it->second > 0)
            {
                it->second--;
                updateVec.push_back(it->first);
                ++it;
            }
            else
            {
                it = cache.erase(it);
            }
        }

        if (config.isClientMode) // Skip script methods in client mode
        {
            return;
        }

        // Iterates all entities
        for (const auto entity : EntityGetRegistry().view<entt::entity>())
        {
            // Invoke tick event on all entities that are in this tick and are scripted
            if (data.isEntityScripted(entity)) [[likely]]
            {
                // Pass a boolean whether the entity is updated => if it's in the cache
                ScriptingInvokeEvent<onTick>(entity, cache.contains(entity));
            }
        }
    }

} // namespace magique

#endif // MAGIQUE_LOGIC_SYSTEM_H
