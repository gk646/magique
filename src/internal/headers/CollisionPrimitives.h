#ifndef MAGIQUE_COLLISIONDETECTION_H
#define MAGIQUE_COLLISIONDETECTION_H

#if MAGIQUE_SIMD == 1
#include <immintrin.h>
#include <emmintrin.h>
#endif

#ifdef __MINGW32__
#include <cmath>
#endif

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

    inline bool PointInRect(const float px, const float py, const float rx, const float ry, const float rw,
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
    inline Rectangle GetBBTriangle(const float x, const float y, const float x2, const float y2, const float x3,
                                   const float y3)
    {
        float minX = x;
        if (x2 < minX)
            minX = x2;
        if (x3 < minX)
            minX = x3;

        float minY = y;
        if (y2 < minY)
            minY = y2;
        if (y3 < minY)
            minY = y3;

        float maxX = x;
        if (x2 > maxX)
            maxX = x2;
        if (x3 > maxX)
            maxX = x3;

        float maxY = y;
        if (y2 > maxY)
            maxY = y2;
        if (y3 > maxY)
            maxY = y3;

        const float width = maxX - minX;
        const float height = maxY - minY;
        return {minX, minY, width, height};
    }

    // Given the 4x and 4y coordinates of a shape returns the bounding rectangle
    inline Rectangle GetBBQuadrilateral(const float (&pxs)[4], const float (&pys)[4])
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
            info.normalVector.x = (x1 + w1 / 2 < x2 + w2 / 2) ? -1.0f : 1.0f;
            info.normalVector.y = 0.0f;
            info.penDepth = overlapX;
        }
        else
        {
            info.normalVector.x = 0.0f;
            info.normalVector.y = (y1 + h1 / 2 < y2 + h2 / 2) ? -1.0f : 1.0f;
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
        if (distance != 0.0f)
        {
            info.normalVector.x = -dx / distance;
            info.normalVector.y = -dy / distance;
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

    // Top left of the capsule and its height and radius
    inline void RectToCapsule(const float x1, const float y1, const float w1, const float h1, const float x2,
                              const float y2, const float radius, const float height, CollisionInfo& info)
    {
        // Fast bounding box check using RectToRect
        if (!RectToRect(x1, y1, w1, h1, x2, y2, 2.0F * radius, height)) [[likely]]
        {
            return;
        }

        RectToCircle(x1, y1, w1, h1, x2 + radius, y2 + radius, radius, info);
        if (info.isColliding())
            return;

        RectToCircle(x1, y1, w1, h1, x2 + radius, y2 + height - radius, radius, info);
        if (info.isColliding())
            return;

        RectToRect(x1, y1, w1, h1, x2, y2 + radius, radius * 2, height - radius, info);
    }

    //----------------- SAT -----------------//

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
        float minPenetration = 222222222.0F;
        Point bestAxis;
#if MAGIQUE_SIMD == 1 // TODO can be optimized a lot by using AVX2
        const auto OverlapOnAxis = [](const float(&pxs)[4], const float(&pys)[4], const float(&p1xs)[4],
                                      const float(&p1ys)[4], float& axisX, float& axisY, float& penetration)
        {
            const auto project = [](const float(&pxs)[4], const float(&pys)[4], const float axisX, const float axisY,
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
        const auto OverlapOnAxis = [](const float(&pxs)[4], const float(&pys)[4], const float(&p1xs)[4],
                                      const float(&p1ys)[4], float& axisX, float& axisY, float& penetration) -> bool
        {
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
                bestAxis = {axisX, axisY};
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
        info.normalVector = bestAxis; // already normalized
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
        float distance_squared = _mm_cvtss_f32(dist_sq_sum);
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
#if MAGIQUE_SIMD == 1
        const __m128 cx_vec = _mm_set1_ps(cx);
        const __m128 cy_vec = _mm_set1_ps(cy);
        const __m128 cr_sq_vec = _mm_set1_ps(cr * cr);
        for (int i = 0; i < 4; ++i)
        {
            int next = i + 1;
            if (next == 4)
                next = 0;
            const __m128 px_vec = _mm_set_ps(pxs[i], pxs[i], pxs[next], pxs[next]);
            const __m128 py_vec = _mm_set_ps(pys[i], pys[i], pys[next], pys[next]);
            const __m128 pqx_vec = _mm_sub_ps(_mm_shuffle_ps(px_vec, px_vec, _MM_SHUFFLE(1, 0, 3, 2)), px_vec);
            const __m128 pqy_vec = _mm_sub_ps(_mm_shuffle_ps(py_vec, py_vec, _MM_SHUFFLE(1, 0, 3, 2)), py_vec);
            const __m128 pcx_vec = _mm_sub_ps(cx_vec, px_vec);
            const __m128 pcy_vec = _mm_sub_ps(cy_vec, py_vec);
            const __m128 pq_dot_pc_vec = _mm_add_ps(_mm_mul_ps(pqx_vec, pcx_vec), _mm_mul_ps(pqy_vec, pcy_vec));
            const __m128 pq_dot_pq_vec = _mm_add_ps(_mm_mul_ps(pqx_vec, pqx_vec), _mm_mul_ps(pqy_vec, pqy_vec));
            __m128 t_vec = _mm_div_ps(pq_dot_pc_vec, pq_dot_pq_vec);
            t_vec = _mm_max_ps(_mm_setzero_ps(), _mm_min_ps(_mm_set1_ps(1.0f), t_vec));
            const __m128 closestX_vec = _mm_add_ps(px_vec, _mm_mul_ps(t_vec, pqx_vec));
            const __m128 closestY_vec = _mm_add_ps(py_vec, _mm_mul_ps(t_vec, pqy_vec));
            const __m128 dx_vec = _mm_sub_ps(closestX_vec, cx_vec);
            const __m128 dy_vec = _mm_sub_ps(closestY_vec, cy_vec);
            const __m128 dist_sq_vec = _mm_add_ps(_mm_mul_ps(dx_vec, dx_vec), _mm_mul_ps(dy_vec, dy_vec));
            if (_mm_movemask_ps(_mm_cmple_ps(dist_sq_vec, cr_sq_vec)))
            {
                float closestX, closestY, dist_sq;
                _mm_store_ss(&closestX, closestX_vec);
                _mm_store_ss(&closestY, closestY_vec);
                _mm_store_ss(&dist_sq, dist_sq_vec);
                float distance = dist_sq;
                SquareRoot(distance);
                info.penDepth = cr - distance;
                info.normalVector.x = (cx - closestX) / distance;
                info.normalVector.y = (cy - closestY) / distance;
                info.collisionPoint.x = closestX;
                info.collisionPoint.y = closestY;
                return;
            }
        }
        int windingNumber = 0;
        for (int i = 0; i < 4; ++i)
        {
            int next = i + 1;
            if (next == 4)
                next = 0;

            if (pys[i] <= cy)
            {
                if (pys[next] > cy && (pxs[next] - pxs[i]) * (cy - pys[i]) - (cx - pxs[i]) * (pys[next] - pys[i]) > 0)
                    ++windingNumber;
            }
            else
            {
                if (pys[next] <= cy && (pxs[next] - pxs[i]) * (cy - pys[i]) - (cx - pxs[i]) * (pys[next] - pys[i]) < 0)
                    --windingNumber;
            }
        }
        if (windingNumber != 0) // Circle center is inside the quadrilateral
        {
            float minDist = std::numeric_limits<float>::max();
            float closestX = 0.0f, closestY = 0.0f;

            for (int i = 0; i < 4; ++i)
            {
                int next = (i + 1) % 4;

                const float dx = pxs[next] - pxs[i];
                const float dy = pys[next] - pys[i];
                float edgeLength = dx * dx + dy * dy;
                SquareRoot(edgeLength);

                float t = ((cx - pxs[i]) * dx + (cy - pys[i]) * dy) / (edgeLength * edgeLength);
                t = std::max(0.0f, std::min(1.0f, t));

                const float closestPx = pxs[i] + t * dx;
                const float closestPy = pys[i] + t * dy;
                const float distSquared = (closestPx - cx) * (closestPx - cx) + (closestPy - cy) * (closestPy - cy);
                if (distSquared < minDist)
                {
                    minDist = distSquared;
                    closestX = closestPx;
                    closestY = closestPy;
                }
            }
            float distance = minDist;
            SquareRoot(distance);
            info.penDepth = cr - distance;
            info.normalVector.x = (cx - closestX) / distance;
            info.normalVector.y = (cy - closestY) / distance;
            info.collisionPoint.x = closestX;
            info.collisionPoint.y = closestY;
        }
#else
        int windingNumber = 0;
        for (int i = 0; i < 4; ++i)
        {
            int next = (i + 1) % 4;
            if (pys[i] <= cy)
            {
                if (pys[next] > cy && (pxs[next] - pxs[i]) * (cy - pys[i]) - (cx - pxs[i]) * (pys[next] - pys[i]) > 0)
                    ++windingNumber;
            }
            else
            {
                if (pys[next] <= cy && (pxs[next] - pxs[i]) * (cy - pys[i]) - (cx - pxs[i]) * (pys[next] - pys[i]) < 0)
                    --windingNumber;
            }
        }
        if (windingNumber != 0)
        {
            info.depth = cr;
            info.normalVector.x = 0.0f;
            info.normalVector.y = 0.0f;
            info.collisionPoint.x = cx;
            info.collisionPoint.y = cy;
            return;
        }

        const auto DistanceSquaredPointToSegment =
            [](const float px, const float py, const float qx, const float qy, const float cx, const float cy)
        {
            const float pqx = qx - px;
            const float pqy = qy - py;
            const float pcx = cx - px;
            const float pcy = cy - py;

            const float pq_dot_pc = pqx * pcx + pqy * pcy;
            const float pq_dot_pq = pqx * pqx + pqy * pqy;
            float t = pq_dot_pc / pq_dot_pq;
            t = (t < 0) ? 0 : (t > 1) ? 1 : t;

            const float closestX = px + t * pqx;
            const float closestY = py + t * pqy;

            const float dx = closestX - cx;
            const float dy = closestY - cy;
            return dx * dx + dy * dy;
        };

        for (int i = 0; i < 4; ++i)
        {
            int next = (i + 1) % 4;
            const float distSq = DistanceSquaredPointToSegment(pxs[i], pys[i], pxs[next], pys[next], cx, cy);
            if (distSq <= cr * cr)
            {
                float distance = distSq;
                SquareRoot(distance);
                info.depth = cr - distance;
                info.normalVector.x = (cx - pxs[i]) / distance;
                info.normalVector.y = (cy - pys[i]) / distance;
                info.collisionPoint.x = pxs[i];
                info.collisionPoint.y = pys[i];
                return;
            }
        }
#endif
    }

    // circle x,y,radius / capsule . topleft(x,y), radius of both circles, total height
    inline void CircleToCapsule(const float cx, const float cy, const float cr, const float x2, const float y2,
                                const float radius, const float height, CollisionInfo& info)
    {
        if (!RectToRect(cx - cr, cy - cr, cr * 2.0F, cr * 2.0F, x2, y2, radius * 2.0F, height)) [[likely]]
        {
            return; // quick bound check
        }
        CircleToCircle(cx, cy, cr, x2 + radius, y2 + radius, radius, info);
        if (info.isColliding())
            return;

        CircleToCircle(cx, cy, cr, x2 + radius, y2 + height - radius, radius, info);
        if (info.isColliding())
            return;

        RectToCircle(x2, y2 + radius, radius * 2.0F, height - radius * 2.0F, cx, cy, cr, info);
    }

    //----------------- CAPSULE -----------------//

    inline void CapsuleToCapsule(const float x1, const float y1, const float r1, const float h1, const float x2,
                                 const float y2, const float r2, const float h2, CollisionInfo& info)
    {
        // Fast bounding box check using RectToRect
        if (!RectToRect(x1, y1, r1 * 2.0F, h1, x2, y2, 2.0F * r2, h2)) [[likely]]
        {
            return; // most of the time we skip
        }

        // Check if any circles intersect with any other
        CircleToCircle(x1 + r1, y1 + r1, r1, x2 + r2, y2 + r2, r2, info);
        if (info.isColliding())
            return;
        CircleToCircle(x1 + r1, y1 + r1, r1, x2 + r2, y2 + h2 - r2, r2, info);
        if (info.isColliding())
            return;
        CircleToCircle(x1 + r1, y1 + h1 - r1, r1, x2 + r2, y2 + r2, r2, info);
        if (info.isColliding())
            return;
        CircleToCircle(x1 + r1, y1 + h1 - r1, r1, x2 + r2, y2 + h2 - r2, r2, info);
        if (info.isColliding())
            return;

        // Check if the rects intersect
        RectToRect(x1, y1 + r1, r1 * 2, h1 - r1 * 2, x2, y2 + r2, r2 * 2, h2 - r2 * 2, info);
        if (info.isColliding())
            return;

        // Check if the circles intersect rect
        RectToCircle(x2, y2 + r2, r2 * 2.0F, h2 - r2 * 2.0F, x1 + r1, y1 + r1, r1, info);
        if (info.isColliding())
            return;
        RectToCircle(x2, y2 + r2, r2 * 2.0F, h2 - r2 * 2.0F, x1 + r1, y1 + h1 - r1, r1, info);
        if (info.isColliding())
            return;
        RectToCircle(x1, y1 + r1, r1 * 2.0F, h1 - r1 * 2.0F, x2 + r2, y2 + r2, r2, info);
        if (info.isColliding())
            return;
        RectToCircle(x1, y1 + r1, r1 * 2.0F, h1 - r1 * 2.0F, x2 + r2, y2 + h2 - r2, r2, info);
    }

    inline void CapsuleToQuadrilateral(const float x, const float y, const float r, const float h, const float (&pxs)[4],
                                       const float (&pys)[4], CollisionInfo& info)
    {
        const auto bb = GetBBQuadrilateral(pxs, pys);
        if (!RectToRect(x, y, r * 2.0F, h, bb.x, bb.y, bb.width, bb.height)) [[likely]]
        {
            return; // quick bound check
        }

        CircleToQuadrilateral(x + r, y + r, r, pxs, pys, info);
        if (info.isColliding())
        {
            return;
        }
        CircleToQuadrilateral(x + r, y + (h - r), r, pxs, pys, info);
        if (info.isColliding())
        {
            return;
        }

        const float cr2xs[4] = {x, x + r * 2.0F, x + r * 2.0F, x};
        const float cr2ys[4] = {y + r, y + r, y + h - r, y + h - r};
        SAT(cr2xs, cr2ys, pxs, pys, info);
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

        __m128 xVec = _mm_set1_ps(x);
        __m128 yVec = _mm_set1_ps(y);
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

#endif //MAGIQUE_COLLISIONDETECTION_H