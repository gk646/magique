#ifndef MAGIQUE_COLLISIONDETECTION_H
#define MAGIQUE_COLLISIONDETECTION_H

#include <immintrin.h>
#include <emmintrin.h>

//-----------------------------------------------
// Math Primitives with up to AVX2 intrinsics
//-----------------------------------------------
// .....................................................................
// In pure benchmark scenarios they are equal, but in game stress tests simd is much faster
// .....................................................................

//----------------- POINT -----------------//

inline bool PointInCircle(const float px, const float py, const float cx, const float cy, const float radius)
{
    // Simd is much slower
    const float dx = px - cx;
    const float dy = py - cy;
    return dx * dx + dy * dy <= radius * radius;
}

inline bool PointInRect(const float px, const float py, const float rx, const float ry, const float rw, const float rh)
{
#ifdef MAGIQUE_USE_AVX2
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

//----------------- SAT -----------------//

// checks 4 points against another 4 points
inline bool SAT(const float (&pxs)[4], const float (&pys)[4], const float (&p1xs)[4], const float (&p1ys)[4])
{
#ifdef MAGIQUE_USE_AVX2
    const auto OverlapOnAxis = [](const float(&pxs)[4], const float(&pys)[4], const float(&p1xs)[4],
                                  const float(&p1ys)[4], const float axisX, const float axisY)
    {
        const auto project =
            [](const float(&pxs)[4], const float(&pys)[4], const float axisX, const float axisY, float& min, float& max)
        {
            const __m128 axisX_vec = _mm_set1_ps(axisX);
            const __m128 axisY_vec = _mm_set1_ps(axisY);

            const __m128 pxs_vec = _mm_loadu_ps(pxs);
            const __m128 pys_vec = _mm_loadu_ps(pys);

            const __m128 proj_vec = _mm_add_ps(_mm_mul_ps(pxs_vec, axisX_vec), _mm_mul_ps(pys_vec, axisY_vec));

            float projections[4];
            _mm_storeu_ps(projections, proj_vec);

            __m128 proj_min = proj_vec;
            __m128 proj_max = proj_vec;

            proj_min = _mm_min_ps(proj_min, _mm_shuffle_ps(proj_min, proj_min, _MM_SHUFFLE(2, 3, 0, 1)));
            proj_min = _mm_min_ps(proj_min, _mm_shuffle_ps(proj_min, proj_min, _MM_SHUFFLE(1, 0, 3, 2)));

            proj_max = _mm_max_ps(proj_max, _mm_shuffle_ps(proj_max, proj_max, _MM_SHUFFLE(2, 3, 0, 1)));
            proj_max = _mm_max_ps(proj_max, _mm_shuffle_ps(proj_max, proj_max, _MM_SHUFFLE(1, 0, 3, 2)));

            min = _mm_cvtss_f32(proj_min);
            max = _mm_cvtss_f32(proj_max);
        };

        float minA, maxA, minB, maxB;
        project(pxs, pys, axisX, axisY, minA, maxA);
        project(p1xs, p1ys, axisX, axisY, minB, maxB);
        return maxA >= minB && maxB >= minA;
    };
#else
    const auto OverlapOnAxis = [](const float(&pxs)[4], const float(&pys)[4], const float(&p1xs)[4],
                                  const float(&p1ys)[4], float axisX, float axisY)
    {
        const auto project =
            [](const float(&pxs)[4], const float(&pys)[4], float axisX, float axisY, float& min, float& max)
        {
            min = max = (pxs[0] * axisX + pys[0] * axisY);
            for (int i = 1; i < 4; ++i)
            {
                const float projection = (pxs[i] * axisX + pys[i] * axisY);
                if (projection < min)
                    min = projection;
                if (projection > max)
                    max = projection;
            }
        };


        float minA, maxA, minB, maxB;
        project(pxs, pys, axisX, axisY, minA, maxA);
        project(p1xs, p1ys, axisX, axisY, minB, maxB);
        return maxA >= minB && maxB >= minA;
    };

#endif
    for (int i = 0; i < 4; ++i)
    {
        int next = i + 1;
        if (next == 4)
            next = 0;
        const float edgeX = pxs[next] - pxs[i];
        const float edgeY = pys[next] - pys[i];

        const float axisX = -edgeY;
        const float axisY = edgeX;

        if (!OverlapOnAxis(pxs, pys, p1xs, p1ys, axisX, axisY))
            return false;
    }

    for (int i = 0; i < 4; ++i)
    {
        int next = i + 1;
        if (next == 4)
            next = 0;
        const float edgeX = p1xs[next] - p1xs[i];
        const float edgeY = p1ys[next] - p1ys[i];
        const float axisX = -edgeY;
        const float axisY = edgeX;

        if (!OverlapOnAxis(pxs, pys, p1xs, p1ys, axisX, axisY))
            return false;
    }
    return true;
}

//----------------- RECT -----------------//

// rect: x,y,width,height / rect: x,y,width,height
inline bool RectToRect(const float x1, const float y1, const float w1, const float h1, const float x2, const float y2,
                       const float w2, const float h2)
{
    // This looks a bit faster in under mass load
#ifdef MAGIQUE_USE_AVX2
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

// rect: x,y,width,height / circle: x,y, radius
inline bool RectToCircle(const float rx, const float ry, const float rw, const float rh, const float cx, const float cy,
                         const float cr)
{
#ifdef MAGIQUE_USE_AVX2
    const __m128 circle_center = _mm_set_ps(0.0f, 0.0f, cy, cx);

    const __m128 rect_min = _mm_set_ps(0.0f, 0.0f, ry, rx);
    const __m128 rect_max = _mm_set_ps(0.0f, 0.0f, ry + rh, rx + rw);

    const __m128 clamped = _mm_min_ps(_mm_max_ps(circle_center, rect_min), rect_max);

    const __m128 diff = _mm_sub_ps(circle_center, clamped);

    const __m128 diff_squared = _mm_mul_ps(diff, diff);

    const __m128 sum = _mm_add_ps(diff_squared, _mm_shuffle_ps(diff_squared, diff_squared, _MM_SHUFFLE(2, 3, 0, 1)));

    const float distance_squared = _mm_cvtss_f32(sum);

    const float radius_squared = cr * cr;
    return distance_squared <= radius_squared;
#else
    float closestX = (cx < rx) ? rx : (cx > rx + rw) ? rx + rw : cx;
    float closestY = (cy < ry) ? ry : (cy > ry + rh) ? ry + rh : cy;

    float dx = cx - closestX;
    float dy = cy - closestY;

    return (dx * dx + dy * dy) <= (cr * cr);
#endif
}

// Top left of the capsule and its height and radius
inline bool RectToCapsule(const float x1, const float y1, const float w1, const float h1, const float x2, const float y2,
                          const float radius, const float height)
{

    // Fast bounding box check using RectToRect
    if (!RectToRect(x1, y1, w1, h1, x2, y2, 2.0F * radius, height))
    {
        return false;
    }


    const float rx2 = x1 + w1;
    const float ry2 = y1 + h1;
    const float cx1 = x2 + radius;
    const float cy1 = y2 + radius;
    const float cx2 = x2 + radius;
    const float cy2 = y2 + height - radius;

    // Check if any corner of the rectangle is inside the capsule's end circles
    if (PointInCircle(x1, y1, cx1, cy1, radius) || PointInCircle(rx2, y1, cx1, cy1, radius) ||
        PointInCircle(x1, ry2, cx1, cy1, radius) || PointInCircle(rx2, ry2, cx1, cy1, radius) ||
        PointInCircle(x1, y1, cx2, cy2, radius) || PointInCircle(rx2, y1, cx2, cy2, radius) ||
        PointInCircle(x1, ry2, cx2, cy2, radius) || PointInCircle(rx2, ry2, cx2, cy2, radius))
    {
        return true;
    }

    if (x1 < x2 + 2 * radius && rx2 > x2 && y1 < y2 + height && ry2 > y2)
    {
        return true;
    }

    return false;
}

inline bool RectToTriangle(const float x1, const float y1, const float w1, const float h1, const float p1x,
                           const float p1y, const float p2x, const float p2y, const float p3x, const float p3y)
{
    const float rectX[4] = {x1, x1 + w1, x1 + w1, x1};
    const float rectY[4] = {y1, y1, y1 + h1, y1 + h1};

    const float triX[4] = {p1x, p2x, p3x, p1x};
    const float triY[4] = {p1y, p2y, p3y, p1y};
    return SAT(rectX, rectY, triX, triY);
}

//----------------- CIRCLE -----------------//

// circle: x,y, radius / circle: x,y, radius
inline bool CircleToCircle(const float x1, const float y1, const float r1, const float x2, const float y2,
                           const float r2)
{
#ifdef MAGIQUE_USE_AVX2
    const __m128 center1 = _mm_set_ps(0, 0, y1, x1);
    const __m128 center2 = _mm_set_ps(0, 0, y2, x2);

    const __m128 diff = _mm_sub_ps(center1, center2);

    const __m128 diff_sq = _mm_mul_ps(diff, diff);

    const __m128 dist_sq = _mm_hadd_ps(diff_sq, diff_sq);
    const __m128 dist_sq_sum = _mm_hadd_ps(dist_sq, dist_sq);

    const float radii_sum = r1 + r2;
    const float radii_sq_sum = radii_sum * radii_sum;
    const __m128 radii_sq_vec = _mm_set1_ps(radii_sq_sum);

    const __m128 cmp = _mm_cmple_ps(dist_sq_sum, radii_sq_vec);
    return (_mm_movemask_ps(cmp) & 0x1) != 0;
#else
    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float distance_squared = dx * dx + dy * dy;
    const float radius_sum = r1 + r2;
    return distance_squared <= radius_sum * radius_sum;
#endif
}

inline bool CircleToQuadrilateral(const float cx, const float cy, const float cr, const float (&pxs)[4],
                                  const float (&pys)[4])
{
#ifdef MAGIQUE_USE_AVX2
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

    if (windingNumber != 0)
        return true;

    const __m128 cx_vec = _mm_set1_ps(cx);
    const __m128 cy_vec = _mm_set1_ps(cy);

    for (int i = 0; i < 4; ++i)
    {
        int next = i + 1;
        if (next == 4)
            next = 0;

        const __m128 px_vec = _mm_set_ps(pxs[next], pxs[next], pxs[i], pxs[i]);
        const __m128 py_vec = _mm_set_ps(pys[next], pys[next], pys[i], pys[i]);
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

        float dist_sq[4];
        _mm_storeu_ps(dist_sq, dist_sq_vec);

        if (dist_sq[0] <= cr * cr || dist_sq[2] <= cr * cr)
            return true;
    }
    return false;
#else
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

    if (windingNumber != 0)
        return true;

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
        int next = i + 1;
        if (next == 4)
            next = 0;
        const float distSq = DistanceSquaredPointToSegment(pxs[i], pys[i], pxs[next], pys[next], cx, cy);
        if (distSq <= cr * cr)
            return true;
    }
    return false;
#endif
}

// circle x,y,radius / capsule . topleft(x,y), radius of both circles, total height
inline bool CircleToCapsule(const float cx, const float cy, const float cr, const float x2, const float y2,
                            const float radius, const float height)
{

    if (CircleToCircle(cx, cy, cr, x2 + radius, y2 + radius, radius) ||
        CircleToCircle(cx, cy, cr, x2 + radius, y2 + height - radius, radius))
    {
        return true;
    }

    return RectToCircle(x2, y2 + radius, radius * 2.0F, height - radius * 2.0F, cx, cy, cr);
}

//----------------- CAPSULE -----------------//

inline bool CapsuleToCapsule(const float x1, const float y1, const float r1, const float h1, const float x2,
                             const float y2, const float r2, const float h2)
{
    // Fast bounding box check using RectToRect
    if (!RectToRect(x1, y1, 2.0F * r1, h1, x2, y2, 2.0F * r2, h2)) [[likely]]
    {
        return false; // most of the time we skip
    }

    // Check if any circles intersect with any other
    if (CircleToCircle(x1 + r1, y1 + r1, r1, x2 + r2, y2 + r2, r2) ||
        CircleToCircle(x1 + r1, y1 + r1, r1, x2 + r2, y2 + h2 - r2, r2) ||
        CircleToCircle(x1 + r1, y1 + h1 - r1, r1, x2 + r2, y2 + r2, r2) ||
        CircleToCircle(x1 + r1, y1 + h1 - r1, r1, x2 + r2, y2 + h2 - r2, r2))
    {
        return true;
    }

    // Check if the rects intersect
    if (RectToRect(x1, y1 + r1, r1 * 2, h1 - r1 * 2, x2, y2 + r2, r2 * 2, h2 - r2 * 2))
    {
        return true;
    }

    // Check if the circles intersect rect
    if (RectToCircle(x2, y2 + r2, h2 - r2 * 2, r2 * 2, x1 + r1, y1 + r1, r1) ||
        RectToCircle(x2, y2 + r2, h2 - r2 * 2, r2 * 2, x1 + r1, y1 + h1 - r1, r1) ||
        RectToCircle(x1, y1 + r1, h1 - r1 * 2, r1 * 2, x2 + r2, y2 + r2, r2) ||
        RectToCircle(x1, y1 + r1, h1 - r1 * 2, r1 * 2, x2 + r2, y2 + h2 - r2, r2))
    {
        return true;
    }

    return false;
}

//----------------- TRIANGLE -----------------//

// Two triangles given by their respective x and y coordinate arrays AND
inline bool TriangleToTriangle(const float t1x1, const float t1y1, const float t1x2, const float t1y2,
                               const float t1x3, const float t1y3, const float t2x1, const float t2y1,
                               const float t2x2, const float t2y2, const float t2x3, const float t2y3)
{
    // Last point is first point repeated
    const float t1xs[4] = {t1x1, t1x2, t1x3, t1x1};
    const float t1ys[4] = {t1y1, t1y2, t1y3, t1y1};
    const float t2xs[4] = {t2x1, t2x2, t2x3, t2x1};
    const float t2ys[4] = {t2y1, t2y2, t2y3, t2y1};
    return SAT(t1xs, t1ys, t2xs, t2ys);
}

//----------------- ROTATION -----------------//

// Takes translation x and y / point coordinates relative to translation / rotation clockwise in degrees from the top / anchor is relative to the points
// Translates and rotates the points!
inline void RotatePoints4(float x, float y, float (&pxs)[4], float (&pys)[4], const float rotation, const float anchorX,
                          const float anchorY)
{
#ifdef MAGIQUE_USE_AVX2
    const float cosTheta = cosf(rotation * DEG2RAD);
    const float sinTheta = sinf(rotation * DEG2RAD);

    __m128 cosVec = _mm_set1_ps(cosTheta);
    __m128 sinVec = _mm_set1_ps(sinTheta);

    __m128 anchorXVec = _mm_set1_ps(anchorX);
    __m128 anchorYVec = _mm_set1_ps(anchorY);

    __m128 px = _mm_set_ps(pxs[3], pxs[2], pxs[1], pxs[0]);
    __m128 py = _mm_set_ps(pys[3], pys[2], pys[1], pys[0]);

    __m128 tx = _mm_sub_ps(px, anchorXVec);
    __m128 ty = _mm_sub_ps(py, anchorYVec);

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

//----------------- BOUNDING BOX -----------------//

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


#endif //MAGIQUE_COLLISIONDETECTION_H