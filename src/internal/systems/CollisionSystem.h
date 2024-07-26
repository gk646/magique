#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <c2/cute_c2.h>

#include <magique/ecs/Scripting.h>
#include <magique/util/Jobs.h>
#include <magique/util/Defines.h>

#include "internal/globals/ScriptEngine.h"
#include "internal/headers/CollisionDetection.h"

// Needs major addition: Collect collision pairs and call them single threaded after
// Defer destruction of entities until after tick or just iterate pairs in order

// Give each thread a piece and the then main thread aswell - so it doesnt wait while doing nothing
static constexpr int WORK_PARTS = MAGIQUE_WORKER_THREADS + 1;

namespace magique
{
    inline bool CheckCollision(const PositionC&, const CollisionC&, const PositionC&, const CollisionC&);

    inline void HandleCollisionPairs(CollPairCollector& colPairs);

    inline void CollisionSystem(entt::registry& registry)
    {
        auto& tickData = global::LOGIC_TICK_DATA;
        auto& grid = tickData.hashGrid;
        auto& collisionVec = tickData.collisionVec;
        auto& collectors = tickData.collectors;
        auto& colPairs = tickData.collisionPairs;

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

                // Query quadtree
                grid.query<vector<entt::entity>>(collector, posA.x, posA.y, colA.width, colA.height);
                for (const auto second : collector)
                {
                    if (first >= second)
                        continue;
                    const auto [posB, colB] = registry.get<const PositionC, const CollisionC>(second);

                    // Not on the same map or not on the same collision layer
                    if (posA.map != posB.map || colA.layerMask & colB.layerMask == 0) [[unlikely]]
                        continue;

                    if (CheckCollision(posA, colA, posB, colB)) [[unlikely]]
                    {
                        pairs.emplace_back(first, second);
                    }
                }
                collector.clear();
            }
        };

        const int size = static_cast<int>(collisionVec.size());
        if (size > 700)
        {
            cxstructs::SmallVector<jobHandle, WORK_PARTS> handles;
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
                handles.push_back(AddJob(CreateExplicitJob(collisionCheck, j, start, end)));
            }
            AwaitJobs(handles);
        }
        else
        {
            collisionCheck(0, 0, size);
        }
#ifdef MAGIQUE_DEBUG_COLLISIONS
        printf("Detected Collisions: %d\n", collisions);
        int correct = 0;
        for (const auto first : collisionVec)
        {
            if (!registry.valid(first)) [[unlikely]]
                continue;
            for (const auto second : collisionVec)
            {
                if (first >= second || !registry.valid(second)) [[unlikely]]
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
        HandleCollisionPairs(colPairs);
    }

    inline void HandleCollisionPairs(CollPairCollector& colPairs)
    {
        int count = 0;
        for (auto& [vec] : colPairs)
        {
            count += vec.size();
            vec.clear();
        }
        printf("Pairs: %d\n", count);

        // Scripts are default filled - with this we can skip the script check
        // InvokeEventDirect<onDynamicCollision>(global::SCRIPT_ENGINE.scripts[posA.type], first, second);
        // Invoke out of seconds view
        //InvokeEventDirect<onDynamicCollision>(global::SCRIPT_ENGINE.scripts[posB.type], second, first);
    }

    inline static constexpr c2x identityTransform{{0, 0}, {1, 0}};

    inline c2Poly RotRect(const float x, const float y, const float width, const float height, const float rot,
                          const float anchorX, const float anchorY)
    {
        c2Poly poly;
        const float rad = rot * (DEG2RAD);
        const float cosTheta = cosf(rad);
        const float sinTheta = sinf(rad);

        // Define local space rectangle corners
        c2v localCorners[4] = {
            {0, 0},          // top-left
            {width, 0},      // top-right
            {width, height}, // bottom-right
            {0, height}      // bottom-left
        };

        // Rotate each corner around the local pivot and translate to world space
        for (int i = 0; i < 4; ++i)
        {
            float localX = localCorners[i].x - anchorX;
            float localY = localCorners[i].y - anchorY;

            // Apply rotation
            float rotatedX = localX * cosTheta - localY * sinTheta;
            float rotatedY = localX * sinTheta + localY * cosTheta;

            // Translate back and offset to world position
            poly.verts[i].x = x + rotatedX + anchorX;
            poly.verts[i].y = y + rotatedY + anchorY;
        }

        poly.count = 4;
        c2MakePoly(&poly);
        return poly;
    }

    inline bool CheckCollision(const PositionC& posA, const CollisionC& colA, const PositionC& posB,
                               const CollisionC& colB)
    {
        if (colA.shape == RECT && colB.shape == RECT && posA.rotation == 0 && posB.rotation == 0) [[likely]]
        {
            return RectIntersectsRect(posA.x, posA.y, colA.width, colA.height, posB.x, posB.y, colB.width, colB.height);
        }
        if (colA.shape == CIRCLE && colB.shape == CIRCLE)
        {
            const c2Circle circleA = {{posA.x + colA.width / 2.0F, posA.y + colA.height / 2.0F}, colA.width};
            const c2Circle circleB = {{posB.x + colB.width / 2.0F, posB.y + colB.height / 2.0F}, colB.width};
            return c2CircletoCircle(circleA, circleB) != 0;
        } // Handle circle-AABB collision
        if (colA.shape == CIRCLE && colB.shape == RECT)
        {
            const c2Circle circleA = {{posA.x + colA.width / 2.0F, posA.y + colA.height / 2.0F}, colA.width};
            const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + colB.width, posB.y + colB.height}};
            return c2CircletoAABB(circleA, aabbB) != 0;
        }
        if (colA.shape == RECT && colB.shape == CIRCLE)
        {
            const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + colA.width, posA.y + colA.height}};
            const c2Circle circleB = {{posB.x + colB.width / 2.0F, posB.y + colB.height / 2.0F}, colB.width};
            return c2CircletoAABB(circleB, aabbA) != 0;
        }
        // Handle AABB-AABB collision (considering rotation)
        if (colA.shape == RECT && colB.shape == RECT)
        {
            if (posA.rotation == 0)
            {
                const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + colA.width, posA.y + colA.height}};
                c2Poly polyB;
                polyB = RotRect(posB.x, posB.y, colB.width, colB.height, posB.rotation, colB.anchorX, colB.anchorY);
                return c2AABBtoPoly(aabbA, &polyB, &identityTransform) != 0;
            }
            if (posB.rotation == 0)
            {
                const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + colB.width, posB.y + colB.height}};
                c2Poly polyA;
                polyA = RotRect(posA.x, posA.y, colA.width, colA.height, posA.rotation, colA.anchorX, colA.anchorY);
                return c2AABBtoPoly(aabbB, &polyA, &identityTransform) != 0;
            }
            c2Poly polyA;
            polyA = RotRect(posA.x, posA.y, colA.width, colA.height, posA.rotation, colA.anchorX, colA.anchorY);

            c2Poly polyB;
            polyB = RotRect(posB.x, posB.y, colB.width, colB.height, posB.rotation, colB.anchorX, colB.anchorY);
            return c2PolytoPoly(&polyA, &identityTransform, &polyB, &identityTransform) != 0;
        }
        return false;
    }
} // namespace magique

#endif //COLLISIONSYSTEM_H