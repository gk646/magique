// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COLLISION_PRIMITIVES_H
#define MAGIQUE_COLLISION_PRIMITIVES_H

#include <cmath>
#include <limits>
#include <algorithm>

#include <magique/util/Math.h>

namespace magique
{

#define RECT_TO_POINTS(name, pos, col)                                                                                  \
    const float name##X[4] = {pos.x + col.offset.x, pos.x + col.offset.x + col.p1, pos.x + col.offset.x + col.p1,       \
                              pos.x + col.offset.x};                                                                    \
    const float name##Y[4] = {pos.y + col.offset.y, pos.y + col.offset.y, pos.y + col.p2 + col.offset.y,                \
                              pos.y + col.p2 + col.offset.y}

#define RECT_ROTATE_POINTS(name, posc, col)                                                                             \
    float name##X[4] = {0, col.p1, col.p1, 0};                                                                          \
    float name##Y[4] = {0, 0, col.p2, col.p2};                                                                          \
    RotatePoints4(posc.pos.x + col.offset.x, posc.pos.y + col.offset.y, name##X, name##Y, posc.rotation, col.anchor.x,  \
                  col.anchor.y);

#define TRI_TO_POINTS(name, pos, col)                                                                                   \
    const float name##X[4] = {pos.x, pos.x + col.p1, pos.x + col.p3, pos.x};                                            \
    const float name##Y[4] = {pos.y, pos.y + col.p2, pos.y + col.p4, pos.y};

