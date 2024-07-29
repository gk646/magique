#ifndef MAGIQUE_COLLISIONDETECTION_H
#define MAGIQUE_COLLISIONDETECTION_H

#include <immintrin.h>
#include <emmintrin.h>

//-----------------------------------------------
// Collision Detection Primitives with up to AVX2 intrinsics
//-----------------------------------------------
// .....................................................................
// In pure benchmark scenarios they are equal, but in game stress tests simd is much faster
// .....................................................................

//----------------- RECT -----------------//

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


//----------------- POINT -----------------//

inline bool PointToCircle(const float px, const float py, const float cx, const float cy, const float radius)
{
    // Simd is much slower
    const float dx = px - cx;
    const float dy = py - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
}

inline bool PointToRect(const float px, const float py, const float rx, const float ry, const float rw, const float rh)
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

//----------------- CIRCLE -----------------//
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


#endif //MAGIQUE_COLLISIONDETECTION_H