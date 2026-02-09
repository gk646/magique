// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/core/CollisionDetection.h>
#include <magique/ecs/Components.h>
#include <magique/internal/Macros.h>

#include "internal/utils/CollisionPrimitives.h"
#include "magique/core/Camera.h"

namespace magique
{

    // Should be the most efficient way - allows jump tables and inlining - this is actually very fast!
    // With 15k entities skipping all switches and returning immediately only saves around 0.1 ms
    void CheckCollisionEntities(const PositionC& pA, const CollisionC& cA, const PositionC& pB, const CollisionC& cB,
                                CollisionInfo& i)
    {
        MAGIQUE_ASSERT(i.isColliding() == false, "Not passing in a new CollisionInfo object");
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
                            return RectToRect(pA.pos.x + cA.offset.x, pA.pos.y + cA.offset.y, cA.p1, cA.p2,
                                              pB.pos.x + cB.offset.x, pB.pos.y + cB.offset.y, cB.p1, cB.p2, i);
                        }
                        else
                        {
                            RECT_TO_POINTS(pa, pA.pos, cA);
                            RECT_ROTATE_POINTS(pb, pB, cB);
                            return SAT(paX, paY, pbX, pbY, i);
                        }
                    }
                    else if (pB.rotation == 0) [[likely]] // Only A is rotated
                    {
                        RECT_ROTATE_POINTS(pa, pA, cA);
                        RECT_TO_POINTS(pb, pB.pos, cB);
                        return SAT(paX, paY, pbX, pbY, i);
                    }
                    else // Both are rotated
                    {
                        RECT_ROTATE_POINTS(pa, pA, cA);
                        RECT_ROTATE_POINTS(pb, pB, cB);
                        return SAT(paX, paY, pbX, pbY, i);
                    }
                }
            case Shape::CIRCLE:
                {
                    if (pA.rotation == 0)
                    {
                        return RectToCircle(pA.pos.x + cA.offset.x, pA.pos.y + cA.offset.y, cA.p1, cA.p2,
                                            pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, i);
                    }
                    else
                    {
                        RECT_ROTATE_POINTS(pa, pA, cA);
                        return QuadrilateralToCircle(paX, paY, pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, i);
                    }
                }
            case Shape::TRIANGLE:
                {
                    if (pA.rotation == 0) [[likely]]
                    {
                        if (pB.rotation == 0)
                        {
                            RECT_TO_POINTS(pa, pA.pos, cA);
                            TRI_TO_POINTS(pb, pB.pos, cB);
                            return SAT(paX, paY, pbX, pbY, i);
                        }
                        else
                        {
                            RECT_TO_POINTS(pa, pA.pos, cA);
                            TRI_ROTATE_POINTS(pb, pB, cB);
                            return SAT(paX, paY, pbX, pbY, i);
                        }
                    }
                    else if (pB.rotation == 0)
                    {
                        RECT_ROTATE_POINTS(pa, pA, cA);
                        TRI_TO_POINTS(pb, pB.pos, cB);
                        return SAT(paX, paY, pbX, pbY, i);
                    }
                    else
                    {
                        RECT_ROTATE_POINTS(pa, pA, cA);
                        TRI_ROTATE_POINTS(pb, pB, cB);
                        return SAT(paX, paY, pbX, pbY, i);
                    }
                }
            }
            break;
        case Shape::CIRCLE:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pB.rotation == 0)
                    {
                        return CircleToRect(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pB.pos.x + cB.offset.x,
                                            pB.pos.y + cB.offset.y, cB.p1, cB.p2, i);
                    }
                    else
                    {
                        RECT_ROTATE_POINTS(pb, pB, cB)
                        return CircleToQuadrilateral(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pbX, pbY, i);
                    }
                }
            case Shape::CIRCLE:
                {
                    // We can skip the translation to the middle point as both are in the same system
                    return CircleToCircle(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pB.pos.x + cB.p1, pB.pos.y + cB.p1,
                                          cB.p1, i);
                }
            case Shape::TRIANGLE:
                {
                    if (pB.rotation == 0)
                    {
                        TRI_TO_POINTS(pb, pB.pos, cB)
                        return CircleToQuadrilateral(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pbX, pbY, i);
                    }
                    else
                    {
                        TRI_ROTATE_POINTS(pb, pB, cB)
                        return CircleToQuadrilateral(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pbX, pbY, i);
                    }
                }
            }
            break;
        case Shape::TRIANGLE:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pA.rotation == 0) [[likely]]
                    {
                        if (pB.rotation == 0) [[likely]]
                        {
                            TRI_TO_POINTS(pa, pA.pos, cA)
                            RECT_TO_POINTS(pb, pB.pos, cB);
                            return SAT(paX, paY, pbX, pbY, i);
                        }
                        else
                        {
                            TRI_TO_POINTS(pa, pA.pos, cA)
                            RECT_ROTATE_POINTS(pb, pB, cB);
                            return SAT(paX, paY, pbX, pbY, i);
                        }
                    }
                    else if (pB.rotation == 0)
                    {
                        TRI_ROTATE_POINTS(pa, pA, cA)
                        RECT_TO_POINTS(pb, pB.pos, cB);
                        return SAT(paX, paY, pbX, pbY, i);
                    }
                    else
                    {
                        TRI_ROTATE_POINTS(pa, pA, cA)
                        RECT_ROTATE_POINTS(pb, pB, cB);
                        return SAT(paX, paY, pbX, pbY, i);
                    }
                }
            case Shape::CIRCLE:
                {
                    if (pA.rotation == 0)
                    {
                        TRI_TO_POINTS(pa, pA.pos, cA)
                        return QuadrilateralToCircle(paX, paY, pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, i);
                    }
                    TRI_ROTATE_POINTS(pa, pA, cA)
                    return QuadrilateralToCircle(paX, paY, pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, i);
                }
            case Shape::TRIANGLE:
                {
                    TRI_ROTATE_POINTS(pa, pA, cA)
                    TRI_ROTATE_POINTS(pb, pB, cB)
                    return SAT(paX, paY, pbX, pbY, i);
                }
            }
            break;
        }
    }

    void CheckCollisionEntityRect(const PositionC& pos, const CollisionC& col, const Rect& r, CollisionInfo& info)
    {
        // Avoids doubling logic - like offset handling
        const PositionC posR{r.pos()};
        const CollisionC colR{r.w, r.h, 0, 0, {}, {}, Shape::RECT};
        CheckCollisionEntities(pos, col, posR, colR, info);
    }

    bool CheckCollisionEntityMouse(const PositionC& pos, const CollisionC& col)
    {
        CollisionInfo info;
        const auto worldMouse = GetScreenToWorld2D(GetMousePosition(), CameraGet());
        CheckCollisionEntityRect(pos, col, Rect{worldMouse.x, worldMouse.y, 1, 1}, info);
        return info.isColliding();
    }

    void CheckCollisionCircleToQuadrilateral(const Point center, const float r, const Point q1, const Point q2,
                                             const Point q3, const Point q4, CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        const float pxs[4] = {q1.x, q2.x, q3.x, q4.x};
        const float pys[4] = {q1.y, q2.y, q3.y, q4.y};
        CircleToQuadrilateral(center.x, center.y, r, pxs, pys, info);
    }

    void CheckCollisionQuadrilaterals(const Point p1, const Point p2, const Point p3, const Point p4, const Point q1,
                                      const Point q2, const Point q3, const Point q4, CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        const float pxs[4] = {p1.x, p2.x, p3.x, p4.x};
        const float pys[4] = {p1.y, p2.y, p3.y, p4.y};
        const float qxs[4] = {q1.x, q2.x, q3.x, q4.x};
        const float qys[4] = {q1.y, q2.y, q3.y, q4.y};
        SAT(pxs, pys, qxs, qys, info);
    }

    void RotatePoints(const float angle, const Point anchor, Point& p1, Point& p2, Point& p3, Point& p4)
    {
        float pxs[4] = {p1.x, p2.x, p3.x, p4.x};
        float pys[4] = {p1.y, p2.y, p3.y, p4.y};
        RotatePoints4(0, 0, pxs, pys, angle, anchor.x, anchor.y);
        p1.x = pxs[0];
        p1.y = pys[0];
        p2.x = pxs[1];
        p2.y = pys[1];
        p3.x = pxs[2];
        p3.y = pys[2];
        p4.x = pxs[3];
        p4.y = pys[3];
    }


} // namespace magique
