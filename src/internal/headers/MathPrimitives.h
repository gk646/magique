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
    // Rectangle edges
    const float rx2 = x1 + w1;
    const float ry2 = y1 + h1;

    // Check collision with capsule's bounding rectangle first
    if (rx2 < x2 || x1 > x2 + 2 * radius || ry2 < y2 || y1 > y2 + height) [[likely]]
    {
        return false;
    }

    // Capsule circles' centers
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

    // Check if rectangle overlaps with the capsule's middle rectangle
    if (x1 < x2 + 2 * radius && rx2 > x2 && y1 < y2 + height && ry2 > y2)
    {
        return true;
    }

    return false;
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

//----------------- ROTATION -----------------//

// Takes translation x and y / point coordinates relative to translation / rotation clockwise in degrees from the top / anchor is relative to the points
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

// Same as RotatePoints4() but with 3
inline void RotatePoints3(float x, float y, float (&pxs)[3], float (&pys)[3], const float rotation, const float anchorX, const float anchorY)
{
#ifdef MAGIQUE_USE_AVX2
    const float cosTheta = cosf(rotation * DEG2RAD);
    const float sinTheta = sinf(rotation * DEG2RAD);

    // Load constants into SIMD registers
    __m128 cosVec = _mm_set1_ps(cosTheta);
    __m128 sinVec = _mm_set1_ps(sinTheta);
    __m128 anchorXVec = _mm_set1_ps(anchorX);
    __m128 anchorYVec = _mm_set1_ps(anchorY);
    __m128 xVec = _mm_set1_ps(x);
    __m128 yVec = _mm_set1_ps(y);

    // Load points into SIMD registers, with a dummy value for the fourth position
    __m128 px = _mm_setr_ps(pxs[0], pxs[1], pxs[2], 0.0f);
    __m128 py = _mm_setr_ps(pys[0], pys[1], pys[2], 0.0f);

    // Subtract anchor from points
    __m128 tx = _mm_sub_ps(px, anchorXVec);
    __m128 ty = _mm_sub_ps(py, anchorYVec);

    // Perform the rotation
    __m128 rotatedX = _mm_sub_ps(_mm_mul_ps(tx, cosVec), _mm_mul_ps(ty, sinVec));
    __m128 rotatedY = _mm_add_ps(_mm_mul_ps(tx, sinVec), _mm_mul_ps(ty, cosVec));

    // Add anchor and offset back to the rotated coordinates
    rotatedX = _mm_add_ps(_mm_add_ps(rotatedX, anchorXVec), xVec);
    rotatedY = _mm_add_ps(_mm_add_ps(rotatedY, anchorYVec), yVec);

    // Store the results back into the input arrays
    alignas(16) float rotatedXArr[4];
    alignas(16) float rotatedYArr[4];

    _mm_store_ps(rotatedXArr, rotatedX);
    _mm_store_ps(rotatedYArr, rotatedY);

    for (int i = 0; i < 3; ++i) {
        pxs[i] = rotatedXArr[i];
        pys[i] = rotatedYArr[i];
    }

#else
    const float cosTheta = cosf(rotation * DEG2RAD);
    const float sinTheta = sinf(rotation * DEG2RAD);

    for (int i = 0; i < 3; ++i)
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

// Given the 4x and 4y coordinates of a shape assigns the bounding rect to given values
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