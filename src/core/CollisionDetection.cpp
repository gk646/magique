// SPDX-License-Identifier: zlib-acknowledgement
#include <utility>
#include <raylib/raylib.h>

#include <magique/core/CollisionDetection.h>
#include <magique/ecs/Components.h>
#include <magique/internal/Macros.h>

#include "internal/utils/CollisionPrimitives.h"

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
                            return RectToRect(pA.x, pA.y, cA.p1, cA.p2, pB.x, pB.y, cB.p1, cB.p2, i);
                        }
                        float pxs[4] = {0, cB.p1, cB.p1, 0};                                      // rect b
                        float pys[4] = {0, 0, cB.p2, cB.p2};                                      // rect b
                        RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY); // rot
                        const float p1xs[4] = {pA.x, pA.x + cA.p1, pA.x + cA.p1, pA.x};           // Rect a
                        const float p1ys[4] = {pA.y, pA.y, pA.y + cA.p2, pA.y + cA.p2};           // Rect a
                        return SAT(pxs, pys, p1xs, p1ys, i);
                    }
                    if (pB.rotation == 0) [[likely]] // Only A is rotated
                    {
                        float pxs[4] = {0, cA.p1, cA.p1, 0};                                      // rect a
                        float pys[4] = {0, 0, cA.p2, cA.p2};                                      // rect a
                        RotatePoints4(pA.x, pA.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY); // rot
                        const float p1xs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p1, pB.x};           // Rect b
                        const float p1ys[4] = {pB.y, pB.y, pB.y + cB.p2, pB.y + cB.p2};           // Rect b
                        return SAT(pxs, pys, p1xs, p1ys, i);
                    } // Both are rotated
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
                SAT(pxs, pys, txs, tys, i);
                return;
            }
            break;
        case Shape::CIRCLE:
            switch (cB.shape)
            {
            case Shape::RECT:
                {
                    if (pB.rotation == 0)
                    {
                        RectToCircle(pB.x, pB.y, cB.p1, cB.p2, pA.x + cA.p1, pA.y + cA.p1, cA.p1, i);
                        return;
                    }
                    float pxs[4] = {0, cB.p1, cB.p1, 0}; // rect b
                    float pys[4] = {0, 0, cB.p2, cB.p2}; // rect b
                    RotatePoints4(pB.x, pB.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY);
                    CircleToQuadrilateral(pA.x + cA.p1, pA.y + cA.p1, cA.p1, pxs, pys, i);
                    return;
                }
            case Shape::CIRCLE:
                // We can skip the translation to the middle point as both are in the same system
                CircleToCircle(pA.x + cA.p1, pA.y + cA.p1, cA.p1, pB.x + cB.p1, pB.y + cB.p1, cB.p1, i);
                return;
            case Shape::CAPSULE:
                CircleToCapsule(pA.x + cA.p1, pA.y + cA.p1, cA.p1, pB.x, pB.y, cB.p1, cB.p2, i);
                return;
            case Shape::TRIANGLE:
                {
                    if (pB.rotation == 0)
                    {
                        const float txs[4] = {pB.x, pB.x + cB.p1, pB.x + cB.p3, pB.x};
                        const float tys[4] = {pB.y, pB.y + cB.p2, pB.y + cB.p4, pB.y};
                        CircleToQuadrilateral(pA.x + cA.p1, pA.y + cA.p1, cA.p1, txs, tys, i);
                        return;
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.x, pB.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    CircleToQuadrilateral(pA.x + cA.p1, pA.y + cA.p1, cA.p1, txs, tys, i);
                    return;
                }
            }
            break;
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
                if (pA.rotation == 0) // For triangles, we don't assume as they are likely rotated
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

    void CheckCollisionEntityRect(const PositionC& pos, const CollisionC& col, const Rectangle& r, CollisionInfo& info)
    {
        switch (col.shape)
        {
        case Shape::RECT:
        {
            if (pos.rotation == 0) [[likely]]
            {
                return RectToRect(pos.x, pos.y, col.p1, col.p2, r.x, r.y, r.width, r.height, info);
            }
            // Entity
            float pxs[4] = {0, col.p1, col.p1, 0};
            float pys[4] = {0, 0, col.p2, col.p2};
            RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);

            // World rect
            const float p1xs[4] = {r.x, r.x + r.width, r.x + r.width, r.x};
            const float p1ys[4] = {r.y, r.y, r.y + r.height, r.y + r.height};
            return SAT(pxs, pys, p1xs, p1ys, info);
        }
        case Shape::CIRCLE:
        {
            return RectToCircle(r.x, r.y, r.width, r.height, pos.x + col.p1, pos.y + col.p1, col.p1, info);
        }
        case Shape::CAPSULE:
        {
            return RectToCapsule(r.x, r.y, r.width, r.height, pos.x, pos.y, col.p1, col.p2, info);
        }
        case Shape::TRIANGLE:
            const float rectX[4] = {r.x, r.x + r.width, r.x + r.width, r.x}; // World rect
            const float rectY[4] = {r.y, r.y, r.y + r.height, r.y + r.height};
            if (pos.rotation == 0) [[likely]]
            {
                const float triX[4] = {pos.x, pos.x + col.p1, pos.x + col.p3, pos.x};
                const float triY[4] = {pos.y, pos.y + col.p2, pos.y + col.p4, pos.y};
                return SAT(rectX, rectY, triX, triY, info);
            }
            float triX[4] = {0, col.p1, col.p3, 0};
            float triY[4] = {0, col.p2, col.p4, 0};
            RotatePoints4(pos.x, pos.y, triX, triY, pos.rotation, col.anchorX, col.anchorY);
            return SAT(rectX, rectY, triX, triY, info);
        }
    }

    void CheckCollisionRecCapsule(const Rectangle& rect, const Point pos, const float r, const float h,
                                  CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        RectToCapsule(rect.x, rect.y, rect.width, rect.height, pos.x, pos.y, r, h, info);
    }

    void CheckCollisionCircleToQuadrilateral(const Point center, const float r, const Point q1, const Point q2,
                                             const Point q3, const Point q4, CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        const float pxs[4] = {q1.x, q2.x, q3.x, q4.x};
        const float pys[4] = {q1.y, q2.y, q3.y, q4.y};
        CircleToQuadrilateral(center.x, center.y, r, pxs, pys, info);
    }

    void CheckCollisionCircleToCapsule(const Point center, const float r, const Point pos, const float cR,
                                       const float cH, CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        CircleToCapsule(center.x, center.y, r, pos.x, pos.y, cR, cH, info);
    }


    void CheckCollisionCapsuleCapsule(const Point p1, const float r1, const float h1, const Point p2, const float r2,
                                      const float h2, CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        CapsuleToCapsule(p1.x, p1.y, r1, h1, p2.x, p2.y, r2, h2, info);
    }

    void CheckCollisionCapsuleQuadrilateral(const Point p1, const float r1, const float h1, const Point q1,
                                            const Point q2, const Point q3, const Point q4, CollisionInfo& info)
    {
        MAGIQUE_ASSERT(info.isColliding() == false, "Not passing in a new CollisionInfo object");
        const float pxs[4] = {q1.x, q2.x, q3.x, q4.x};
        const float pys[4] = {q1.y, q2.y, q3.y, q4.y};
        CapsuleToQuadrilateral(p1.x, p1.y, r1, h1, pxs, pys, info);
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

    Rectangle GetEntityBoundingBox(const PositionC& pos, const CollisionC& col)
    {
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return {pos.x, pos.y, col.p1, col.p2};
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                return GetBBQuadrilateral(pxs, pys);
            }
        case Shape::CIRCLE:
            // Top left and diameter as w and h
            return {pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F};
        case Shape::CAPSULE:
            // Top left and height as height / diameter as w
            return {pos.x, pos.y, col.p1 * 2.0F, col.p2};
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    return GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3, pos.y + col.p4);
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                return GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
            }
        }
        return {};
    }

} // namespace magique