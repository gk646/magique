#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

namespace magique
{
    inline Vector3 GetUpdateCircle(const float x, const float y)
    {
        return {x, y, global::ENGINE_CONFIG.entityUpdateDistance};
    }

    // Insert numbers into flattened array
    inline void
    InsertToActorDist(cxstructs::SmallVector<int8_t, MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS>& actorDist,
                      const int map, const int num)
    {
        for (int i = 0; i < 4; i++)
        {
            if (actorDist[MAGIQUE_MAX_PLAYERS * map + i] == -1)
            {
                actorDist[MAGIQUE_MAX_PLAYERS * map + i] = static_cast<int8_t>(num);
                return;
            }
        }
    }

    // This is a bit complicated as we dont know how many total maps there are
    // So we stay flexible with sbo vectors that expand if needed
    inline void BuildCache(const entt::registry& registry, std::array<MapID, MAGIQUE_MAX_PLAYERS>& loadedMaps,
                           Vector3 (&actorCircles)[4],
                           cxstructs::SmallVector<bool, MAGIQUE_MAX_EXPECTED_MAPS>& actorMaps,
                           cxstructs::SmallVector<int8_t, MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS>& actorDist,
                           int& actorCount)
    {
        std::memset(loadedMaps.data(), UINT8_MAX, MAGIQUE_MAX_PLAYERS);
        actorDist.resize(MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS, -1);
        actorMaps.resize(MAGIQUE_MAX_EXPECTED_MAPS, false);

        const auto view = registry.view<const ActorC, const PositionC>();
        for (const auto actor : view)
        {
            ASSERT(actorCount < MAGIQUE_MAX_PLAYERS, "More actors than configured!");
            const auto& pos = view.get<const PositionC>(actor);
            actorDist.resize(static_cast<int>(pos.map), -1);    // initializes new values to -1
            actorMaps.resize(static_cast<int>(pos.map), false); // initializes new values to false

            actorMaps[static_cast<int>(pos.map)] = true;
            loadedMaps[actorCount] = pos.map;
            actorCircles[actorCount] = GetUpdateCircle(pos.x, pos.y);
            InsertToActorDist(actorDist, static_cast<int>(pos.map), actorCount);
            actorCount++;
        }
    }

    void HandleCollisionEntity(const entt::entity e, const PositionC pos, const CollisionC& col, auto& grid, auto& cVec)
    {
        cVec.push_back(e);
        switch (col.shape) // Same as CollisionSystem::QueryHashGrid()
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return grid.insert(e, pos.x, pos.y, col.p1, col.p2);
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                return grid.insert(e, bb.x, bb.y, bb.width, bb.height);
            }
        case Shape::CIRCLE:
            return grid.insert(e, pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F); // Top left and diameter as w and h
        case Shape::CAPSULE:
            // Top left and height as height / diameter as w
            return grid.insert(e, pos.x, pos.y, col.p1 * 2.0F, col.p2);
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    const auto bb = GetBBTriangle(pos.x, pos.y, col.p1, col.p2, col.p3, col.p4);
                    return grid.insert(e, bb.x, bb.y, bb.width, bb.height);
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                return grid.insert(e, bb.x, bb.y, bb.width, bb.height);
            }
        }
    }

    inline void AssignCameraData(const entt::registry& registry)
    {
        const auto view = registry.view<const CameraC, const PositionC>();
        auto& tickData = global::ENGINE_DATA;
#if MAGIQUE_DEBUG == 1
        int count = 0;
#endif
        const auto sWidth = static_cast<float>(CORE.Window.screen.width);
        const auto sHeight = static_cast<float>(CORE.Window.screen.height);
        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            tickData.cameraMap = pos.map;
            tickData.cameraEntity = e;
            tickData.camera.target = {pos.x, pos.y};
            tickData.camera.offset = {sWidth / 2.0F, sHeight / 2.0F};
            const auto coll = internal::REGISTRY.try_get<CollisionC>(e);
            if (coll) [[likely]]
            {
                switch (coll->shape)
                {
                case Shape::RECT:
                    tickData.camera.target.x += coll->p1 / 2.0F;
                    tickData.camera.target.y += coll->p2 / 2.0F;
                    break;
                case Shape::CIRCLE:
                    tickData.camera.target.x += coll->p1;
                    tickData.camera.target.y += coll->p1;
                    break;
                case Shape::CAPSULE:
                    tickData.camera.target.x += coll->p1;
                    tickData.camera.target.y += coll->p2 / 2.0F;
                    break;
                case Shape::TRIANGLE:
                    break;
                }
            }

            const auto manualOff = global::ENGINE_CONFIG.manualCamOff;
            if (manualOff.x != 0 || manualOff.y != 0) // Use the custom offset if supplied
            {
                tickData.camera.offset = manualOff;
            }
#if MAGIQUE_DEBUG == 1
            count++;
#endif
        }
        // Center the camera
