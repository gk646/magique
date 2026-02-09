// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COLLISION_PRIMITIVES_H
#define MAGIQUE_COLLISION_PRIMITIVES_H



#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define MAGIQUE_SIMD 1
#else
#undef MAGIQUE_SIMD
#define MAGIQUE_SIMD 0
#endif
#if MAGIQUE_SIMD == 1
#include <immintrin.h>
#endif

#include <cmath>
#include <limits>
#include <algorithm>
#include <magique/util/Math.h>

//-----------------------------------------------
// Collision Primitives with up to AVX2 intrinsics
//-----------------------------------------------
// .....................................................................
// In pure benchmark scenarios they are equal, but in game stress tests simd is much faster
// The SIMD should be quite primitive and can probably be optimized!
// Uses out params for CollisionInfo to guarantee optimized handling (and its likely always bit faster)
// .....................................................................

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


    inline void SquareRoot(float& pIn)
    {
#if MAGIQUE_SIMD == 1 // rsqrt is quite a bit faster
        __m128 in = _mm_load_ss(&pIn);
        in = _mm_mul_ss(in, _mm_rsqrt_ss(in));
        _mm_store_ss(&pIn, in);
#else
        pIn = std::sqrt(pIn);
#endif
    }

    //----------------- POINT -----------------//

    inline bool PointToRect(const float px, const float py, const float rx, const float ry, const float rw,
                            const float rh)
    {
#if MAGIQUE_SIMD == 1
        const __m256 point = _mm256_set_ps(py, py, py, py, px, px, px, px);
        const __m256 rect_min = _mm256_set_ps(ry, ry, ry, ry, rx, rx, rx, rx);
        const __m256 rect_max = _mm256_set_ps(ry + rh, ry + rh, ry + rh, ry + rh, rx + rw, rx + rw, rx + rw, rx + rw);
        const __m256 cmp_min = _mm256_cmp_ps(point, rect_min, _CMP_GE_OQ);
        const __m256 cmp_max = _mm256_cmp_ps(point, rect_max, _CMP_LE_OQ);
        const __m256 result = _mm256_and_ps(cmp_min, cmp_max);
        return (_mm256_movemask_ps(result) & 0x33) == 0x33;
#else
        return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
#endif
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
        // This looks a bit faster in under mass load
#if MAGIQUE_SIMD == 1
        const __m256 rect1 = _mm256_set_ps(y1 + h1, y1, x1 + w1, x1, y1 + h1, y1, x1 + w1, x1);
        const __m256 rect2 = _mm256_set_ps(y2, y2 + h2, x2, x2 + w2, y2, y2 + h2, x2, x2 + w2);
        const __m256 cmp_lt = _mm256_cmp_ps(rect1, rect2, _CMP_LT_OQ);
        const __m256 cmp_gt = _mm256_cmp_ps(rect1, rect2, _CMP_GT_OQ);
        const __m256 x_intersect = _mm256_and_ps(_mm256_permute_ps(cmp_gt, 0x50), _mm256_permute_ps(cmp_lt, 0xA0));
        const __m256 y_intersect = _mm256_and_ps(_mm256_permute_ps(cmp_gt, 0xFA), _mm256_permute_ps(cmp_lt, 0x0F));
        const __m256 result = _mm256_and_ps(x_intersect, y_intersect);
        return (_mm256_movemask_ps(result) & 0x88) == 0x88;
#else
        return !(x1 >= x2 + w2 || x1 + w1 <= x2 || y1 >= y2 + h2 || y1 + h1 <= y2);
#endif
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
        float closestX, closestY, dx, dy, distance_squared;
        const float radius_squared = cr * cr;
#if MAGIQUE_SIMD == 1
        const __m128 circle_center = _mm_set_ps(0.0f, 0.0f, cy, cx);
        const __m128 rect_min = _mm_set_ps(0.0f, 0.0f, ry, rx);
        const __m128 rect_max = _mm_set_ps(0.0f, 0.0f, ry + rh, rx + rw);
        const __m128 clamped = _mm_min_ps(_mm_max_ps(circle_center, rect_min), rect_max);
        const __m128 diff = _mm_sub_ps(circle_center, clamped);
        const __m128 diff_squared = _mm_mul_ps(diff, diff);
        const __m128 sum = _mm_add_ps(diff_squared, _mm_shuffle_ps(diff_squared, diff_squared, _MM_SHUFFLE(2, 3, 0, 1)));
        distance_squared = _mm_cvtss_f32(sum);
        _mm_store_ss(&closestX, _mm_shuffle_ps(clamped, clamped, _MM_SHUFFLE(0, 0, 0, 0)));
        _mm_store_ss(&closestY, _mm_shuffle_ps(clamped, clamped, _MM_SHUFFLE(1, 1, 1, 1)));
        dx = cx - closestX;
        dy = cy - closestY;
#else
        closestX = cx < rx ? rx : cx > rx + rw ? rx + rw : cx;
        closestY = cy < ry ? ry : cy > ry + rh ? ry + rh : cy;
        dx = cx - closestX;
        dy = cy - closestY;
        distance_squared = dx * dx + dy * dy;
#endif
        if (distance_squared > radius_squared) [[likely]]
        {
            return;
        }
        float distance = distance_squared;
        SquareRoot(distance);
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
#if MAGIQUE_SIMD == 1
            const auto project = [](const float (&pxs)[4], const float (&pys)[4], const float axisX, const float axisY,
                                    float& min, float& max)
            {
                __m128 axisX_vec = _mm_set1_ps(axisX);
                __m128 axisY_vec = _mm_set1_ps(axisY);
                __m128 pxs_vec = _mm_loadu_ps(pxs);
                __m128 pys_vec = _mm_loadu_ps(pys);
                __m128 proj_vec = _mm_add_ps(_mm_mul_ps(pxs_vec, axisX_vec), _mm_mul_ps(pys_vec, axisY_vec));
                __m128 shuffled1 = _mm_shuffle_ps(proj_vec, proj_vec, _MM_SHUFFLE(2, 3, 0, 1));
                __m128 min_vec = _mm_min_ps(proj_vec, shuffled1);
                __m128 max_vec = _mm_max_ps(proj_vec, shuffled1);
                __m128 shuffled2 = _mm_shuffle_ps(min_vec, min_vec, _MM_SHUFFLE(1, 0, 3, 2));
                min_vec = _mm_min_ps(min_vec, shuffled2);
                shuffled2 = _mm_shuffle_ps(max_vec, max_vec, _MM_SHUFFLE(1, 0, 3, 2));
                max_vec = _mm_max_ps(max_vec, shuffled2);
                min = _mm_cvtss_f32(min_vec);
                max = _mm_cvtss_f32(max_vec);
            };
            float minA, maxA, minB, maxB;
            project(pxs, pys, axisX, axisY, minA, maxA);
            project(p1xs, p1ys, axisX, axisY, minB, maxB);
#else
            float minA = pxs[0] * axisX + pys[0] * axisY;
            float maxA = minA;
            float projection = pxs[1] * axisX + pys[1] * axisY;
            if (projection < minA)
                minA = projection;
            if (projection > maxA)
                maxA = projection;

            projection = pxs[2] * axisX + pys[2] * axisY;
            if (projection < minA)
                minA = projection;
            if (projection > maxA)
                maxA = projection;

            projection = pxs[3] * axisX + pys[3] * axisY;
            if (projection < minA)
                minA = projection;
            if (projection > maxA)
                maxA = projection;

            float minB = p1xs[0] * axisX + p1ys[0] * axisY;
            float maxB = minB;

            projection = p1xs[1] * axisX + p1ys[1] * axisY;
            if (projection < minB)
                minB = projection;
            if (projection > maxB)
                maxB = projection;

            projection = p1xs[2] * axisX + p1ys[2] * axisY;
            if (projection < minB)
                minB = projection;
            if (projection > maxB)
                maxB = projection;

            projection = p1xs[3] * axisX + p1ys[3] * axisY;
            if (projection < minB)
                minB = projection;
            if (projection > maxB)
                maxB = projection;
#endif
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
            SquareRoot(axisLength); // Might be able to be delayed
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
        // manually unrolled loops
        if (!processEdge(pxs[0], pys[0], pxs[1], pys[1]) || !processEdge(pxs[1], pys[1], pxs[2], pys[2]) ||
            !processEdge(pxs[2], pys[2], pxs[3], pys[3]) || !processEdge(pxs[3], pys[3], pxs[0], pys[0]) ||
            !processEdge(p1xs[0], p1ys[0], p1xs[1], p1ys[1]) || !processEdge(p1xs[1], p1ys[1], p1xs[2], p1ys[2]) ||
            !processEdge(p1xs[2], p1ys[2], p1xs[3], p1ys[3]) || !processEdge(p1xs[3], p1ys[3], p1xs[0], p1ys[0]))
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
#if MAGIQUE_SIMD == 1
        const __m128 center1 = _mm_set_ps(0, 0, y1, x1);
        const __m128 center2 = _mm_set_ps(0, 0, y2, x2);
        const __m128 diff = _mm_sub_ps(center1, center2);
        const __m128 diff_sq = _mm_mul_ps(diff, diff);
        const __m128 dist_sq = _mm_hadd_ps(diff_sq, diff_sq);
        const __m128 dist_sq_sum = _mm_hadd_ps(dist_sq, dist_sq);
        const float radii_sq_sum = radiiSum * radiiSum;
        const __m128 radii_sq_vec = _mm_set1_ps(radii_sq_sum);
        const __m128 cmp = _mm_cmple_ps(dist_sq_sum, radii_sq_vec);
        if ((_mm_movemask_ps(cmp) & 0x1) == 0) [[likely]] // more likely no collision
        {
            return;
        }
        const float distance_squared = _mm_cvtss_f32(dist_sq_sum);
#else
        const float dx = x2 - x1;
        const float dy = y2 - y1;
        float distance_squared = dx * dx + dy * dy;
        if (distance_squared > radiiSum * radiiSum)
        {
            return;
        }
#endif
        float distance = distance_squared;
        SquareRoot(distance);
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
            float distance = minDist;
            SquareRoot(distance);
            info.penDepth = cr - distance;
            info.normalVector = GetDirFromPoints(*closest, mid);
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
#if MAGIQUE_SIMD == 1
        const float cosTheta = cosf(rotation * DEG2RAD);
        const float sinTheta = sinf(rotation * DEG2RAD);

        const __m128 cosVec = _mm_set1_ps(cosTheta);
        const __m128 sinVec = _mm_set1_ps(sinTheta);

        const __m128 anchorXVec = _mm_set1_ps(anchorX);
        const __m128 anchorYVec = _mm_set1_ps(anchorY);

        const __m128 px = _mm_set_ps(pxs[3], pxs[2], pxs[1], pxs[0]);
        const __m128 py = _mm_set_ps(pys[3], pys[2], pys[1], pys[0]);

        const __m128 tx = _mm_sub_ps(px, anchorXVec);
        const __m128 ty = _mm_sub_ps(py, anchorYVec);

        __m128 rotatedX = _mm_sub_ps(_mm_mul_ps(tx, cosVec), _mm_mul_ps(ty, sinVec));
        __m128 rotatedY = _mm_add_ps(_mm_mul_ps(tx, sinVec), _mm_mul_ps(ty, cosVec));

        __m128 const xVec = _mm_set1_ps(x);
        __m128 const yVec = _mm_set1_ps(y);
        rotatedX = _mm_add_ps(_mm_add_ps(rotatedX, anchorXVec), xVec);
        rotatedY = _mm_add_ps(_mm_add_ps(rotatedY, anchorYVec), yVec);

        _mm_store_ps(pxs, rotatedX);
        _mm_store_ps(pys, rotatedY);
#else
        const float cosTheta = cosf(rotation * DEG2RAD);
        const float sinTheta = sinf(rotation * DEG2RAD);

        for (int i = 0; i < 4; ++i)
        {
            const float localX = pxs[i] - anchorX;
            const float localY = pys[i] - anchorY;

            // Apply rotation
            const float rotatedX = localX * cosTheta - localY * sinTheta;
            const float rotatedY = localX * sinTheta + localY * cosTheta;

            // Translate back and offset to world position
            pxs[i] = x + rotatedX + anchorX;
            pys[i] = y + rotatedY + anchorY;
        }
#endif
    }

} // namespace magique

#endif // MAGIQUE_COLLISION_PRIMITIVES_H
