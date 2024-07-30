#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <magique/ecs/Scripting.h>
#include <magique/util/Jobs.h>
#include <magique/util/Defines.h>

#include "internal/globals/ScriptEngine.h"
#include "internal/headers/MathPrimitives.h"

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
    inline bool CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&);
    inline void HandleCollisionPairs(CollPairCollector& colPairs, HashSet<uint64_t>& pairSet);
    inline void QueryHashGrid(vector<entt::entity>& collector, const HashGrid&, float x, float y, const CollisionC& col);

    // System
    inline void CollisionSystem(entt::registry& registry)
    {
        auto& tickData = global::LOGIC_TICK_DATA;
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
                QueryHashGrid(collector, grid, posA.x, posA.y, colA);
                for (const auto second : collector)
                {
                    if (first >= second)
                        continue;
                    const auto [posB, colB] = registry.get<const PositionC, const CollisionC>(second);
                    if (posA.map != posB.map || (colA.layerMask & colB.layerMask) == 0) [[unlikely]]
                        continue; // Not on the same map or not on the same collision layer
                    if (CheckCollision(posA, colA, posB, colB)) [[unlikely]]
                    {
                        pairs.emplace_back(first, posA.type, second, posB.type);
                    }
                }
                collector.clear();
            }
        };

        const int size = static_cast<int>(collisionVec.size());
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

    inline void QueryHashGrid(vector<entt::entity>& collector, const HashGrid& grid, const float x, const float y,
                              const CollisionC& col)
    {
        //TODO missing rotatino - same as LogicSystem
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            grid.query<vector<entt::entity>>(collector, x, y, col.p1, col.p2);
            break;
        case Shape::CAPSULE:
            grid.query<vector<entt::entity>>(collector, x, y, col.p1 * 2.0F, col.p2);
            break;
        case Shape::CIRCLE:
            grid.query<vector<entt::entity>>(collector, x, y, col.p1, col.p1);
            break;
        case Shape::TRIANGLE:
            LOG_FATAL("Method not implemented");
            break;
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
                InvokeEventDirect<onDynamicCollision>(global::SCRIPT_ENGINE.scripts[id1], e1, e2);
                // Invoke out of seconds view
                InvokeEventDirect<onDynamicCollision>(global::SCRIPT_ENGINE.scripts[id2], e2, e1);
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
                if (posA.rotation == 0) [[likely]]
                {
                    if (posB.rotation == 0) [[likely]]
                    {
                        return RectToRect(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2);
                    }
                    LOG_FATAL("Method not implemented");
                    return false;
                }
                if (posB.rotation == 0) [[likely]]
                {
                    LOG_FATAL("Method not implemented");
                }
                else
                {
                    LOG_FATAL("Method not implemented");
                }
                return false;

            case Shape::CIRCLE:
                if (posA.rotation == 0)
                {
                    return RectToCircle(posA.x, posA.y, colA.p1, colA.p2, posB.x + colB.p1 / 2.0F,
                                        posB.y + colB.p1 / 2.0F, colB.p1);
                }
                LOG_FATAL("Method not implemented");

            case Shape::CAPSULE:
                if (posA.rotation == 0) [[likely]]
                {
                    return RectToCapsule(posA.x, posA.y, colA.p1, colA.p2, posB.x, posB.y, colB.p1, colB.p2);
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                LOG_FATAL("Method not implemented");
                break;
            }
        case Shape::CIRCLE:
            switch (colB.shape)
            {
            case Shape::RECT:
                if (posB.rotation == 0)
                {
                    return RectToCircle(posB.x, posB.y, colB.p1, colB.p2, posA.x + colA.p1 / 2.0F,
                                        posA.y + colA.p1 / 2.0F, colA.p1);
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CIRCLE:
                return CircleToCircle(posA.x, posA.y, colA.p1, posB.x, posB.y, colB.p2);
            case Shape::CAPSULE:
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                LOG_FATAL("Method not implemented");
                break;
            }
        case Shape::CAPSULE:
            switch (colB.shape)
            {
            case Shape::RECT:
                if (posB.rotation == 0) [[likely]]
                {
                    return RectToCapsule(posB.x, posB.y, colB.p1, colB.p2, posA.x, posA.y, colA.p1, colA.p2);
                }
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CIRCLE:
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CAPSULE:
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
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CIRCLE:
                LOG_FATAL("Method not implemented");
                break;
            case Shape::CAPSULE:
                LOG_FATAL("Method not implemented");
                break;
            case Shape::TRIANGLE:
                LOG_FATAL("Method not implemented");
                break;
            }
        }
        return false;
    }
} // namespace magique

#endif //COLLISIONSYSTEM_H