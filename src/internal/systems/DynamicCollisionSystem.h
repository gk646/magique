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
//       have faster lookups times but MUCH more complex to manage and move objects efficiently
// -> But doesn't change anything for user
// .....................................................................

namespace magique
{
    void CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&, CollisionInfo& i);
    void HandleCollisionPairs();
    void CheckHashGridCells(float beginPercent, float endPercent, int thread);

    //----------------- SYSTEM -----------------//

    inline void DynamicCollisionSystem()
    {
        const int size = global::ENGINE_DATA.collisionVec.size(); // Multithreading over certain amount
        if (size > 1)
        {
            std::array<jobHandle, WORK_PARTS> handles{};
            constexpr float mainThreadPart = 100.0F / WORK_PARTS * 1.25F; // 25% more work for main thread
            constexpr float workerPart = (100.0F - mainThreadPart) / (WORK_PARTS - 1);
            float beginPercent = 0.0F;
            for (int j = 0; j < WORK_PARTS - 1; ++j)
            {
                handles[j] = AddJob(CreateExplicitJob(CheckHashGridCells, beginPercent, beginPercent + workerPart, j));
                beginPercent += workerPart;
            }
            CheckHashGridCells(beginPercent, 1.0F, WORK_PARTS - 1);
            AwaitJobs(handles);
        }
        else
        {
            CheckHashGridCells(0.0F, 1.0F, WORK_PARTS - 1);
        }
        HandleCollisionPairs();
    }

    //----------------- IMPLEMENTATION -----------------//

    inline void CheckHashGridCells(const float beginP, const float endP, const int thread)
    {
        const auto& data = global::ENGINE_DATA;
        auto& dynamic = global::DY_COLL_DATA;
        auto& pairs = dynamic.collisionPairs[thread].vec;
        for (const auto loadedMap : data.loadedMaps)
        {
            const auto& hashGrid = dynamic.mapEntityGrids[loadedMap];
            const int size = static_cast<int>(hashGrid.cellMap.size());
            if (size < WORK_PARTS && thread != WORK_PARTS-1) // If cant be split into parts let main thread do it alone
                continue;

            const int startIdx = static_cast<int>(beginP * static_cast<float>(size));
            const int endIdx = static_cast<int>(endP * static_cast<float>(size));
            const auto start = hashGrid.dataBlocks.begin() + startIdx;
            const auto end = hashGrid.dataBlocks.begin() + endIdx;
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
                        CheckCollisionEntities(posA, colA, posB, colB, info);
                        if (info.isColliding())
                        {
                            pairs.push_back(PairInfo{info, colA, colB, posA, posB, first, second});
                        }
                    }
                }
            }
        }
    }

    inline void HandleCollisionPairs()
    {
        const auto& scriptVec = global::SCRIPT_DATA.scripts;
        const auto& colVec = global::ENGINE_DATA.collisionVec;
        const auto& group = internal::POSITION_GROUP;
        auto& dynamic = global::DY_COLL_DATA;

        auto& colPairs = dynamic.collisionPairs;
        auto& pairSet = dynamic.pairSet;
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
            if (col.resolutionVec.x != 0.0F && col.resolutionVec.y != 0.0F)
            {
                volatile int b = 123;
            }
            pos.x += col.resolutionVec.x;
            pos.y += col.resolutionVec.y;
            col.clearCollisionData();
        }
        pairSet.clear();
    }

} // namespace magique

#endif //DYNAMIC_COLLISION_SYSTEM_H