#define TRI_ROTATE_POINTS(name, posc, col)                                                                              \
    float name##X[4] = {0, col.p1, col.p3, 0};                                                                          \
    float name##Y[4] = {0, col.p2, col.p4, 0};                                                                          \
    RotatePoints4(posc.pos.x, posc.pos.y, name##X, name##Y, posc.rotation, col.anchor.x, col.anchor.y);


    //----------------- POINT -----------------//

    inline bool PointToRect(const float px, const float py, const float rx, const float ry, const float rw,
                            const float rh)
    {
        bool ret = true;
        ret &= px >= rx;
        ret &= px <= rx + rw;
        ret &= py >= ry;
        ret &= py <= ry + rh;
        return ret;
    }

    //----------------- BOUNDING BOX -----------------//

    // returns the bounding box of a triangle given by the 3 points
    inline Rect GetBBTriangle(const float x, const float y, const float x2, const float y2, const float x3,
                              const float y3)
    {
        const float minX = std::min({x, x2, x3});
        const float minY = std::min({y, y2, y3});
        const float maxX = std::max({x, x2, x3});
        const float maxY = std::max({y, y2, y3});

        const float width = maxX - minX;
        const float height = maxY - minY;
        return {minX, minY, width, height};
    }

    // Given the 4x and 4y coordinates of a shape returns the bounding rectangle
    inline Rect GetBBQuadrilateral(const float (&pxs)[4], const float (&pys)[4])
    {
        float minX = pxs[0];
        if (pxs[1] < minX)
            minX = pxs[1];
        if (pxs[2] < minX)
            minX = pxs[2];
        if (pxs[3] < minX)
            minX = pxs[3];

        float minY = pys[0];
        if (pys[1] < minY)
            minY = pys[1];
        if (pys[2] < minY)
            minY = pys[2];
        if (pys[3] < minY)
            minY = pys[3];

        float maxX = pxs[0];
        if (pxs[1] > maxX)
            maxX = pxs[1];
        if (pxs[2] > maxX)
            maxX = pxs[2];
        if (pxs[3] > maxX)
            maxX = pxs[3];

        float maxY = pys[0];
        if (pys[1] > maxY)
            maxY = pys[1];
        if (pys[2] > maxY)
            maxY = pys[2];
        if (pys[3] > maxY)
            maxY = pys[3];

        const float width = maxX - minX;
        const float height = maxY - minY;
        return {minX, minY, width, height};
    }

    //----------------- RECT -----------------//

    // rect: x,y,width,height / rect: x,y,width,height
    inline bool RectToRect(const float x1, const float y1, const float w1, const float h1, const float x2,
                           const float y2, const float w2, const float h2)
    {
        bool ret = true;
        ret &= x1 < x2 + w2;
        ret &= x1 + w1 > x2;
        ret &= y1 < y2 + h2;
        ret &= y1 + h1 > y2;
        return ret;
    }

    inline void RectToRect(const float x1, const float y1, const float w1, const float h1, const float x2,
                           const float y2, const float w2, const float h2, CollisionInfo& info)
    {
        if (!RectToRect(x1, y1, w1, h1, x2, y2, w2, h2))
        {
            return;
        }
        const float overlapX = std::min(x1 + w1, x2 + w2) - std::max(x1, x2);
        const float overlapY = std::min(y1 + h1, y2 + h2) - std::max(y1, y2);

        if (overlapX < overlapY)
        {
            info.normalVector.x = x1 + w1 / 2 < x2 + w2 / 2 ? -1.0f : 1.0f;
            info.normalVector.y = 0.0f;
            info.penDepth = overlapX;
        }
        else
        {
            info.normalVector.x = 0.0f;
            info.normalVector.y = y1 + h1 / 2 < y2 + h2 / 2 ? -1.0f : 1.0f;
            info.penDepth = overlapY;
        }

        info.collisionPoint.x = (std::max(x1, x2) + std::min(x1 + w1, x2 + w2)) / 2.0f;
        info.collisionPoint.y = (std::max(y1, y2) + std::min(y1 + h1, y2 + h2)) / 2.0f;
    }

    inline void RectToCircle(const float rx, const float ry, const float rw, const float rh, const float cx,
                             const float cy, const float cr, CollisionInfo& info)
    {
        const float radius_squared = cr * cr;
        const float closestX = cx < rx ? rx : cx > rx + rw ? rx + rw : cx;
        const float closestY = cy < ry ? ry : cy > ry + rh ? ry + rh : cy;
        const float dx = cx - closestX;
        const float dy = cy - closestY;
        const float distance_squared = dx * dx + dy * dy;
        if (distance_squared > radius_squared) [[likely]]
        {
            return;
        }
        const float distance = std::sqrt(distance_squared);
        if (distance != 0.0f) [[likely]]
        {
            info.normalVector.x = dx / distance;
            info.normalVector.y = dy / distance;
        }
        else
        {
            info.normalVector.x = 0.0f;
            info.normalVector.y = 0.0f;
        }
        info.penDepth = cr - distance;
        info.collisionPoint.x = closestX;
        info.collisionPoint.y = closestY;
    }

    inline void CircleToRect(const float cx, const float cy, const float cr, const float rx, const float ry,
                             const float rw, const float rh, CollisionInfo& info)
    {
        RectToCircle(rx, ry, rw, rh, cx, cy, cr, info);
        info.normalVector = -info.normalVector;
    }

    // --------- SAT -----------------//

    // checks 4 points against another 4 points
    inline void SAT(const float (&pxs)[4], const float (&pys)[4], const float (&p1xs)[4], const float (&p1ys)[4],
                    CollisionInfo& info)
    {
        const auto [x, y, w, h] = GetBBQuadrilateral(pxs, pys);
        const auto [x1, y1, w1, h1] = GetBBQuadrilateral(p1xs, p1ys);
        if (!RectToRect(x, y, w, h, x1, y1, w1, h1)) [[likely]]
        {
            return; // Early bound check
        }
        float minPenetration = std::numeric_limits<float>::max();
        Point bestAxis{};
        const auto OverlapOnAxis = [](const float (&pxs)[4], const float (&pys)[4], const float (&p1xs)[4],
                                      const float (&p1ys)[4], float& axisX, float& axisY, float& penetration) -> bool
        {
            float projA[4];
            projA[0] = pxs[0] * axisX + pys[0] * axisY;
            projA[1] = pxs[1] * axisX + pys[1] * axisY;
            projA[2] = pxs[2] * axisX + pys[2] * axisY;
            projA[3] = pxs[3] * axisX + pys[3] * axisY;

            float minA = std::min(std::min(std::min(projA[0], projA[1]), projA[2]), projA[3]);
            float maxA = std::max(std::max(std::max(projA[0], projA[1]), projA[2]), projA[3]);

            // Project points of the second shape onto the axis
            float projB[4];
            projB[0] = p1xs[0] * axisX + p1ys[0] * axisY;
            projB[1] = p1xs[1] * axisX + p1ys[1] * axisY;
            projB[2] = p1xs[2] * axisX + p1ys[2] * axisY;
            projB[3] = p1xs[3] * axisX + p1ys[3] * axisY;

            float minB = std::min(std::min(std::min(projB[0], projB[1]), projB[2]), projB[3]);
            float maxB = std::max(std::max(std::max(projB[0], projB[1]), projB[2]), projB[3]);

            if (maxA >= minB && maxB >= minA)
            {
                const float overlap1 = maxA - minB;
                const float overlap2 = maxB - minA;

                if (overlap1 < overlap2)
                {
                    penetration = overlap1;
                }
                else
                {
                    penetration = overlap2;
                    axisX = -axisX;
                    axisY = -axisY;
                }
                return true;
            }
            return false;
        };

        const auto processEdge = [&](const float x1, const float y1, const float x2, const float y2)
        {
            const float edgeX = x2 - x1;
            const float edgeY = y2 - y1;
            float axisX = -edgeY;
            float axisY = edgeX;
            float penetration;

            float axisLength = axisX * axisX + axisY * axisY;
            if (axisLength == 0.0f)
            {
                return true; // Skip this axis, continue with others
            }
            axisLength = std::sqrt(axisLength);
            axisX /= axisLength;
            axisY /= axisLength;

            if (!OverlapOnAxis(pxs, pys, p1xs, p1ys, axisX, axisY, penetration))
                return false;

            if (penetration < minPenetration)
            {
                minPenetration = penetration;
                bestAxis = Point{axisX, axisY};
            }
            return true;
        };

        bool validAxisFound = true;
        validAxisFound &= processEdge(pxs[0], pys[0], pxs[1], pys[1]);
        validAxisFound &= processEdge(pxs[1], pys[1], pxs[2], pys[2]);
        validAxisFound &= processEdge(pxs[2], pys[2], pxs[3], pys[3]);
        validAxisFound &= processEdge(pxs[3], pys[3], pxs[0], pys[0]);
        validAxisFound &= processEdge(p1xs[0], p1ys[0], p1xs[1], p1ys[1]);
        validAxisFound &= processEdge(p1xs[1], p1ys[1], p1xs[2], p1ys[2]);
        validAxisFound &= processEdge(p1xs[2], p1ys[2], p1xs[3], p1ys[3]);
        validAxisFound &= processEdge(p1xs[3], p1ys[3], p1xs[0], p1ys[0]);

        if (!validAxisFound)
        {
            return; // No collision detected
        }

        info.normalVector = -bestAxis; // already normalized
        info.penDepth = minPenetration;
        info.collisionPoint = {(pxs[0] + pxs[1] + pxs[2] + pxs[3]) / 4.0f, (pys[0] + pys[1] + pys[2] + pys[3]) / 4.0f};
    }

    //----------------- CIRCLE -----------------//

    // circle: x,y, radius / circle: x,y, radius
    inline void CircleToCircle(const float x1, const float y1, const float r1, const float x2, const float y2,
                               const float r2, CollisionInfo& info)
    {
        const float radiiSum = r1 + r2;

        const float dx = x2 - x1;
        const float dy = y2 - y1;
        float distance_squared = dx * dx + dy * dy;
        if (distance_squared > radiiSum * radiiSum)
        {
            return;
        }
        float distance = std::sqrt(distance_squared);
        info.penDepth = radiiSum - distance;
        info.normalVector = {(x1 - x2) / distance, (y1 - y2) / distance};
        info.collisionPoint = {x1 + info.normalVector.x * r1, y1 + info.normalVector.y * r1};
    }

    inline void CircleToQuadrilateral(const float cx, const float cy, const float cr, const float (&pxs)[4],
                                      const float (&pys)[4], CollisionInfo& info)
    {
        const auto [x, y, width, height] = GetBBQuadrilateral(pxs, pys);
        if (!RectToRect(cx - cr, cy - cr, cr * 2.0F, cr * 2.0F, x, y, width, height)) [[likely]]
        {
            return; // quick bound check
        }

        const auto getClosestPointOnLine = [](const Point& p, const Point& q, const Point& mid) -> Point
        {
            const auto lineVec = q - p;
            const auto pToMid = mid - p;
            const auto dotLine = lineVec.dot(pToMid);
            const auto dotMid = lineVec.dot(lineVec);
            const float t = std::clamp(dotLine / dotMid, 0.0F, 1.0F);
            return p + (lineVec * t);
        };

        const Point mid = {cx, cy};
        const auto radiusSq = cr * cr;

        const Point* closest = nullptr;
        const auto closest1 = getClosestPointOnLine({pxs[0], pys[0]}, {pxs[1], pys[1]}, mid);
        const auto closest2 = getClosestPointOnLine({pxs[1], pys[1]}, {pxs[2], pys[2]}, mid);
        const auto closest3 = getClosestPointOnLine({pxs[2], pys[2]}, {pxs[3], pys[3]}, mid);
        const auto closest4 = getClosestPointOnLine({pxs[3], pys[3]}, {pxs[0], pys[0]}, mid);

        const auto dist1 = closest1.euclideanSqr(mid);
        const auto dist2 = closest2.euclideanSqr(mid);
        const auto dist3 = closest3.euclideanSqr(mid);
        const auto dist4 = closest4.euclideanSqr(mid);

        float minDist = dist1;
        closest = &closest1;

        if (dist2 < minDist)
        {
            minDist = dist2;
            closest = &closest2;
        }
        if (dist3 < minDist)
        {
            minDist = dist3;
            closest = &closest3;
        }
        if (dist4 < minDist)
        {
            minDist = dist4;
            closest = &closest4;
        }

        if (minDist <= radiusSq)
        {
            const float distance = std::sqrt(minDist);
            info.penDepth = cr - distance;
            info.normalVector = closest->dir(mid);
            info.collisionPoint = *closest;
        }
    }

    inline void QuadrilateralToCircle(const float (&pxs)[4], const float (&pys)[4], const float cx, const float cy,
                                      const float cr, CollisionInfo& info)
    {
        // Inverted
        CircleToQuadrilateral(cx, cy, cr, pxs, pys, info);
        info.normalVector = -info.normalVector;
    }


    //----------------- ROTATION -----------------//

    // Takes translation x and y / point coordinates relative to translation / rotation clockwise in degrees from the top / anchor is relative to the points
    // Translates and rotates the points!
    inline void RotatePoints4(float x, float y, float (&pxs)[4], float (&pys)[4], const float rotation,
                              const float anchorX, const float anchorY)
    {
        const float cosTheta = cosf(rotation * DEG2RAD);
        const float sinTheta = sinf(rotation * DEG2RAD);

        // Rotate point 0
        const float localX0 = pxs[0] - anchorX;
        const float localY0 = pys[0] - anchorY;
        const float rotatedX0 = localX0 * cosTheta - localY0 * sinTheta;
        const float rotatedY0 = localX0 * sinTheta + localY0 * cosTheta;
        pxs[0] = x + rotatedX0 + anchorX;
        pys[0] = y + rotatedY0 + anchorY;

        // Rotate point 1
        const float localX1 = pxs[1] - anchorX;
        const float localY1 = pys[1] - anchorY;
        const float rotatedX1 = localX1 * cosTheta - localY1 * sinTheta;
        const float rotatedY1 = localX1 * sinTheta + localY1 * cosTheta;
        pxs[1] = x + rotatedX1 + anchorX;
        pys[1] = y + rotatedY1 + anchorY;

        // Rotate point 2
        const float localX2 = pxs[2] - anchorX;
        const float localY2 = pys[2] - anchorY;
        const float rotatedX2 = localX2 * cosTheta - localY2 * sinTheta;
        const float rotatedY2 = localX2 * sinTheta + localY2 * cosTheta;
        pxs[2] = x + rotatedX2 + anchorX;
        pys[2] = y + rotatedY2 + anchorY;

        // Rotate point 3
        const float localX3 = pxs[3] - anchorX;
        const float localY3 = pys[3] - anchorY;
        const float rotatedX3 = localX3 * cosTheta - localY3 * sinTheta;
        const float rotatedY3 = localX3 * sinTheta + localY3 * cosTheta;
        pxs[3] = x + rotatedX3 + anchorX;
        pys[3] = y + rotatedY3 + anchorY;
    }
} // namespace magique

#endif // MAGIQUE_COLLISION_PRIMITIVES_H
