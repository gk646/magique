#pragma once
#ifndef COLLISIONSYSTEM_H
#define COLLISIONSYSTEM_H

#include "core/datastructures/HashGrid.h"

#include <c2/cute_c2.h>


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
    if (colA.shape == AABB && colB.shape == AABB && posA.rotation == 0 && posB.rotation == 0) [[likely]]
    {
        const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + posA.width, posA.y + posA.height}};
        const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + posB.width, posB.y + posB.height}};
        return c2AABBtoAABB(aabbA, aabbB) != 0;
    }
    if (colA.shape == CIRCLE && colB.shape == CIRCLE)
    {
        const c2Circle circleA = {{posA.x + posA.width / 2.0F, posA.y + posA.height / 2.0F},
                                  static_cast<float>(posA.width)};
        const c2Circle circleB = {{posB.x + posB.width / 2.0F, posB.y + posB.height / 2.0F},
                                  static_cast<float>(posB.width)};
        return c2CircletoCircle(circleA, circleB) != 0;
    } // Handle circle-AABB collision
    if (colA.shape == CIRCLE && colB.shape == AABB)
    {
        const c2Circle circleA = {{posA.x + posA.width / 2.0F, posA.y + posA.height / 2.0F},
                                  static_cast<float>(posA.width)};
        const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + posB.width, posB.y + posB.height}};
        return c2CircletoAABB(circleA, aabbB) != 0;
    }
    if (colA.shape == AABB && colB.shape == CIRCLE)
    {
        const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + posA.width, posA.y + posA.height}};
        const c2Circle circleB = {{posB.x + posB.width / 2.0F, posB.y + posB.height / 2.0F},
                                  static_cast<float>(posB.width)};
        return c2CircletoAABB(circleB, aabbA) != 0;
    }
    // Handle AABB-AABB collision (considering rotation)
    if (colA.shape == AABB && colB.shape == AABB)
    {
        if (posA.rotation == 0)
        {
            const c2AABB aabbA = {{posA.x, posA.y}, {posA.x + posA.width, posA.y + posA.height}};
            c2Poly polyB;
            polyB = RotRect(posB.x, posB.y, posB.width, posB.height, posB.rotation, colB.anchorX, colB.anchorY);
            return c2AABBtoPoly(aabbA, &polyB, &identityTransform) != 0;
        }
        if (posB.rotation == 0)
        {
            const c2AABB aabbB = {{posB.x, posB.y}, {posB.x + posB.width, posB.y + posB.height}};
            c2Poly polyA;
            polyA = RotRect(posA.x, posA.y, posA.width, posA.height, posA.rotation, colA.anchorX, colA.anchorY);
            return c2AABBtoPoly(aabbB, &polyA, &identityTransform) != 0;
        }
        c2Poly polyA;
        polyA = RotRect(posA.x, posA.y, posA.width, posA.height, posA.rotation, colA.anchorX, colA.anchorY);

        c2Poly polyB;
        polyB = RotRect(posB.x, posB.y, posB.width, posB.height, posB.rotation, colB.anchorX, colB.anchorY);
        return c2PolytoPoly(&polyA, &identityTransform, &polyB, &identityTransform) != 0;
    }
    return false;
}

namespace magique::ecs
{
    inline void CheckCollisions(entt::registry& registry)
    {



    }
} // namespace magique::ecs

#endif //COLLISIONSYSTEM_H