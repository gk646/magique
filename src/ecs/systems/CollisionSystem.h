#pragma once
#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include <magique/ecs/InternalScripting.h>
#include <magique/util/Jobs.h>

#include <c2/cute_c2.h>
#include <cxutil/cxtime.h>

#include "core/Config.h"
#include "ecs/ScriptEngine.h"


inline c2Poly RotRect(float x, float y, float width, float height, float rot, float anchorX, float anchorY)
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
                           const CollisionC& colB) noexcept
{
    constexpr c2x identityTransform{{0, 0}, {1, 0}};
    // both shapes are non-rotated AABBs
    if (colA.shape == magique::RECT && colB.shape == magique::RECT && posA.rotation == 0 && posB.rotation == 0)
        [[likely]]
    {
        const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + colA.width, posA.y + colA.height}};
        const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + colB.width, posB.y + colB.height}};
        return c2AABBtoAABB(aabbA, aabbB) != 0;
    }
    if (colA.shape == magique::CIRCLE && colB.shape == magique::CIRCLE)
    {
        const c2Circle circleA = {{posA.x + colA.width / 2.0F, posA.y + colA.height / 2.0F},
                                  static_cast<float>(colA.width)};
        const c2Circle circleB = {{posB.x + colB.width / 2.0F, posB.y + colB.height / 2.0F},
                                  static_cast<float>(colB.width)};
        return c2CircletoCircle(circleA, circleB) != 0;
    } // Handle circle-AABB collision
    if (colA.shape == magique::CIRCLE && colB.shape == magique::RECT)
    {
        const c2Circle circleA = {{posA.x + colA.width / 2.0F, posA.y + colA.height / 2.0F},
                                  static_cast<float>(colA.width)};
        const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + colB.width, posB.y + colB.height}};
        return c2CircletoAABB(circleA, aabbB) != 0;
    }
    if (colA.shape == magique::RECT && colB.shape == magique::CIRCLE)
    {
        const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + colA.width, posA.y + colA.height}};
        const c2Circle circleB = {{posB.x + colB.width / 2.0F, posB.y + colB.height / 2.0F},
                                  static_cast<float>(colB.width)};
        return c2CircletoAABB(circleB, aabbA) != 0;
    }
    // Handle AABB-AABB collision (considering rotation)
    if (colA.shape == magique::RECT && colB.shape == magique::RECT)
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

namespace magique
{
    inline void CheckCollisions(entt::registry& registry)
    {
        auto& tickData = global::LOGIC_TICK_DATA;
        auto& grid = tickData.hashGrid;
        auto& updateVec = tickData.entityUpdateVec;
        auto& collectors = tickData.collectors;

#ifdef MAGIQUE_DEBUG_COLLISIONS
        int collisions = 0;
#endif

        const auto collisionCheck = [&](const int j, const int startIdx, const int endIdx)
        {
            const auto start = updateVec.begin() + startIdx;
            const auto end = updateVec.begin() + endIdx;
            for (auto it = start; it != end; ++it)
            {
                const auto first = *it;
                const auto [posA, colA] = registry.get<const PositionC, const CollisionC>(first);
                auto* firstScript = SCRIPT_ENGINE.scripts[posA.type];
                // Query quadtree
                grid.query<HashSet<entt::entity>>(collectors[j], posA.x, posA.y, colA.width, colA.height);
                for (const auto second : collectors[j])
                {
                    if (first >= second)
                        continue;

                    auto [posB, colB] = registry.get<const PositionC, const CollisionC>(second);

                    if (posA.map != posB.map || colA.layerMask & colB.layerMask != 0) [[unlikely]]
                        continue;

                    if (CheckCollision(posA, colA, posB, colB)) [[unlikely]]
                    {
                        auto* secondScript = SCRIPT_ENGINE.scripts[posB.type];
                        InvokeEventDirect<onDynamicCollision>(firstScript, first, second);
                        // Invoke out of seconds view
                        InvokeEventDirect<onDynamicCollision>(secondScript, second, first);
#ifdef MAGIQUE_DEBUG_COLLISIONS
                        collisions++;
#endif
                    }
                }
                collectors[j].clear();
            }
        };

        //cxstructs::now();
        constexpr int parts = 4;
        std::vector<std::thread> threads;
        threads.reserve(5);
        int start = 0;
        int end = 0;
        const int partSize = updateVec.size() / parts;
        for (int j = 0; j < parts; ++j)
        {
            start = end;
            end = start + partSize;
            if (j == parts - 1)
            {
                collisionCheck(j, start, updateVec.size());
                break;
            }

            if (start - end == 0)
                continue;

            threads.emplace_back(collisionCheck, j, start, end);
        }

        for (auto& t : threads)
        {
            t.join();
        }

        //cxstructs::printTime<std::chrono::nanoseconds>();

#ifdef MAGIQUE_DEBUG_COLLISIONS
        printf("Detected Collisions: %d\n", collisions);
        int correct = 0;
        for (const auto first : updateVec)
        {
            for (const auto second : updateVec)
            {
                if (first >= second)
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
    }
} // namespace magique

#endif //COLLISIONSYSTEM_H