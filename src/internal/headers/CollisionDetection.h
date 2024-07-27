#ifndef MAGIQUE_COLLISIONDETECTION_H
#define MAGIQUE_COLLISIONDETECTION_H

#include <immintrin.h>

//-----------------------------------------------
// Collision Detection Primitives with AVX2 intrinsics
//-----------------------------------------------

inline bool RectIntersectsRect(const float x1, const float y1, const float w1, const float h1, const float x2,
                               const float y2, const float w2, const float h2)
{
#ifndef MAGIQUE_USE_AVX2
    const __m256 rect1 = _mm256_set_ps(y1 + h1, y1, x1 + w1, x1, y1 + h1, y1, x1 + w1, x1);
    const __m256 rect2 = _mm256_set_ps(y2, y2 + h2, x2, x2 + w2, y2, y2 + h2, x2, x2 + w2);

    const __m256 cmp1 = _mm256_cmp_ps(rect1, rect2, _CMP_GE_OQ);
    const __m256 cmp2 = _mm256_cmp_ps(rect1, rect2, _CMP_LE_OQ);

    const __m256 result = _mm256_and_ps(cmp1, cmp2);

    return (_mm256_movemask_ps(result) & 0xF) == 0xF;
#else
    // Non-SIMD version
    return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
#endif
}

inline bool PointIntersectsCircle(const float px, const float py, const float cx, const float cy, const float radius)
{
#ifdef MAGIQUE_USE_AVX2
    const __m256 point = _mm256_set_ps(px, py, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    const __m256 circle = _mm256_set_ps(cx, cy, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    const __m256 diff = _mm256_sub_ps(point, circle);

    const __m256 diff_squared = _mm256_mul_ps(diff, diff);

    const __m256 squared_dist = _mm256_hadd_ps(diff_squared, diff_squared);

    const float sum_of_squares = _mm256_cvtss_f32(squared_dist) + _mm256_cvtss_f32(_mm256_permute2f128_ps(squared_dist, squared_dist, 0x1));

    const float radius_squared = radius * radius;
    return sum_of_squares <= radius_squared;
#else
    // Non-SIMD version
    const float dx = px - cx;
    const float dy = py - cy;
    return (dx * dx + dy * dy) <= (radius * radius);
#endif
}

inline bool PointIntersectsRect(const float px, const float py, const float rx, const float ry, const float rw, const float rh)
{
#ifdef MAGIQUE_USE_AVX2
    const __m256 point = _mm256_set_ps(py, py, py, py, px, px, px, px);
    const __m256 rect_min = _mm256_set_ps(ry, ry, ry, ry, rx, rx, rx, rx);
    const __m256 rect_max = _mm256_set_ps(ry + rh, ry + rh, ry + rh, ry + rh, rx + rw, rx + rw, rx + rw, rx + rw);

    const __m256 cmp_min = _mm256_cmp_ps(point, rect_min, _CMP_GE_OQ);
    const __m256 cmp_max = _mm256_cmp_ps(point, rect_max, _CMP_LE_OQ);

    const __m256 result = _mm256_and_ps(cmp_min, cmp_max);

    return (_mm256_movemask_ps(result) & 0x3) == 0x3;
#else
    // Non-SIMD version
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
#endif
}

#endif //MAGIQUE_COLLISIONDETECTION_H