#if MAGIQUE_DEBUG == 1
        ASSERT(count < 2, "You have multiple cameras? O.O");
#endif
    }

    inline void LogicSystem(const entt::registry& registry)
    {
        auto& tickData = global::ENGINE_DATA;
        auto& hashGrid = tickData.hashGrid;
        auto& drawVec = tickData.drawVec;
        auto& cache = tickData.entityUpdateCache;
        auto& updateVec = tickData.entityUpdateVec;
        auto& collisionVec = tickData.collisionVec;
        auto& loadedMaps = tickData.loadedMaps;

        AssignCameraData(registry);

        // Cache
        const auto cameraMap = tickData.cameraMap;
        const uint16_t cacheDuration = global::ENGINE_CONFIG.entityCacheDuration;
        const auto camBound = GetCameraBounds();
        int actorCount = 0;

        // Lookup tables
        // Dist is just a flattened array: int [Count Maps][Count Players]
        cxstructs::SmallVector<int8_t, MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS> actorDistribution{};
        cxstructs::SmallVector<bool, MAGIQUE_MAX_EXPECTED_MAPS> actorMaps{};
        Vector3 actorCircles[MAGIQUE_MAX_PLAYERS];

        BuildCache(registry, loadedMaps, actorCircles, actorMaps, actorDistribution, actorCount);
        {
            drawVec.clear();
            hashGrid.clear();
            updateVec.clear();
            collisionVec.clear();

            const auto view = registry.view<const PositionC>();
            for (const auto e : view)
            {
                const auto& pos = view.get<const PositionC>(e);
                const auto map = pos.map;
                if (actorMaps[static_cast<int>(map)]) [[likely]] // entity is in any map where at least 1 actor is
                {
                    // Check if inside the camera bounds already
                    if (map == cameraMap &&
                        PointInRect(pos.x, pos.y, camBound.x, camBound.y, camBound.width, camBound.height))
                    {
                        drawVec.push_back(e); // Should be drawn
                        cache[e] = cacheDuration;
                        const auto collision = registry.try_get<CollisionC>(e);
                        if (collision != nullptr) [[likely]]
                            HandleCollisionEntity(e, pos, *collision, hashGrid, collisionVec);
                    }
                    else
                    {
                        for (int i = 0; i < actorCount; ++i)
                        {
                            const int8_t actorNum = actorDistribution[static_cast<int>(map) * MAGIQUE_MAX_PLAYERS];
                            if (actorNum == -1)
                                break;
                            const auto [x, y, z] = actorCircles[actorNum];
                            // Check if insdie any update circle
                            if (PointInRect(pos.x, pos.y, x, y, z, z))
                            {
                                cache[e] = cacheDuration;
                                const auto collision = registry.try_get<CollisionC>(e);
                                if (collision != nullptr) [[likely]]
                                    HandleCollisionEntity(e, pos, *collision, hashGrid, collisionVec);
                                break;
                            }
                        }
                    }
                }
            }
        }

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

        for (const auto e : updateVec)
        {
            if (registry.all_of<ScriptC>(e))
            {
                InvokeEvent<onTick>(e); // Invoke tick event on all entities that are in this tick
            }
        }
    }

} // namespace magique

#endif //LOGICSYSTEM_H