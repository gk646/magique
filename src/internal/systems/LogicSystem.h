// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOGIC_SYSTEM_H
#define MAGIQUE_LOGIC_SYSTEM_H

namespace magique
{
    // Flattened array
    struct ActorMapDistribution final
    {
        static constexpr int maxPlayers = MAGIQUE_MAX_PLAYERS;
        cxstructs::SmallVector<int8_t, MAGIQUE_EXPECTED_MAPS * maxPlayers> dataVec;

        ActorMapDistribution() { dataVec.resize(MAGIQUE_EXPECTED_MAPS * maxPlayers, -1); }

        int8_t getActorNum(const MapID map, const int offset)
        {
            return dataVec[(static_cast<int>(map) * MAGIQUE_MAX_PLAYERS) + offset];
        }

        void insertActorNum(const MapID map, const int num)
        {
            dataVec.resize(static_cast<int>(map) * maxPlayers, -1);
            for (int i = 0; i < maxPlayers; i++)
            {
                if (dataVec[(maxPlayers * static_cast<int>(map)) + i] == -1)
                {
                    dataVec[(maxPlayers * static_cast<int>(map)) + i] = static_cast<int8_t>(num);
                    return;
                }
            }
        }
    };


    inline void HandleCollisionEntity(const entt::entity e, const PositionC pos, const CollisionC& col,
                                      EntityHashGrid& grid, vector<entt::entity>& cVec)
    {
        auto& pathData = global::PATH_DATA;
        auto& dynamicGrid = pathData.mapsDynamicGrids[pos.map]; // Must exist - checked in CreateEntity()
        const auto isPathSolid = pathData.getIsPathSolid(e, pos.type);

        cVec.push_back(e);
        switch (col.shape) // Same as CollisionSystem::QueryHashGrid()
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    grid.insert(e, pos.x, pos.y, col.p1, col.p2);
                    if (isPathSolid) [[unlikely]]
                        dynamicGrid.insert(pos.x, pos.y, col.p1, col.p2);
                    break;
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                grid.insert(e, bb.x, bb.y, bb.width, bb.height);
                if (isPathSolid) [[unlikely]]
                    dynamicGrid.insert(bb.x, bb.y, bb.width, bb.height);
            }
            break;
        case Shape::CIRCLE:
            grid.insert(e, pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F); // Top left and diameter as w and h
            if (isPathSolid) [[unlikely]]
                dynamicGrid.insert(pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F);
            break;
        case Shape::CAPSULE:
            grid.insert(e, pos.x, pos.y, col.p1 * 2.0F, col.p2); // Top left and height as height / diameter as w
            if (isPathSolid) [[unlikely]]
                dynamicGrid.insert(pos.x, pos.y, col.p1 * 2.0F, col.p2);
            break;
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    const auto bb =
                        GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3, pos.y + col.p4);
                    grid.insert(e, bb.x, bb.y, bb.width, bb.height);
                    if (isPathSolid) [[unlikely]]
                        dynamicGrid.insert(bb.x, bb.y, bb.width, bb.height);
                    break;
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                grid.insert(e, bb.x, bb.y, bb.width, bb.height);
                if (isPathSolid) [[unlikely]]
                    dynamicGrid.insert(bb.x, bb.y, bb.width, bb.height);
            }
            break;
        }
    }

    inline void AssignCameraData(const entt::registry& registry)
    {
        const auto view = registry.view<const CameraC, const PositionC>();
        auto& tickData = global::ENGINE_DATA;
#ifdef MAGIQUE_DEBUG
        int count = 0;
#endif
        const auto sWidth = static_cast<float>(GetScreenWidth());
        const auto sHeight = static_cast<float>(GetScreenHeight());
        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            tickData.cameraMap = pos.map;
            tickData.cameraEntity = e;
            tickData.camera.offset = {std::floor(sWidth / 2.0F), std::floor(sHeight / 2.0F)};
            const auto manualOff = global::ENGINE_CONFIG.cameraViewOff;
            if (manualOff.x != 0 || manualOff.y != 0) // Use the custom offset if supplied
            {
                tickData.camera.offset = manualOff;
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
    inline void BuildCache(Vector2 (&actorCircles)[MAGIQUE_MAX_PLAYERS], bool (&actorMaps)[UINT8_MAX],
                           ActorMapDistribution& actorDist, int& actorCount)
    {
        const auto& registry = internal::REGISTRY;
        const auto& config = global::ENGINE_CONFIG;
        const auto view = registry.view<const ActorC, const PositionC>();
        for (const auto actor : view)
        {
            MAGIQUE_ASSERT(actorCount < MAGIQUE_MAX_PLAYERS, "More actors than configured!");
            const auto& pos = view.get<const PositionC>(actor);
            actorMaps[static_cast<int>(pos.map)] = true;
            actorCircles[actorCount] = {pos.x - config.entityUpdateDistance, pos.y - config.entityUpdateDistance};
            actorDist.insertActorNum(pos.map, actorCount);
            actorCount++;
        }
    }

    inline void IterateEntities(const entt::registry& registry)
    {
        auto& data = global::ENGINE_DATA;
        auto& pathData = global::PATH_DATA;
        auto& config = global::ENGINE_CONFIG;
        const auto& group = internal::POSITION_GROUP;
        auto& dynamicData = global::DY_COLL_DATA;

        auto& drawVec = data.drawVec;
        auto& cache = data.entityUpdateCache;
        auto& collisionVec = data.collisionVec;
        auto& loadedMaps = data.loadedMaps;

        // Cache
        const auto updateDist = config.entityUpdateDistance * 2.0F;
        const uint16_t cacheDuration = config.entityCacheDuration;
        int actorCount = 0;
        const auto cameraMap = data.cameraMap;
        const auto camBound = GetCameraBounds();

        // Lookup tables
        ActorMapDistribution actorDist{};
        Vector2 updateOffsets[MAGIQUE_MAX_PLAYERS];
        bool loadedMapsTable[UINT8_MAX]{};
        bool actorMapsTable[UINT8_MAX]{};

        BuildCache(updateOffsets, actorMapsTable, actorDist, actorCount);

        // Iterate all entities and insert them into the hashgrids and drawVec/collisionVec
        const auto view = registry.view<PositionC>();
        for (const auto e : view)
        {
            const auto& pos = group.get<const PositionC>(e);
            const auto map = pos.map;

            loadedMapsTable[static_cast<int>(map)] = true;

            // Sadly have to check that - could be that an entity just switched layer
            if (!dynamicData.mapEntityGrids.contains(map)) [[unlikely]]
                dynamicData.mapEntityGrids.add(map);

            if (!pathData.mapsDynamicGrids.contains(map)) [[unlikely]]
                pathData.mapsDynamicGrids.add(map);

            auto& hashGrid = dynamicData.mapEntityGrids[map];

            if (actorMapsTable[static_cast<int>(map)]) [[likely]] // entity is in any map where at least 1 actor is
            {
                // Check if inside the camera bounds already
                if (map == cameraMap &&
                    PointToRect(pos.x, pos.y, camBound.x, camBound.y, camBound.width, camBound.height))
                {
                    drawVec.push_back(e); // Should be drawn
                    cache[e] = cacheDuration;
                    if (registry.all_of<CollisionC>(e))
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
                            break;

                        const auto [x, y] = updateOffsets[actorNum];
                        // Check if inside any update rect - rect is an enlarged rectangle
                        if (PointToRect(pos.x, pos.y, x, y, updateDist, updateDist))
                        {
                            cache[e] = cacheDuration;
                            if (registry.all_of<CollisionC>(e))
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
            if (loadedMapsTable[i])
                loadedMaps.push_back(static_cast<MapID>(i));
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
        IterateEntities(registry);

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

        if (config.isClientMode) // Skip script method in client mode
            return;

        for (const auto pair : data.entityUpdateCache)
        {
            // Invoke tick event on all entities that are in this tick
            InvokeEvent<onTick>(pair.first, data.isEntityScripted(pair.first));
        }
    }

} // namespace magique

#endif //MAGIQUE_LOGIC_SYSTEM_H