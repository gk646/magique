#ifndef DYNAMIC_COLLISION_SYSTEM_H
#define DYNAMIC_COLLISION_SYSTEM_H

//-----------------------------------------------
// Dynamic Collision System
//-----------------------------------------------
// .....................................................................
// Optimizations:
// 1. Single Threaded pass through all entities inserting them into the hashgrid and collision vector
//    -> uses custom cache friendly hashgrid (with third-party hashmap)
//    -> first checks if inside camera bounds otherwise if close to any actor
// 2. Multithreaded broad phase (scalable to any amount)
//    -> iterate all hash grid cells
//    -> Collision is checked with SIMD enabled primitive functions
//    -> if colliding collision pair is stored
//    -> uses separate pair collectors to prevent false sharing
// 3. Single threaded pass over all pairs invoking event methods
//    -> Uses custom Hashset with uint64_t key to mark checked pairs
//
// Note: Can still be optimized -> Using a AABB tree (or bounding volume hierarchy) avoids the hashset and will probably
//       have faster lookups times BUT MUCH more complex to manage and move objects efficiently
// -> But doesn't change anything for user
// .....................................................................

namespace magique
{
    void CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&, CollisionInfo& i);
    void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet);
    void CheckHashGridCells(const EntityHashGrid& grid, vector<PairInfo>& pairs, int startIdx, int endIdx);

    //----------------- SYSTEM -----------------//

    inline void DynamicCollisionSystem()
    {
        auto& dyCollData = global::DY_COLL_DATA;
        auto& grid = dyCollData.hashGrid;
        auto& colPairs = dyCollData.collisionPairs;
        auto& pairSet = dyCollData.pairSet;

        const int size = static_cast<int>(grid.cellMap.size()); // Multithread over certain amount
        if (size > 150)
        {
            std::array<jobHandle, WORK_PARTS> handles{};
            int end = 0;
            const int partSize = static_cast<int>((float)size * 0.81F) / WORK_PARTS; // Give main thread more work
            for (int j = 0; j < WORK_PARTS - 1; ++j)
            {
                const int start = end;
                end = start + partSize;
                handles[j] =
                    AddJob(CreateExplicitJob(CheckHashGridCells, std::ref(grid), std::ref(colPairs[j].vec), start, end));
            }
            CheckHashGridCells(grid, colPairs[WORK_PARTS - 1].vec, end, size);
            AwaitJobs(handles);
        }
        else
        {
            CheckHashGridCells(grid, colPairs[0].vec, 0, size);
        }

        HandleCollisionPairs(colPairs, pairSet);
    }

    //----------------- IMPLEMENTATION -----------------//

    inline void CheckHashGridCells(const EntityHashGrid& grid, vector<PairInfo>& pairs, const int startIdx,
                                   const int endIdx)
    {
        const auto start = grid.dataBlocks.begin() + startIdx;
        const auto end = grid.dataBlocks.begin() + endIdx;
        const auto& group = internal::POSITION_GROUP;
        for (auto it = start; it != end; ++it)
        {
            const auto& block = *it;
            const auto dStart = block.data;
            const auto dEnd = block.data + block.count;
            for (auto dIt1 = dStart; dIt1 != dEnd; ++dIt1)
            {
                const auto first = *dIt1;
                auto [posA, colA] = group.get<const PositionC, CollisionC>(first);
                for (auto dIt2 = dStart; dIt2 != dEnd; ++dIt2)
                {
                    const auto second = *dIt2;
                    if (first >= second)
                        continue;
                    auto [posB, colB] = group.get<const PositionC, CollisionC>(second);
                    if (posA.map != posB.map || (colA.layerMask & colB.layerMask) == 0)
                        continue; // Not on the same map or not on the same collision layer
                    CollisionInfo info{};
                    CheckCollision(posA, colA, posB, colB, info);
                    if (info.isColliding())
                    {
                        pairs.push_back(PairInfo{info, colA, colB, posA, posB, first, second});
                    }
                }
            }
        }
    }

    inline void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet)
    {
        const auto& scriptVec = global::SCRIPT_DATA.scripts;
        const auto& colVec = global::ENGINE_DATA.collisionVec;
        const auto group = internal::POSITION_GROUP;

        for (auto& [vec] : colPairs)
        {
            for (auto& [info, col1, col2, p1, p2, e1, e2] : vec)
            {
                auto num = static_cast<uint64_t>(e1) << 32 | static_cast<uint32_t>(e2);
                const auto it = pairSet.find(num);
                if (it != pairSet.end()) // This cannot be avoided as duplicates are inserted into the hashgrid
                    continue;
                pairSet.insert(it, num);

                auto secondInfo = info; // Prepare second info
                secondInfo.normalVector.x *= -1;
                secondInfo.normalVector.y *= -1;

                // Call for first entity
                InvokeEventDirect<onDynamicCollision>(scriptVec[p1.type], e1, e2, info);

                if (info.getIsAccumulated())
                    AccumulateInfo(col1, info);

                // Call for second entity
                InvokeEventDirect<onDynamicCollision>(scriptVec[p2.type], e2, e1, secondInfo);

                if (secondInfo.getIsAccumulated())
                    AccumulateInfo(col2, secondInfo);
            }
            vec.clear();
        }

        for (const auto e : colVec)
        {
            auto [pos, col] = group.get<PositionC, CollisionC>(e);
            pos.x += 0;
            pos.y += 0;
            col.clearCollisionData();
        }
        pairSet.clear();
    }

    // Should be the most efficient way - allows jump tables and inlining - this is actually very fast!
    // With 15k entities skipping all switches and returning immediately only saves around 0.1 ms
    inline void CheckCollision(const PositionC& pA, const CollisionC& cA, const PositionC& pB, const CollisionC& cB,
                               CollisionInfo& i)
    {
        switch (cA.shape)
        {
        case Shape::RECT:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pA.rotation == 0) [[likely]]
                    {
                        if (pB.rotation == 0) [[likely]]
                        {
                            return RectToRect(pA.x, pA.y, cA.p1, cA.p2, pB.x, pB.y, cB.p1, cB.p2, i);
                        }
                        float pxs[4] = {0, cB.p1, cB.p1, 0};                                      // rect b
                        float pys[4] = {0, 0, cB.p2, cB.p2};                                      // rect b
                        RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY); // rot
                        const float p1xs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p1, pA.x};           // Rect a
                        const float p1ys[4] = {pA.y, pA.y, pA.y + cA.p2, pA.y + cA.p2};           // Rect a
                        return SAT(pxs, pys, p1xs, p1ys, i);
                    }
                    if (pB.rotation == 0) [[likely]] // Only a is rotated
                    {
                        float pxs[4] = {0, cA.p1, cA.p1, 0};                                      // rect a
                        float pys[4] = {0, 0, cA.p2, cA.p2};                                      // rect a
                        RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY); // rot
                        const float p1xs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p1, pB.x};           // Rect b
                        const float p1ys[4] = {pB.y, pB.y, pB.y + cB.p2, pB.y + cB.p2};           // Rect b
                        return SAT(pxs, pys, p1xs, p1ys, i);
                    }                                     // Both are rotated
                    float pxs[4] = {0, cA.p1, cA.p1, 0};  // rect a
                    float pys[4] = {0, 0, cA.p2, cA.p2};  // rect a
                    float p1xs[4] = {0, cB.p1, cB.p1, 0}; // Rect b
                    float p1ys[4] = {0, 0, cB.p2, cB.p2}; // Rect b
                    RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY);
                    RotatePoints4(pB.x, pB.y, p1xs, p1ys, pB.rotation, cB.anchorX, cB.anchorY);
                    return SAT(pxs, pys, p1xs, p1ys, i);
                }
            case Shape::CIRCLE:
                {
                    if (pA.rotation == 0)
                    {
                        return RectToCircle(pA.x, pA.y, cA.p1, cA.p2, pB.x + cB.p1, pB.y + cB.p1, cB.p1, i);
                    }
                    float pxs[4] = {0, cA.p1, cA.p1, 0}; // rect a
                    float pys[4] = {0, 0, cA.p2, cA.p2}; // rect a
                    RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY);
                    return CircleToQuadrilateral(pB.x + cB.p1, pB.y + cB.p1, cB.p1, pxs, pys, i);
                }
            case Shape::CAPSULE:
                {
                    if (pA.rotation == 0) [[likely]]
                    {
                        return RectToCapsule(pA.x, pA.y, cA.p1, cA.p2, pB.x, pB.y, cB.p1, cB.p2, i);
                    }
                    float pxs[4] = {0, cA.p1, cA.p1, 0};                                      // rect a
                    float pys[4] = {0, 0, cA.p2, cA.p2};                                      // rect a
                    RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY); // rot
                    return CapsuleToQuadrilateral(pB.x, pB.y, cB.p1, cB.p2, pxs, pys, i);
                }
            case Shape::TRIANGLE:
                if (pA.rotation == 0) [[likely]]
                {
                    if (pB.rotation == 0)
                    {
                        const float rectX[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p1, pA.x};
                        const float rectY[4] = {pA.y, pA.y, pA.y + cA.p2, pA.y + cA.p2};

                        const float triX[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                        const float triY[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                        return SAT(rectX, rectY, triX, triY, i);
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.x, pB.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    const float p1xs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p1, pA.x}; // Rect a
                    const float p1ys[4] = {pA.y, pA.y, pA.y + cA.p2, pA.y + cA.p2}; // Rect a
                    return SAT(txs, tys, p1xs, p1ys, i);
                }
                if (pB.rotation == 0)
                {
                    float pxs[4] = {0, cA.p1, cA.p1, 0};
                    float pys[4] = {0, 0, cA.p2, cA.p2};
                    RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY);
                    const float txs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                    const float tys[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                    return SAT(pxs, pys, txs, tys, i);
                }
                float pxs[4] = {0, cA.p1, cA.p1, 0};
                float pys[4] = {0, 0, cA.p2, cA.p2};
                RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY);
                float txs[4] = {0, cB.p1, cB.p3, 0};
                float tys[4] = {0, cB.p2, cB.p4, 0};
                RotatePoints4(pB.x, pB.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                return SAT(pxs, pys, txs, tys, i);
            }
        case Shape::CIRCLE:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pB.rotation == 0)
                    {
                        return RectToCircle(pB.x, pB.y, cB.p1, cB.p2, pA.x + cA.p1, pA.y + cA.p1, cA.p1, i);
                    }
                    float pxs[4] = {0, cB.p1, cB.p1, 0}; // rect b
                    float pys[4] = {0, 0, cB.p2, cB.p2}; // rect b
                    RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY);
                    return CircleToQuadrilateral(pA.x + cA.p1, pA.y + cA.p1, cA.p1, pxs, pys, i);
                }
            case Shape::CIRCLE:
                // We can skip the translation to the middle point as both are in the same system
                return CircleToCircle(pA.x + cA.p1, pA.y + cA.p1, cA.p1, pB.x + cB.p1, pB.y + cB.p1, cB.p1, i);
            case Shape::CAPSULE:
                return CircleToCapsule(pA.x + cA.p1, pA.y + cA.p1, cA.p1, pB.x, pB.y, cB.p1, cB.p2, i);
            case Shape::TRIANGLE:
                {
                    if (pB.rotation == 0)
                    {
                        const float txs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                        const float tys[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                        return CircleToQuadrilateral(pA.x + cA.p1, pA.y + cA.p1, cA.p1, txs, tys, i);
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.x, pB.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    return CircleToQuadrilateral(pA.x + cA.p1, pA.y + cA.p1, cA.p1, txs, tys, i);
                }
            }
        case Shape::CAPSULE:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pB.rotation == 0) [[likely]]
                    {
                        return RectToCapsule(pB.x, pB.y, cB.p1, cB.p2, pA.x, pA.y, cA.p1, cA.p2, i);
                    }
                    float pxs[4] = {0, cB.p1, cB.p1, 0};                                      // rect b
                    float pys[4] = {0, 0, cB.p2, cB.p2};                                      // rect b
                    RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY); // rot
                    return CapsuleToQuadrilateral(pA.x, pA.y, cA.p1, cA.p2, pxs, pys, i);
                }
            case Shape::CIRCLE:
                return CircleToCapsule(pB.x + cB.p1, pB.y + cB.p1, cB.p1, pA.x, pA.y, cA.p1, cA.p2, i);
            case Shape::CAPSULE:
                return CapsuleToCapsule(pA.x, pA.y, cA.p1, cA.p2, pB.x, pB.y, cB.p1, cB.p2, i);
            case Shape::TRIANGLE:
                if (pB.rotation == 0)
                {
                    const float txs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                    const float tys[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                    return CapsuleToQuadrilateral(pA.x, pA.y, cA.p1, cA.p2, txs, tys, i);
                }
                float txs[4] = {0, cB.p1, cB.p3, 0};
                float tys[4] = {0, cB.p2, cB.p4, 0};
                RotatePoints4(pB.x, pB.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                return CapsuleToQuadrilateral(pA.x, pA.y, cA.p1, cA.p2, txs, tys, i);
            }
        case Shape::TRIANGLE:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pA.rotation == 0) [[likely]]
                    {
                        if (pB.rotation == 0) [[likely]]
                        {
                            const float rectX[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p1, pB.x};
                            const float rectY[4] = {pB.y, pB.y, pB.y + cB.p2, pB.y + cB.p2};

                            const float triX[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p3, pA.x};
                            const float triY[4] = {pA.y, pA.y + cA.p2, pA.y + cA.p4, pA.y};
                            return SAT(rectX, rectY, triX, triY, i);
                        }
                        float pxs[4] = {0, cB.p1, cB.p1, 0};                                      // rect b
                        float pys[4] = {0, 0, cB.p2, cB.p2};                                      // rect b
                        RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY); // rot
                        const float t1xs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p3, pA.x};
                        const float t1ys[4] = {pA.y, pA.y + cA.p2, pA.y + cA.p4, pA.y};
                        return SAT(pxs, pys, t1xs, t1ys, i);
                    }
                    if (pB.rotation == 0)
                    {
                        float txs[4] = {0, cA.p1, cA.p3, 0};
                        float tys[4] = {0, cA.p2, cA.p4, 0};
                        RotatePoints4(pA.x, pA.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                        const float p1xs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p1, pB.x}; // Rect b
                        const float p1ys[4] = {pB.y, pB.y, pB.y + cB.p2, pB.y + cB.p2}; // Rect b
                        return SAT(txs, tys, p1xs, p1ys, i);
                    }
                    float txs[4] = {0, cA.p1, cA.p3, 0}; // Triangle rotated
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.x, pA.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    float pxs[4] = {0, cB.p1, cB.p1, 0}; // Rect rotated
                    float pys[4] = {0, 0, cB.p2, cB.p2};
                    RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY);
                    return SAT(pxs, pys, txs, tys, i);
                }
            case Shape::CIRCLE:
                {
                    if (pA.rotation == 0)
                    {
                        const float txs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p3, pA.x};
                        const float tys[4] = {pA.y, pA.y + cA.p2, pA.y + cA.p4, pA.y};
                        return CircleToQuadrilateral(pB.x + cB.p1, pB.y + cB.p1, cB.p1, txs, tys, i);
                    }
                    float txs[4] = {0, cA.p1, cA.p3, 0};
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.x, pA.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    return CircleToQuadrilateral(pB.x + cB.p1, pB.y + cB.p1, cB.p1, txs, tys, i);
                }
            case Shape::CAPSULE:
                {
                    if (pA.rotation == 0)
                    {
                        const float txs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p3, pA.x};
                        const float tys[4] = {pA.y, pA.y + cA.p2, pA.y + cA.p4, pA.y};
                        return CapsuleToQuadrilateral(pB.x, pB.y, cB.p1, cB.p2, txs, tys, i);
                    }
                    float txs[4] = {0, cA.p1, cA.p3, 0};
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.x, pA.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    return CapsuleToQuadrilateral(pB.x, pB.y, cB.p1, cB.p2, txs, tys, i);
                }
            case Shape::TRIANGLE:
                if (pA.rotation == 0) // For triangles we dont assume as they are likely rotated
                {
                    if (pB.rotation == 0)
                    {
                        const float t1xs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p3, pA.x};
                        const float t1ys[4] = {pA.y, pA.y + cA.p2, pA.y + cA.p4, pA.y};
                        const float t2xs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                        const float t2ys[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                        return SAT(t1xs, t1ys, t2xs, t2ys, i);
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.x, pB.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    const float t1xs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p3, pA.x};
                    const float t1ys[4] = {pA.y, pA.y + cA.p2, pA.y + cA.p4, pA.y};
                    return SAT(txs, tys, t1xs, t1ys, i);
                }
                if (pB.rotation == 0)
                {
                    float txs[4] = {0, cA.p1, cA.p3, 0};
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.x, pA.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    const float t1xs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                    const float t1ys[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                    return SAT(txs, tys, t1xs, t1ys, i);
                }
                float txs[4] = {0, cA.p1, cA.p3, 0};
                float tys[4] = {0, cA.p2, cA.p4, 0};
                float t1xs[4] = {0, cB.p1, cB.p3, 0};
                float t1ys[4] = {0, cB.p2, cB.p4, 0};
                RotatePoints4(pA.x, pA.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                RotatePoints4(pB.x, pB.y, t1xs, t1ys, pB.rotation, cB.anchorX, cB.anchorY);
                return SAT(txs, tys, t1xs, t1ys, i);
            }
        }
    }
} // namespace magique

#endif //DYNAMIC_COLLISION_SYSTEM_H