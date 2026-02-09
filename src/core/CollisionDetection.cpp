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
                        float pxs[4] = {0, cB.p1, cB.p1, 0}; // rect b
                        float pys[4] = {0, 0, cB.p2, cB.p2}; // rect b
                        RotatePoints4(pB.pos.x + cB.offset.x, pB.pos.y + cB.offset.y, pxs, pys, pB.rotation, cB.anchorX,
                                      cB.anchorY); // rot
                        const float p1xs[4] = {pA.pos.x + cA.offset.x, pA.pos.x + cA.offset.x + cA.p1,
                                               pA.pos.x + cA.offset.x + cA.p1, pA.pos.x + cA.offset.x}; // Rect a
                        const float p1ys[4] = {pA.pos.y + cA.offset.y, pA.pos.y + cA.offset.y,
                                               pA.pos.y + cA.p2 + cA.offset.y, pA.pos.y + cA.p2 + cA.offset.y}; // Rect a
                        return SAT(pxs, pys, p1xs, p1ys, i);
                    }
                    if (pB.rotation == 0) [[likely]] // Only A is rotated
                    {
                        float pxs[4] = {0, cA.p1, cA.p1, 0}; // rect a
                        float pys[4] = {0, 0, cA.p2, cA.p2}; // rect a
                        RotatePoints4(pA.pos.x + cA.offset.x, pA.pos.y + cA.offset.y, pxs, pys, pA.rotation, cA.anchorX,
                                      cA.anchorY); // rot
                        const float p1xs[4] = {pB.pos.x + cB.offset.x, pB.pos.x + cB.p1 + cB.offset.x,
                                               pB.pos.x + cB.p1 + cB.offset.x, pB.pos.x + cB.offset.x}; // Rect b
                        const float p1ys[4] = {pB.pos.y + cB.offset.y, pB.pos.y + cB.offset.y,
                                               pB.pos.y + cB.p2 + cB.offset.y, pB.pos.y + cB.p2 + cB.offset.y}; // Rect b
                        return SAT(pxs, pys, p1xs, p1ys, i);
                    } // Both are rotated
                    float pxs[4] = {0, cA.p1, cA.p1, 0};  // rect a
                    float pys[4] = {0, 0, cA.p2, cA.p2};  // rect a
                    float p1xs[4] = {0, cB.p1, cB.p1, 0}; // Rect b
                    float p1ys[4] = {0, 0, cB.p2, cB.p2}; // Rect b
                    RotatePoints4(pB.pos.x + cB.offset.x, pB.pos.y + cB.offset.y, pxs, pys, pA.rotation, cA.anchorX,
                                  cA.anchorY);
                    RotatePoints4(pB.pos.x + cB.offset.x, pB.pos.y + cB.offset.y, p1xs, p1ys, pB.rotation, cB.anchorX,
                                  cB.anchorY);
                    return SAT(pxs, pys, p1xs, p1ys, i);
                }
            case Shape::CIRCLE:
                {
                    if (pA.rotation == 0)
                    {
                        return RectToCircle(pA.pos.x + cA.offset.x, pA.pos.y + cA.offset.y, cA.p1, cA.p2,
                                            pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, i);
                    }
                    float pxs[4] = {0, cA.p1, cA.p1, 0}; // rect a
                    float pys[4] = {0, 0, cA.p2, cA.p2}; // rect a
                    RotatePoints4(pA.pos.x + cA.offset.x, pA.pos.y + cA.offset.y, pxs, pys, pA.rotation, cA.anchorX,
                                  cA.anchorY);
                    return CircleToQuadrilateral(pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, pxs, pys, i);
                }
            case Shape::TRIANGLE:
                if (pA.rotation == 0) [[likely]]
                {
                    if (pB.rotation == 0)
                    {
                        const float rectX[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p1, pA.pos.x};
                        const float rectY[4] = {pA.pos.y, pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p2};

                        const float triX[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p3, pB.pos.x};
                        const float triY[4] = {pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p4, pB.pos.y};
                        return SAT(rectX, rectY, triX, triY, i);
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.pos.x, pB.pos.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    const float p1xs[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p1, pA.pos.x}; // Rect a
                    const float p1ys[4] = {pA.pos.y, pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p2}; // Rect a
                    return SAT(txs, tys, p1xs, p1ys, i);
                }
                if (pB.rotation == 0)
                {
                    float pxs[4] = {0, cA.p1, cA.p1, 0};
                    float pys[4] = {0, 0, cA.p2, cA.p2};
                    RotatePoints4(pA.pos.x, pA.pos.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY);
                    const float txs[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p3, pB.pos.x};
                    const float tys[4] = {pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p4, pB.pos.y};
                    return SAT(pxs, pys, txs, tys, i);
                }
                float pxs[4] = {0, cA.p1, cA.p1, 0};
                float pys[4] = {0, 0, cA.p2, cA.p2};
                RotatePoints4(pA.pos.x, pA.pos.y, pxs, pys, pA.rotation, cA.anchorX, cA.anchorY);
                float txs[4] = {0, cB.p1, cB.p3, 0};
                float tys[4] = {0, cB.p2, cB.p4, 0};
                RotatePoints4(pB.pos.x, pB.pos.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
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
                        RectToCircle(pB.pos.x + cB.offset.x, pB.pos.y + cB.offset.y, cB.p1, cB.p2, pA.pos.x + cA.p1,
                                     pA.pos.y + cA.p1, cA.p1, i);
                        return;
                    }
                    float pxs[4] = {0, cB.p1, cB.p1, 0}; // rect b
                    float pys[4] = {0, 0, cB.p2, cB.p2}; // rect b
                    RotatePoints4(pB.pos.x + cB.offset.x, pB.pos.y + cB.offset.y, pxs, pys, pB.rotation, cB.anchorX,
                                  cB.anchorY);
                    CircleToQuadrilateral(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pxs, pys, i);
                    return;
                }
            case Shape::CIRCLE:
                // We can skip the translation to the middle point as both are in the same system
                CircleToCircle(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, i);
                return;
            case Shape::TRIANGLE:
                {
                    if (pB.rotation == 0)
                    {
                        const float txs[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p3, pB.pos.x};
                        const float tys[4] = {pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p4, pB.pos.y};
                        CircleToQuadrilateral(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, txs, tys, i);
                        return;
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.pos.x, pB.pos.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    return CircleToQuadrilateral(pA.pos.x + cA.p1, pA.pos.y + cA.p1, cA.p1, txs, tys, i);
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
                            const float rectX[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p1, pB.pos.x};
                            const float rectY[4] = {pB.pos.y, pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p2};

                            const float triX[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p3, pA.pos.x};
                            const float triY[4] = {pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p4, pA.pos.y};
                            return SAT(rectX, rectY, triX, triY, i);
                        }
                        float pxs[4] = {0, cB.p1, cB.p1, 0};                                              // rect b
                        float pys[4] = {0, 0, cB.p2, cB.p2};                                              // rect b
                        RotatePoints4(pB.pos.x, pB.pos.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY); // rot
                        const float t1xs[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p3, pA.pos.x};
                        const float t1ys[4] = {pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p4, pA.pos.y};
                        return SAT(pxs, pys, t1xs, t1ys, i);
                    }
                    if (pB.rotation == 0)
                    {
                        float txs[4] = {0, cA.p1, cA.p3, 0};
                        float tys[4] = {0, cA.p2, cA.p4, 0};
                        RotatePoints4(pA.pos.x, pA.pos.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                        const float p1xs[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p1, pB.pos.x}; // Rect b
                        const float p1ys[4] = {pB.pos.y, pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p2}; // Rect b
                        return SAT(txs, tys, p1xs, p1ys, i);
                    }
                    float txs[4] = {0, cA.p1, cA.p3, 0}; // Triangle rotated
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.pos.x, pA.pos.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    float pxs[4] = {0, cB.p1, cB.p1, 0}; // Rect rotated
                    float pys[4] = {0, 0, cB.p2, cB.p2};
                    RotatePoints4(pB.pos.x, pB.pos.y, pxs, pys, pB.rotation, cB.anchorX, cB.anchorY);
                    return SAT(pxs, pys, txs, tys, i);
                }
            case Shape::CIRCLE:
                {
                    if (pA.rotation == 0)
                    {
                        const float txs[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p3, pA.pos.x};
                        const float tys[4] = {pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p4, pA.pos.y};
                        return CircleToQuadrilateral(pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, txs, tys, i);
                    }
                    float txs[4] = {0, cA.p1, cA.p3, 0};
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.pos.x, pA.pos.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    return CircleToQuadrilateral(pB.pos.x + cB.p1, pB.pos.y + cB.p1, cB.p1, txs, tys, i);
                }
            case Shape::TRIANGLE:
                if (pA.rotation == 0) // For triangles, we don't assume as they are likely rotated
                {
                    if (pB.rotation == 0)
                    {
                        const float t1xs[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p3, pA.pos.x};
                        const float t1ys[4] = {pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p4, pA.pos.y};
                        const float t2xs[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p3, pB.pos.x};
                        const float t2ys[4] = {pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p4, pB.pos.y};
                        return SAT(t1xs, t1ys, t2xs, t2ys, i);
                    }
                    float txs[4] = {0, cB.p1, cB.p3, 0};
                    float tys[4] = {0, cB.p2, cB.p4, 0};
                    RotatePoints4(pB.pos.x, pB.pos.y, txs, tys, pB.rotation, cB.anchorX, cB.anchorY);
                    const float t1xs[4] = {pA.pos.x, pA.pos.x + cA.p1, pA.pos.x + cA.p3, pA.pos.x};
                    const float t1ys[4] = {pA.pos.y, pA.pos.y + cA.p2, pA.pos.y + cA.p4, pA.pos.y};
                    return SAT(txs, tys, t1xs, t1ys, i);
                }
                if (pB.rotation == 0)
                {
                    float txs[4] = {0, cA.p1, cA.p3, 0};
                    float tys[4] = {0, cA.p2, cA.p4, 0};
                    RotatePoints4(pA.pos.x, pA.pos.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                    const float t1xs[4] = {pB.pos.x, pB.pos.x + cB.p1, pB.pos.x + cB.p3, pB.pos.x};
                    const float t1ys[4] = {pB.pos.y, pB.pos.y + cB.p2, pB.pos.y + cB.p4, pB.pos.y};
                    return SAT(txs, tys, t1xs, t1ys, i);
                }
                float txs[4] = {0, cA.p1, cA.p3, 0};
                float tys[4] = {0, cA.p2, cA.p4, 0};
                float t1xs[4] = {0, cB.p1, cB.p3, 0};
                float t1ys[4] = {0, cB.p2, cB.p4, 0};
                RotatePoints4(pA.pos.x, pA.pos.y, txs, tys, pA.rotation, cA.anchorX, cA.anchorY);
                RotatePoints4(pB.pos.x, pB.pos.y, t1xs, t1ys, pB.rotation, cB.anchorX, cB.anchorY);
                return SAT(txs, tys, t1xs, t1ys, i);
            }
        }
    }

    void CheckCollisionEntityRect(const PositionC& pos, const CollisionC& col, const Rect& r, CollisionInfo& info)
    {
        switch (col.shape)
        {
        case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return RectToRect(pos.pos.x, pos.pos.y, col.p1, col.p2, r.x, r.y, r.w, r.h, info);
                }
                // Entity
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.pos.x, pos.pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);

                // World rect
                const float p1xs[4] = {r.x, r.x + r.w, r.x + r.w, r.x};
                const float p1ys[4] = {r.y, r.y, r.y + r.h, r.y + r.h};
                return SAT(pxs, pys, p1xs, p1ys, info);
            }
        case Shape::CIRCLE:
            {
                return RectToCircle(r.x, r.y, r.w, r.h, pos.pos.x + col.p1, pos.pos.y + col.p1, col.p1, info);
            }
        case Shape::TRIANGLE:
            const float rectX[4] = {r.x, r.x + r.w, r.x + r.w, r.x}; // World rect
            const float rectY[4] = {r.y, r.y, r.y + r.h, r.y + r.h};
            if (pos.rotation == 0) [[likely]]
            {
                const float triX[4] = {pos.pos.x, pos.pos.x + col.p1, pos.pos.x + col.p3, pos.pos.x};
                const float triY[4] = {pos.pos.y, pos.pos.y + col.p2, pos.pos.y + col.p4, pos.pos.y};
                return SAT(rectX, rectY, triX, triY, info);
            }
            float triX[4] = {0, col.p1, col.p3, 0};
            float triY[4] = {0, col.p2, col.p4, 0};
            RotatePoints4(pos.pos.x, pos.pos.y, triX, triY, pos.rotation, col.anchorX, col.anchorY);
            return SAT(rectX, rectY, triX, triY, info);
        }
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
