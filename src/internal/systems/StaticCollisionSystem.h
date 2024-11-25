// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STATIC_COLLISION_SYSTEM_H
#define MAGIQUE_STATIC_COLLISION_SYSTEM_H

//-----------------------------------------------
// Static Collision System
//-----------------------------------------------
// .....................................................................
// World bounds is given as white list area -> check against the outer rectangles
// Collidable tiles are treated as squares and inserted into the grid
// .....................................................................

namespace magique
{
    void CheckStaticCollisionRange(int thread, int start, int end);
    void HandleCollisionPairs(StaticPairCollector& pairColl, HashSet<uint64_t>& pairSet);

    inline void StaticCollisionSystem()
    {
        const auto& data = global::ENGINE_DATA;
        auto& staticData = global::STATIC_COLL_DATA;
        const int size = data.collisionVec.size(); // Multithread over certain amount
        if (size < 100)
        {
            CheckStaticCollisionRange(0, 0, size);
        }
        else
        {
            std::array<jobHandle, WORK_PARTS> handles{};
            int end = 0;
            const int partSize = static_cast<int>(static_cast<float>(size) * 0.81F) / WORK_PARTS;
            for (int j = 0; j < WORK_PARTS - 1; ++j) // Gives more work to main thread cause its faster
            {
                const int start = end;
                end = start + partSize;
                handles[j] = AddJob(CreateExplicitJob(CheckStaticCollisionRange, j, start, end));
            }
            CheckStaticCollisionRange(WORK_PARTS - 1, end, size);
            AwaitJobs(handles); // Await completion - for caller its sequential -> easy reasoning and simplicity
        }
        // Handle unique pairs
        HandleCollisionPairs(staticData.pairCollector, staticData.pairSet);
    }

