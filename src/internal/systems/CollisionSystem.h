#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

//-----------------------------------------------
// Collision System
//-----------------------------------------------
// .....................................................................
// Optimizations:
// 1. Single Threaded pass through all entities inserting them into the hashgrid and collision vector
//    -> uses custom cache friendly hashgrid (with third-party hashmap)
//    -> first checks if inside camera bounds otherwise if close to any actor
// 2. Multithreaded broad phase (scalable to any amount)
//    -> vector collects all entities in overlapping cells
//    -> Collision is checked with SIMD enabled primitive functions
//    -> if colliding collision pair is stored
//    -> uses separate pair and cell collectors to prevent false sharing
// 3. Single threaded pass over all pairs invoking event methods
//    -> Uses custom Hashset with uint64_t key to mark checked pairs
// .....................................................................

// Give each thread a piece and the then main thread aswell - so it doesnt wait while doing nothing
static constexpr int WORK_PARTS = MAGIQUE_WORKER_THREADS + 1;

namespace magique
{
    bool CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&);
    void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet);
    void QueryHashGrid(vector<entt::entity>& collector, const HashGrid&, const PositionC& pos, const CollisionC& col);

    // System
    inline void CollisionSystem(const entt::registry& registry)
    {
        auto& tickData = global::ENGINE_DATA;
        auto& grid = tickData.hashGrid;
        auto& collisionVec = tickData.collisionVec;
        auto& collectors = tickData.collectors;
        auto& colPairs = tickData.collisionPairs;
        auto& pairSet = tickData.pairSet;

        const auto collisionCheck = [&](const int j, const int startIdx, const int endIdx)
        {
            const auto start = collisionVec.begin() + startIdx;
            const auto end = collisionVec.begin() + endIdx;
            auto& collector = collectors[j].vec;
            auto& pairs = colPairs[j].vec;
            for (auto it = start; it != end; ++it)
            {
                const auto first = *it;
                const auto [posA, colA] = registry.get<const PositionC, const CollisionC>(first);
                QueryHashGrid(collector, grid, posA, colA);
                for (const auto second : collector)
                {
                    if (first >= second)
                        continue;
                    const auto [posB, colB] = registry.get<const PositionC, const CollisionC>(second);
                    if (posA.map != posB.map || (colA.layerMask & colB.layerMask) == 0) [[unlikely]]
                        continue; // Not on the same map or not on the same collision layer
                    if (CheckCollision(posA, colA, posB, colB)) [[unlikely]]
                    {
                        pairs.push_back({first, posA.type, second, posB.type});
                    }
                }
                collector.clear();
            }
        };

        const int size = static_cast<int>(collisionVec.size()); // Multithread over certain amount
        if (size > 500)
        {
            std::array<jobHandle, WORK_PARTS> handles{};
            int end = 0;
            const int partSize = size / WORK_PARTS;
            for (int j = 0; j < WORK_PARTS; ++j)
            {
                const int start = end;
                end = start + partSize;
                if (j == WORK_PARTS - 1)
                {
                    collisionCheck(j, start, end);
                    break;
                }
                if (start - end == 0)
                    continue;
                handles[j] = AddJob(CreateExplicitJob(collisionCheck, j, start, end));
            }
            AwaitJobs(handles);
        }
        else
        {
            collisionCheck(0, 0, size);
        }
#ifdef MAGIQUE_DEBUG_COLLISIONS
        int correct = 0;
        for (const auto first : collisionVec)
        {
            for (const auto second : collisionVec)
            {
                if (first >= second) [[unlikely]]
                    continue;
                auto [posA, colA] = registry.get<PositionC, const CollisionC>(first);
                auto [posB, colB] = registry.get<PositionC, const CollisionC>(second);
                if (CheckCollision(posA, colA, posB, colB)) [[unlikely]]
                {
                    correct++;
                }
            }
        }
        printf("Collisions: %d\n", correct);
#endif
        grid.clear();
        HandleCollisionPairs(colPairs, pairSet);
    }

    inline void QueryHashGrid(vector<entt::entity>& collector, const HashGrid& grid, const PositionC& pos,
                              const CollisionC& col)
    {
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return grid.query(collector, pos.x, pos.y, col.p1, col.p2);
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
            }
        case Shape::CIRCLE:
            return grid.query(collector, pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F); // Top left and diameter as w and h
        case Shape::CAPSULE:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    // Top left and height as height / diameter as w
                    return grid.query(collector, pos.x, pos.y, col.p1 * 2, col.p2);
                }
                float pxs[4] = {0, col.p1 * 2.0F, col.p1 * 2.0F, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
            }
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    const auto bb = GetBBTriangle(pos.x, pos.y, col.p1, col.p2, col.p3, col.p4);
                    return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
            }
        }
    }

    inline void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet)
    {
        const auto GetEntityHash = [](const entt::entity e1, const entt::entity e2)
        { return static_cast<uint64_t>(e1) << 32 | static_cast<uint32_t>(e2); };

        for (auto& [vec] : colPairs)
        {
            for (const auto [e1, id1, e2, id2] : vec)
            {
                auto num = GetEntityHash(e1, e2);
                if (pairSet.contains(num))
                    continue;
                pairSet.insert(num);
                InvokeEventDirect<onDynamicCollision>(global::SCRIPT_DATA.scripts[id1], e1, e2);
                // Invoke out of seconds view
                InvokeEventDirect<onDynamicCollision>(global::SCRIPT_DATA.scripts[id2], e2, e1);
            }
            vec.clear();
        }
        pairSet.clear();
    }

    inline bool CheckCollision(const PositionC& posA, const CollisionC& colA, const PositionC& posB,
                               const CollisionC& colB)
    {
        switch (colA.shape)
        {
        case Shape::RECT:
            switch (colB.shape)
            {
            case Shape::RECT:
                {
                    if (posA.rotation == 0) [[likely]]
                    {
                        if (posB.rotation == 0) [[likely]]
                        {
                            return RectToRect(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2);
                        }
                        float pxs[4] = {0, colB.p1, colB.p1, 0};                                            // rect b
                        float pys[4] = {0, 0, colB.p2, colB.p2};                                            // rect b
                        RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY); // rot
                        const float p1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p1, posA.x};         // Rect a
                        const float p1ys[4] = {posA.y, posA.y, posA.y + colA.p2, posA.y + colA.p2};         // Rect a
                        return SAT(pxs, pys, p1xs, p1ys);
                    }
                    if (posB.rotation == 0) [[likely]] // Only a is rotated
                    {
                        float pxs[4] = {0, colA.p1, colA.p1, 0};                                            // rect a
                        float pys[4] = {0, 0, colA.p2, colA.p2};                                            // rect a
                        RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY); // rot
                        const float p1xs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p1, posB.x};         // Rect b
                        const float p1ys[4] = {posB.y, posB.y, posB.y + colB.p2, posB.y + colB.p2};         // Rect b
                        return SAT(pxs, pys, p1xs, p1ys);
                    }                                         // Both are rotated
                    float pxs[4] = {0, colA.p1, colA.p1, 0};  // rect a
                    float pys[4] = {0, 0, colA.p2, colA.p2};  // rect a
                    float p1xs[4] = {0, colB.p1, colB.p1, 0}; // Rect b
                    float p1ys[4] = {0, 0, colB.p2, colB.p2}; // Rect b
                    RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY);
                    RotatePoints4(posB.x, posB.y, p1xs, p1ys, posB.rotation, colB.anchorX, colB.anchorY);
                    return SAT(pxs, pys, p1xs, p1ys);
                }
            case Shape::CIRCLE:
                {
                    if (posA.rotation == 0)
                    {
                        return RectToCircle(posA.x, posA.y, colA.p1, colA.p2, posB.x + colB.p1 / 2.0F,
                                            posB.y + colB.p1 / 2.0F, colB.p1);
                    }
                    float pxs[4] = {0, colA.p1, colA.p1, 0}; // rect a
                    float pys[4] = {0, 0, colA.p2, colA.p2}; // rect a
                    RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY);
                    return CircleToQuadrilateral(posB.x + colB.p1 / 2.0F, posB.y + colB.p1 / 2.0F, colB.p1, pxs, pys);
                }
            case Shape::CAPSULE:
                if (posA.rotation == 0) [[likely]]
                {
                    return RectToCapsule(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2);
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                if (posA.rotation == 0) [[likely]]
                {
                    if (posB.rotation == 0) [[likely]]
                    {
                        return RectToTriangle(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, posB.x + colB.p1,
                                              posB.y + colB.p2, posB.x + colB.p3, posB.y + colB.p4);
                    }
                }
                LOG_FATAL("Method not implemented");
                break;
            }
        case Shape::CIRCLE:
            switch (colB.shape)
            {
            case Shape::RECT:
                {
                    if (posB.rotation == 0)
                    {
                        return RectToCircle(posB.x, posB.y, colB.p1, colB.p2, posA.x + colA.p1 / 2.0F,
                                            posA.y + colA.p1 / 2.0F, colA.p1);
                    }
                    float pxs[4] = {0, colB.p1, colB.p1, 0}; // rect b
                    float pys[4] = {0, 0, colB.p2, colB.p2}; // rect b
                    RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY);
                    return CircleToQuadrilateral(posA.x + colA.p1 / 2.0F, posA.y + colA.p1 / 2.0F, colA.p1, pxs, pys);
                }
            case Shape::CIRCLE:
                // We can skip the translation to the middle point as both are in the same system
                return CircleToCircle(posA.x, posA.y, colA.p1, posB.x, posB.y, colB.p1);
            case Shape::CAPSULE:
                if (posB.rotation == 0) [[likely]]
                {
                    return CircleToCapsule(posA.x + colA.p1 / 2.0F, posA.y + colA.p1 / 2.0F, colA.p1, posB.x, posB.y,
                                           colB.p1, colB.p2);
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                {
                    if (posB.rotation == 0)
                    {
                        const float txs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                        const float tys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                        return CircleToQuadrilateral(posA.x + colA.p1 / 2.0F, posA.y + colA.p1 / 2.0F, colA.p1, txs,
                                                     tys);
                    }
                    float txs[4] = {0, colB.p1, colB.p3, 0};
                    float tys[4] = {0, colB.p2, colB.p4, 0};
                    RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                    return CircleToQuadrilateral(posA.x + colA.p1 / 2.0F, posA.y + colA.p1 / 2.0F, colA.p1, txs, tys);
                }
            }
        case Shape::CAPSULE:
            switch (colB.shape)
            {
            case Shape::RECT:
                if (posA.rotation == 0) [[likely]]
                {
                    if (posB.rotation == 0) [[likely]]
                    {
                        return RectToCapsule(posB.x, posB.y, colB.p1, colB.p2, posA.x, posA.y, colA.p1, colA.p2);
                    }
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CIRCLE:
                if (posA.rotation == 0) [[likely]]
                {
                    return CircleToCapsule(posB.x + colB.p1 / 2.0F, posB.y + colB.p1 / 2.0F, colB.p1, posA.x, posA.y,
                                           colA.p1, colA.p2);
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CAPSULE:
                if (posA.rotation == 0) [[likely]]
                {
                    if (posB.rotation == 0) [[likely]]
                    {
                        return CapsuleToCapsule(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2);
                    }
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                LOG_FATAL("Method not implemented");
                break;
            }
        case Shape::TRIANGLE:
            switch (colB.shape)
            {
            case Shape::RECT:
                if (posA.rotation == 0) [[likely]]
                {
                    if (posB.rotation == 0) [[likely]]
                    {
                        return RectToTriangle(posB.x, posB.y, colB.p1, colB.p2, posA.x, posA.y, posA.x + colA.p1,
                                              posA.y + colA.p2, posA.x + colA.p3, posA.y + colA.p4);
                    }
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CIRCLE:
                {
                    if (posA.rotation == 0)
                    {
                        const float txs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                        const float tys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                        return CircleToQuadrilateral(posB.x + colB.p1 / 2.0F, posB.y + colB.p1 / 2.0F, colB.p1, txs,
                                                     tys);
                    }
                    float txs[4] = {0, colA.p1, colA.p3, 0};
                    float tys[4] = {0, colA.p2, colA.p4, 0};
                    RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                    return CircleToQuadrilateral(posB.x + colB.p1 / 2.0F, posB.y + colB.p1 / 2.0F, colB.p1, txs, tys);
                }
            case Shape::CAPSULE:
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                if (posA.rotation == 0) // For triangles we dont assume as they are likely rotated
                {
                    if (posB.rotation == 0)
                    {
                        return TriangleToTriangle(posA.x, posA.y, posA.x + colA.p1, posA.y + colA.p2, posA.x + colA.p3,
                                                  posA.y + colA.p4, // Triangle 2
                                                  posB.x, posB.y, posB.x + colB.p1, posB.y + colB.p2, posB.x + colB.p3,
                                                  posB.y + colB.p4);
                    }
                    float txs[4] = {0, colB.p1, colB.p3, 0};
                    float tys[4] = {0, colB.p2, colB.p4, 0};
                    RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                    const float t1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                    const float t1ys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                    return SAT(txs, tys, t1xs, t1ys);
                }
                if (posB.rotation == 0)
                {
                    float txs[4] = {0, colA.p1, colA.p3, 0};
                    float tys[4] = {0, colA.p2, colA.p4, 0};
                    RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                    const float t1xs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                    const float t1ys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                    return SAT(txs, tys, t1xs, t1ys);
                }
                float txs[4] = {0, colA.p1, colA.p3, 0};
                float tys[4] = {0, colA.p2, colA.p4, 0};
                float t1xs[4] = {0, colB.p1, colB.p3, 0};
                float t1ys[4] = {0, colB.p2, colB.p4, 0};
                RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                RotatePoints4(posB.x, posB.y, t1xs, t1ys, posB.rotation, colB.anchorX, colB.anchorY);
                return SAT(txs, tys, t1xs, t1ys);
            }
        }
        LOG_FATAL("Method not implemented");
        return false;
    }
} // namespace magique

#endif //COLLISIONSYSTEM_H