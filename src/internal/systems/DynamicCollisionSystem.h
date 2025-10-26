// SPDX-License-Identifier: zlib-acknowledgement
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

// Problems:
// - Sticky corners
// - Double collisions
// - Tunneling

// Ouickfixes:
// - Detect corner collisions and skip  => contributes to tunneling
// - Negate same normal two times in a row

// Bigger changes:
// - Sort entities/collision after their distance of collision points
// - Subticks

// Plan:
// - Keep the whole system
// - Apply specialized solutions

// Solution:
// save when there is a rectangle collision in a direction

// When a corner collisions occurs
// - for vertical stick edge:
//      - when resolution happens horizontally right or left and vertically against the movement direction
//      - collision point x is the same
// - for horizontal sticky edge:
//      - when resolution happens vertically up or down and horizontally against the movement direction
//      - collision point y is the same
// Double collisions:
// - save in each direction only the greatest extent

// i just sat there and thought for a day, no online solutions were really good
// and then at some point i thought when this case actually happens,
// and how you can detect it in O(1) time while preserving multithreading

namespace magique
{
    void CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&, CollisionInfo& i);
    void HandleCollisionPairs();
    void CheckHashGridCells(float beginPercent, float endPercent, int thread);

    //----------------- SYSTEM -----------------//

    inline void DynamicCollisionSystem()
    {
        const int size = global::ENGINE_DATA.collisionVec.size();
        if (size > 500) // Multithreading over certain amount
        {
            std::array<jobHandle, COL_WORK_PARTS> handles{};
            constexpr float mainThreadPart = 1.0F / COL_WORK_PARTS * 1.25F; // 25% more work for main thread
            constexpr float workerPart = (1.0F - mainThreadPart) / (COL_WORK_PARTS - 1);
            float beginPercent = 0.0F;
            for (int j = 0; j < COL_WORK_PARTS - 1; ++j)
            {
                handles[j] = AddJob(CreateExplicitJob(CheckHashGridCells, beginPercent, beginPercent + workerPart, j));
                beginPercent += workerPart;
            }
            CheckHashGridCells(beginPercent, 1.0F, COL_WORK_PARTS - 1);
            AwaitJobs(handles);
        }
        else
        {
            CheckHashGridCells(0.0F, 1.0F, COL_WORK_PARTS - 1);
        }
        HandleCollisionPairs();
    }

    //----------------- IMPLEMENTATION -----------------//

    inline void CheckHashGridCells(const float beginP, const float endP, const int thread)
    {
        const auto& data = global::ENGINE_DATA;
        auto& dynamic = global::DY_COLL_DATA;
        const auto& group = internal::POSITION_GROUP;

        auto& pairs = dynamic.collisionPairs[thread].vec;
        for (const auto loadedMap : data.loadedMaps)
        {
            const auto& hashGrid = dynamic.mapEntityGrids[loadedMap];
            const int size = static_cast<int>(hashGrid.cellMap.size());
            if (size < COL_WORK_PARTS && thread != COL_WORK_PARTS - 1)
            {
                // If cant be split into parts let main thread do it alone
                continue;
            }

            const int startIdx = static_cast<int>(beginP * static_cast<float>(size));
            const int endIdx = static_cast<int>(endP * static_cast<float>(size));
            const auto* start = hashGrid.dataBlocks.begin() + startIdx;
            const auto* end = hashGrid.dataBlocks.begin() + endIdx;
            for (const auto* it = start; it != end; ++it)
            {
                const auto& block = *it;
                const auto* dStart = block.data;
                const auto* dEnd = block.data + block.size;
                for (const auto* dIt1 = dStart; dIt1 != dEnd; ++dIt1)
                {
                    const auto first = *dIt1;
                    auto [posA, colA] = group.get<const PositionC, CollisionC>(first);
                    for (const auto* dIt2 = dIt1 + 1; dIt2 != dEnd; ++dIt2)
                    {
                        const auto second = *dIt2;
                        auto [posB, colB] = group.get<const PositionC, CollisionC>(second);
                        if (!colA.detects(colB) && !colB.detects(colA))
                        {
                            continue; // Not checking for each other
                        }
                        CollisionInfo info{};
                        CheckCollisionEntities(posA, colA, posB, colB, info);
                        if (info.isColliding())
                        {
                            pairs.push_back(PairInfo{info, first, second});
                        }
                    }
                }
            }
        }
    }

    inline void HandleCollisionPairs()
    {
        const auto& scriptVec = global::SCRIPT_DATA.scripts;
        const auto& group = internal::POSITION_GROUP;
        auto& dynamic = global::DY_COLL_DATA;

        auto& colPairs = dynamic.collisionPairs;
        auto& pairSet = dynamic.pairSet;

        for (auto& [vec] : colPairs)
        {
            for (auto& pairInfo : vec)
            {
                const auto e1 = pairInfo.e1;
                const auto e2 = pairInfo.e2;

                // This cannot be avoided as duplicates are inserted into the hashgrid
                if (dynamic.isMarked(e1, static_cast<uint32_t>(e2)))
                {
                    continue;
                }

                // this checks existence as well - also needed cause deletion caused reference invalidation
                const auto p1 = TryGetComponent<const PositionC>(e1);
                const auto p2 = TryGetComponent<const PositionC>(e2);
                if (p1 == nullptr || p2 == nullptr) [[unlikely]]
                {
                    continue;
                }
                auto& col1 = GetComponent<CollisionC>(e1);
                auto& col2 = GetComponent<CollisionC>(e2);

                // Prepare second info - with the fresh data
                auto secondInfo = pairInfo.info;
                secondInfo.normalVector.x *= -1;
                secondInfo.normalVector.y *= -1;

                if (col1.detects(col2))
                {
                    // Already checked if both entities exist
                    InvokeEventDirect<onDynamicCollision>(scriptVec[p1->type], e1, e2, pairInfo.info);

                    if (pairInfo.info.getIsAccumulated())
                    {
                        AccumulateInfo(col1, col2.shape, pairInfo.info);
                    }
                }

                if (col2.detects(col1))
                {
                    // Call for second entity
#if MAGIQUE_CHECK_EXISTS_BEFORE_EVENT == 1
                    bool invokeEvent = group.contains(e1) && group.contains(e2); // Needs recheck as first could delete
                    if (invokeEvent)
#endif
                        InvokeEventDirect<onDynamicCollision>(scriptVec[p2->type], e2, e1, secondInfo);

                    if (secondInfo.getIsAccumulated())
                    {
                        AccumulateInfo(col2, col2.shape, secondInfo);
                    }
                }
            }
            vec.clear();
        }
        pairSet.clear();
    }

} // namespace magique

#endif //DYNAMIC_COLLISION_SYSTEM_H
