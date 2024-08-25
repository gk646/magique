#ifndef DYNAMIC_COLLISIONSYSTEM_H
#define DYNAMIC_COLLISIONSYSTEM_H

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
// .....................................................................

namespace magique
{
    void CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&, CollisionInfo& info);
    void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet);
    void CheckHashGridCells(const EntityHashGrid& grid, vector<PairInfo>& pairs, int startIdx, int endIdx);

    // System
    inline void DynamicCollisionSystem()
    {
        return;
        auto& data = global::ENGINE_DATA;
        auto& grid = data.hashGrid;
        auto& colPairs = data.collisionPairs;

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
        HandleCollisionPairs(colPairs, data.pairSet);
    }

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
                const auto [posA, colA] = group.get<const PositionC, const CollisionC>(first);
                for (auto dIt2 = dStart; dIt2 != dEnd; ++dIt2)
                {
                    const auto second = *dIt2;
                    if (first >= second)
                        continue;
                    const auto [posB, colB] = group.get<const PositionC, const CollisionC>(second);
                    if (posA.map != posB.map || (colA.layerMask & colB.layerMask) == 0)
                        continue; // Not on the same map or not on the same collision layer
                    CollisionInfo info = CollisionInfo::NoCollision();
                    CheckCollision(posA, colA, posB, colB, info);
                    if (info.isColliding())
                    {
                        pairs.push_back({info, first, second, posA.type, posB.type});
                    }
                }
            }
        }
    }

    inline void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet, CollisionInfoMap& infoMap)
    {
        for (auto& [vec] : colPairs)
        {
            for (auto [info, e1, e2, id1, id2] : vec)
            {
                auto num = static_cast<uint64_t>(e1) << 32 | static_cast<uint32_t>(e2);
                const auto it = pairSet.find(num);
                if (it == pairSet.end())
                    continue;
                pairSet.insert(it, num);

                auto& e1Info = infoMap[e1];
                e1Info.info = info;
                e1Info.id = id1;

                info.normalVector.x = -info.normalVector.x;
                info.normalVector.y = -info.normalVector.y;

                auto& e2Info = infoMap[e2];
                e2Info.info = info;
                e2Info.id = id1;
            }
            vec.clear();
        }

        pairSet.clear();
    }

    inline void CheckCollision(const PositionC& posA, const CollisionC& colA, const PositionC& posB,
                               const CollisionC& colB, CollisionInfo& info)
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
                            return RectToRect(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2, info);
                        }
                        float pxs[4] = {0, colB.p1, colB.p1, 0};                                            // rect b
                        float pys[4] = {0, 0, colB.p2, colB.p2};                                            // rect b
                        RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY); // rot
                        const float p1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p1, posA.x};         // Rect a
                        const float p1ys[4] = {posA.y, posA.y, posA.y + colA.p2, posA.y + colA.p2};         // Rect a
                        return SAT(pxs, pys, p1xs, p1ys, info);
                    }
                    if (posB.rotation == 0) [[likely]] // Only a is rotated
                    {
                        float pxs[4] = {0, colA.p1, colA.p1, 0};                                            // rect a
                        float pys[4] = {0, 0, colA.p2, colA.p2};                                            // rect a
                        RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY); // rot
                        const float p1xs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p1, posB.x};         // Rect b
                        const float p1ys[4] = {posB.y, posB.y, posB.y + colB.p2, posB.y + colB.p2};         // Rect b
                        return SAT(pxs, pys, p1xs, p1ys, info);
                    }                                         // Both are rotated
                    float pxs[4] = {0, colA.p1, colA.p1, 0};  // rect a
                    float pys[4] = {0, 0, colA.p2, colA.p2};  // rect a
                    float p1xs[4] = {0, colB.p1, colB.p1, 0}; // Rect b
                    float p1ys[4] = {0, 0, colB.p2, colB.p2}; // Rect b
                    RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY);
                    RotatePoints4(posB.x, posB.y, p1xs, p1ys, posB.rotation, colB.anchorX, colB.anchorY);
                    return SAT(pxs, pys, p1xs, p1ys, info);
                }
            case Shape::CIRCLE:
                {
                    if (posA.rotation == 0)
                    {
                        return RectToCircle(posA.x, posA.y, colA.p1, colA.p2, posB.x + colB.p1, posB.y + colB.p1,
                                            colB.p1, info);
                    }
                    float pxs[4] = {0, colA.p1, colA.p1, 0}; // rect a
                    float pys[4] = {0, 0, colA.p2, colA.p2}; // rect a
                    RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY);
                    return CircleToQuadrilateral(posB.x + colB.p1, posB.y + colB.p1, colB.p1, pxs, pys, info);
                }
            case Shape::CAPSULE:
                {
                    if (posA.rotation == 0) [[likely]]
                    {
                        return RectToCapsule(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2, info);
                    }
                    float pxs[4] = {0, colA.p1, colA.p1, 0};                                            // rect a
                    float pys[4] = {0, 0, colA.p2, colA.p2};                                            // rect a
                    RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY); // rot
                    return CapsuleToQuadrilateral(posB.x, posB.y, colB.p1, colB.p2, pxs, pys, info);
                }
            case Shape::TRIANGLE:
                if (posA.rotation == 0) [[likely]]
                {
                    if (posB.rotation == 0)
                    {
                        const float rectX[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p1, posA.x};
                        const float rectY[4] = {posA.y, posA.y, posA.y + colA.p2, posA.y + colA.p2};

                        const float triX[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                        const float triY[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                        return SAT(rectX, rectY, triX, triY, info);
                    }
                    float txs[4] = {0, colB.p1, colB.p3, 0};
                    float tys[4] = {0, colB.p2, colB.p4, 0};
                    RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                    const float p1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p1, posA.x}; // Rect a
                    const float p1ys[4] = {posA.y, posA.y, posA.y + colA.p2, posA.y + colA.p2}; // Rect a
                    return SAT(txs, tys, p1xs, p1ys, info);
                }
                if (posB.rotation == 0)
                {
                    float pxs[4] = {0, colA.p1, colA.p1, 0};
                    float pys[4] = {0, 0, colA.p2, colA.p2};
                    RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY);
                    const float txs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                    const float tys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                    return SAT(pxs, pys, txs, tys, info);
                }
                float pxs[4] = {0, colA.p1, colA.p1, 0};
                float pys[4] = {0, 0, colA.p2, colA.p2};
                RotatePoints4(posA.x, posA.y, pxs, pys, posA.rotation, colA.anchorX, colA.anchorY);
                float txs[4] = {0, colB.p1, colB.p3, 0};
                float tys[4] = {0, colB.p2, colB.p4, 0};
                RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                return SAT(pxs, pys, txs, tys, info);
            }
        case Shape::CIRCLE:
            switch (colB.shape)
            {
            case Shape::RECT:
                {
                    if (posB.rotation == 0)
                    {
                        return RectToCircle(posB.x, posB.y, colB.p1, colB.p2, posA.x + colA.p1, posA.y + colA.p1,
                                            colA.p1, info);
                    }
                    float pxs[4] = {0, colB.p1, colB.p1, 0}; // rect b
                    float pys[4] = {0, 0, colB.p2, colB.p2}; // rect b
                    RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY);
                    return CircleToQuadrilateral(posA.x + colA.p1, posA.y + colA.p1, colA.p1, pxs, pys, info);
                }
            case Shape::CIRCLE:
                // We can skip the translation to the middle point as both are in the same system
                return CircleToCircle(posA.x + colA.p1, posA.y + colA.p1, colA.p1, posB.x + colB.p1, posB.y + colB.p1,
                                      colB.p1, info);
            case Shape::CAPSULE:
                return CircleToCapsule(posA.x + colA.p1, posA.y + colA.p1, colA.p1, posB.x, posB.y, colB.p1, colB.p2,
                                       info);
            case Shape::TRIANGLE:
                {
                    if (posB.rotation == 0)
                    {
                        const float txs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                        const float tys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                        return CircleToQuadrilateral(posA.x + colA.p1, posA.y + colA.p1, colA.p1, txs, tys, info);
                    }
                    float txs[4] = {0, colB.p1, colB.p3, 0};
                    float tys[4] = {0, colB.p2, colB.p4, 0};
                    RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                    return CircleToQuadrilateral(posA.x + colA.p1, posA.y + colA.p1, colA.p1, txs, tys, info);
                }
            }
        case Shape::CAPSULE:
            switch (colB.shape)
            {
            case Shape::RECT:
                {
                    if (posB.rotation == 0) [[likely]]
                    {
                        return RectToCapsule(posB.x, posB.y, colB.p1, colB.p2, posA.x, posA.y, colA.p1, colA.p2, info);
                    }
                    float pxs[4] = {0, colB.p1, colB.p1, 0};                                            // rect b
                    float pys[4] = {0, 0, colB.p2, colB.p2};                                            // rect b
                    RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY); // rot
                    return CapsuleToQuadrilateral(posA.x, posA.y, colA.p1, colA.p2, pxs, pys, info);
                }
            case Shape::CIRCLE:
                return CircleToCapsule(posB.x + colB.p1, posB.y + colB.p1, colB.p1, posA.x, posA.y, colA.p1, colA.p2,
                                       info);
            case Shape::CAPSULE:
                return CapsuleToCapsule(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2, info);
            case Shape::TRIANGLE:
                if (posB.rotation == 0)
                {
                    const float txs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                    const float tys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                    return CapsuleToQuadrilateral(posA.x, posA.y, colA.p1, colA.p2, txs, tys, info);
                }
                float txs[4] = {0, colB.p1, colB.p3, 0};
                float tys[4] = {0, colB.p2, colB.p4, 0};
                RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                return CapsuleToQuadrilateral(posA.x, posA.y, colA.p1, colA.p2, txs, tys, info);
            }
        case Shape::TRIANGLE:
            switch (colB.shape)
            {
            case Shape::RECT:
                {
                    if (posA.rotation == 0) [[likely]]
                    {
                        if (posB.rotation == 0) [[likely]]
                        {
                            const float rectX[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p1, posB.x};
                            const float rectY[4] = {posB.y, posB.y, posB.y + colB.p2, posB.y + colB.p2};

                            const float triX[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                            const float triY[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                            return SAT(rectX, rectY, triX, triY, info);
                        }
                        float pxs[4] = {0, colB.p1, colB.p1, 0};                                            // rect b
                        float pys[4] = {0, 0, colB.p2, colB.p2};                                            // rect b
                        RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY); // rot
                        const float t1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                        const float t1ys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                        return SAT(pxs, pys, t1xs, t1ys, info);
                    }
                    if (posB.rotation == 0)
                    {
                        float txs[4] = {0, colA.p1, colA.p3, 0};
                        float tys[4] = {0, colA.p2, colA.p4, 0};
                        RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                        const float p1xs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p1, posB.x}; // Rect b
                        const float p1ys[4] = {posB.y, posB.y, posB.y + colB.p2, posB.y + colB.p2}; // Rect b
                        return SAT(txs, tys, p1xs, p1ys, info);
                    }
                    float txs[4] = {0, colA.p1, colA.p3, 0}; // Triangle rotated
                    float tys[4] = {0, colA.p2, colA.p4, 0};
                    RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                    float pxs[4] = {0, colB.p1, colB.p1, 0}; // Rect rotated
                    float pys[4] = {0, 0, colB.p2, colB.p2};
                    RotatePoints4(posB.x, posB.y, pxs, pys, posB.rotation, colB.anchorX, colB.anchorY);
                    return SAT(pxs, pys, txs, tys, info);
                }
            case Shape::CIRCLE:
                {
                    if (posA.rotation == 0)
                    {
                        const float txs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                        const float tys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                        return CircleToQuadrilateral(posB.x + colB.p1, posB.y + colB.p1, colB.p1, txs, tys, info);
                    }
                    float txs[4] = {0, colA.p1, colA.p3, 0};
                    float tys[4] = {0, colA.p2, colA.p4, 0};
                    RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                    return CircleToQuadrilateral(posB.x + colB.p1, posB.y + colB.p1, colB.p1, txs, tys, info);
                }
            case Shape::CAPSULE:
                {
                    if (posA.rotation == 0)
                    {
                        const float txs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                        const float tys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                        return CapsuleToQuadrilateral(posB.x, posB.y, colB.p1, colB.p2, txs, tys, info);
                    }
                    float txs[4] = {0, colA.p1, colA.p3, 0};
                    float tys[4] = {0, colA.p2, colA.p4, 0};
                    RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                    return CapsuleToQuadrilateral(posB.x, posB.y, colB.p1, colB.p2, txs, tys, info);
                }
            case Shape::TRIANGLE:
                if (posA.rotation == 0) // For triangles we dont assume as they are likely rotated
                {
                    if (posB.rotation == 0)
                    {
                        const float t1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                        const float t1ys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                        const float t2xs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                        const float t2ys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                        return SAT(t1xs, t1ys, t2xs, t2ys, info);
                    }
                    float txs[4] = {0, colB.p1, colB.p3, 0};
                    float tys[4] = {0, colB.p2, colB.p4, 0};
                    RotatePoints4(posB.x, posB.y, txs, tys, posB.rotation, colB.anchorX, colB.anchorY);
                    const float t1xs[4] = {posA.x, posA.x + colA.p1, posA.x + colA.p3, posA.x};
                    const float t1ys[4] = {posA.y, posA.y + colA.p2, posA.y + colA.p4, posA.y};
                    return SAT(txs, tys, t1xs, t1ys, info);
                }
                if (posB.rotation == 0)
                {
                    float txs[4] = {0, colA.p1, colA.p3, 0};
                    float tys[4] = {0, colA.p2, colA.p4, 0};
                    RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                    const float t1xs[4] = {posB.x, posB.x + colB.p1, posB.x + colB.p3, posB.x};
                    const float t1ys[4] = {posB.y, posB.y + colB.p2, posB.y + colB.p4, posB.y};
                    return SAT(txs, tys, t1xs, t1ys, info);
                }
                float txs[4] = {0, colA.p1, colA.p3, 0};
                float tys[4] = {0, colA.p2, colA.p4, 0};
                float t1xs[4] = {0, colB.p1, colB.p3, 0};
                float t1ys[4] = {0, colB.p2, colB.p4, 0};
                RotatePoints4(posA.x, posA.y, txs, tys, posA.rotation, colA.anchorX, colA.anchorY);
                RotatePoints4(posB.x, posB.y, t1xs, t1ys, posB.rotation, colB.anchorX, colB.anchorY);
                return SAT(txs, tys, t1xs, t1ys, info);
            }
        }
    }
} // namespace magique

#endif //DYNAMIC_COLLISIONSYSTEM_H