    template <class TypeHashGrid>
    void QueryHashGrid(vector<StaticID>& vec, const TypeHashGrid& grid, const PositionC& pos, const CollisionC& col)
    {
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return grid.query(vec, pos.x, pos.y, col.p1, col.p2);
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                return grid.query(vec, bb.x, bb.y, bb.width, bb.height);
            }
        case Shape::CIRCLE:
            return grid.query(vec, pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F); // Top left and diameter as w and h
        case Shape::CAPSULE:
            // Top left and height as height / diameter as w
            return grid.query(vec, pos.x, pos.y, col.p1 * 2.0F, col.p2);
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    const auto bb =
                        GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3, pos.y + col.p4);
                    return grid.query(vec, bb.x, bb.y, bb.width, bb.height);
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                return grid.query(vec, bb.x, bb.y, bb.width, bb.height);
            }
        }
    }

    inline void CheckAgainstRect(const PositionC& pos, const CollisionC& col, const Rectangle& r, CollisionInfo& info)
    {
        switch (col.shape)
        {
        case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return RectToRect(pos.x, pos.y, col.p1, col.p2, r.x, r.y, r.width, r.height, info);
                }
                // Entity
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);

                // World rect
                const float p1xs[4] = {r.x, r.x + r.width, r.x + r.width, r.x};
                const float p1ys[4] = {r.y, r.y, r.y + r.height, r.y + r.height};
                return SAT(pxs, pys, p1xs, p1ys, info);
            }
        case Shape::CIRCLE:
            {
                return RectToCircle(r.x, r.y, r.width, r.height, pos.x + col.p1, pos.y + col.p1, col.p1, info);
            }
        case Shape::CAPSULE:
            {
                return RectToCapsule(r.x, r.y, r.width, r.height, pos.x, pos.y, col.p1, col.p2, info);
            }
        case Shape::TRIANGLE:
            const float rectX[4] = {r.x, r.x + r.width, r.x + r.width, r.x}; // World rect
            const float rectY[4] = {r.y, r.y, r.y + r.height, r.y + r.height};
            if (pos.rotation == 0) [[likely]]
            {
                const float triX[4] = {pos.x, pos.x + col.p1, pos.x + col.p3, pos.x};
                const float triY[4] = {pos.y, pos.y + col.p2, pos.y + col.p4, pos.y};
                return SAT(rectX, rectY, triX, triY, info);
            }
            float triX[4] = {0, col.p1, col.p3, 0};
            float triY[4] = {0, col.p2, col.p4, 0};
            RotatePoints4(pos.x, pos.y, triX, triY, pos.rotation, col.anchorX, col.anchorY);
            return SAT(rectX, rectY, triX, triY, info);
        }
    }

    inline void CheckAgainstWorldBounds(vector<StaticPair>& collector, const entt::entity e, const PositionC& pos,
                                        CollisionC& col, const Rectangle& r, const uint32_t num)
    {
        CollisionInfo info{};
        CheckAgainstRect(pos, col, r, info);
        if (info.isColliding())
        {
            // subtract 0-3 depending on the world bound
            // We just need to have a unqiue object num so if a collision is found in multiple cells
            // due to duplicate insertion its not processed
            // UINT32_MAX will never be reached and subtracting will be never be negative
            collector.push_back({info, &col, e, UINT32_MAX - num, 0, ColliderType::WORLD_BOUNDS, pos.type});
        }
    }

    template <class TypeHashGrid>
    void CheckHashGrid(const entt::entity e, const TypeHashGrid& grid, vector<StaticID>& collector,
                       vector<StaticPair>& pairCollector, const ColliderType type,
                       const vector<StaticCollider>& colliders, const PositionC& pos, CollisionC& col)
    {
        QueryHashGrid(collector, grid, pos, col); // Tilemap objects
        for (const auto num : collector)
        {
            const auto objectNum = StaticIDHelper::GetObjectNum(num);
            const auto [x, y, p1, p2] = colliders[(int)objectNum]; // O(1) direct lookup
            CollisionInfo info{};
            CheckAgainstRect(pos, col, {x, y, p1, p2}, info);
            if (info.isColliding())
            {
                pairCollector.push_back({info, &col, e, objectNum, StaticIDHelper::GetData(num), type, pos.type});
            }
        }
        collector.clear();
    }

    inline void CheckStaticCollisionRange(const int thread, const int start, const int end) // Runs on each thread
    {
        const auto& data = global::ENGINE_DATA;
        const auto& group = internal::POSITION_GROUP;
        auto& staticData = global::STATIC_COLL_DATA; // non const - modifying the collectors

        const auto& collisionVec = data.collisionVec;
        const auto& colliderStorage = staticData.colliderStorage.colliders;

        // Collectors
        auto& idCollector = staticData.colliderCollector[thread].vec; // non const
        auto& pairCollector = staticData.pairCollector[thread].vec;   // non const

        // Cache
        constexpr float depth = 250.0F;
        const auto wBounds = staticData.worldBounds;
        const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
        const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
        const auto checkWorld = staticData.getIsWorldBoundSet();

        const auto startIt = collisionVec.begin() + start;
        const auto endIt = collisionVec.begin() + end;
        for (auto it = startIt; it != endIt; ++it)
        {
            const auto e = *it;
            const auto& pos = group.get<const PositionC>(e);
            auto& col = group.get<CollisionC>(e); // Non cost for saving modifiable pointer

            if (checkWorld) // Check if worldbounds active
            {
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r1, 0);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r2, 1);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r3, 2);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r4, 3);
            }
            const auto map = pos.map;

            // Query object grid if it has any entries
            if (staticData.mapObjectGrids.contains(map))
            {
                const auto& objectGrid = staticData.mapObjectGrids[map];
                constexpr auto objectType = ColliderType::TILEMAP_OBJECT;
                CheckHashGrid(e, objectGrid, idCollector, pairCollector, objectType, colliderStorage, pos, col);
            }

            // Query tile grid
            if (staticData.mapTileGrids.contains(map))
            {
                const auto& tileGrid = staticData.mapTileGrids[map];
                constexpr auto tileType = ColliderType::TILESET_TILE;
                CheckHashGrid(e, tileGrid, idCollector, pairCollector, tileType, colliderStorage, pos, col);
            }

            // Query group grid
            if (staticData.mapGroupGrids.contains(map))
            {
                const auto& groupGrid = staticData.mapGroupGrids[map];
                constexpr auto groupType = ColliderType::MANUAL_COLLIDER;
                CheckHashGrid(e, groupGrid, idCollector, pairCollector, groupType, colliderStorage, pos, col);
            }
        }
    }

    inline void AccumulateInfo(CollisionC& col, const CollisionInfo& info)
    {
        if (col.lastNormal.x == info.normalVector.x && col.lastNormal.y == info.normalVector.y) [[unlikely]]
            return;
        col.lastNormal = info.normalVector;
        col.resolutionVec.x += info.normalVector.x * info.penDepth;
        col.resolutionVec.y += info.normalVector.y * info.penDepth;
    }

    inline void HandleCollisionPairs(StaticPairCollector& pairColl, HashSet<uint64_t>& pairSet)
    {
        const auto& scripts = global::SCRIPT_DATA.scripts;
        for (auto& [vec] : pairColl)
        {
            for (auto& data : vec)
            {
                const auto uniqueNum = static_cast<uint64_t>(data.entity) << 32 | data.objectNum;
                const auto it = pairSet.find(uniqueNum);
                if (it != pairSet.end()) // This cannot be avoided as duplicates are inserted into the hashgrid
                    continue;
                pairSet.insert(it, uniqueNum);

                // Process the collision
                const auto colliderInfo = ColliderInfo{data.data, data.type};
                InvokeEventDirect<onStaticCollision>(scripts[data.entityType], data.entity, colliderInfo, data.info);
                if (data.info.getIsAccumulated()) // Accumulate the data if specified
                {
                    AccumulateInfo(*data.col, data.info);
                }
            }
            vec.clear();
        }
        pairSet.clear();
    }
} // namespace magique


#endif //MAGIQUE_STATIC_COLLISION_SYSTEM_H