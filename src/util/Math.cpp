// SPDX-License-Identifier: zlib-acknowledgement
#include <algorithm>

#include <magique/util/Math.h>
#include <raylib/raylib.h>
#include <magique/util/RayUtils.h>

#include "internal/utils/CollisionPrimitives.h"

namespace magique
{
    float MathRandom(const float min, const float max)
    {
        constexpr float ACCURACY = 100'000.0F;
        const int minI = static_cast<int>(min * ACCURACY);
        const int maxI = static_cast<int>(max * ACCURACY);
        const auto val = static_cast<float>(GetRandomValue(minI, maxI));
        return val / ACCURACY;
    }

    float MathLerp(const float min, const float max, const float step) { return min + step * (max - min); }

    float MathLerpInverse(const float min, const float max, const float value)
    {
        MAGIQUE_ASSERT(value <= max && value >= min, "'value' needs to be within the given bounds");
        return (value - min) / (max - min);
    }

    float MathLermp(const float oldV, const float minO, const float maxO, const float minN, const float maxN)
    {
        return (oldV - minO / maxO - minO) * (maxN - minN) + minN;
    }

    float MathDecayExp(float a, float b, float minRatio, float steepness)
    {
        const float difference = std::abs(a - b);
        const float ratio = minRatio + (1.0f - minRatio) * std::exp(-steepness * difference);
        return ratio;
    }

    bool MathRoll(const float chance)
    {
        const auto rand = MathRandom(0.0F, 1.0F);
        return rand < std::clamp(chance, 0.0F, 1.0F);
    }

    bool MathIsPowerOfTwo(const unsigned int x) { return x != 0 && (x & (x - 1)) == 0; }

    unsigned int MathNextPowerOfTwo(unsigned int n)
    {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }

    bool MathIsWhole(const float num)
    {
        constexpr float EPSILON = 0.00001F;
        const float fractionalPart = num - static_cast<float>(static_cast<int>(num));
        return (fractionalPart < EPSILON) || (1.0F - fractionalPart < EPSILON);
    }

    bool IsEvenNumber(const float num)
    {
        const auto integer = static_cast<int64_t>(std::floor(num));
        return integer % 2 == 0;
    }

    float TowardsZero(float value, float change)
    {
        if (value < 0.0F)
        {
            return std::min(value + change, 0.0F);
        }
        if (value > 0.0F)
        {
            return std::max(value - change, 0.0F);
        }
        return value;
    }

    float AwayFromZero(float value, float change, float max)
    {
        if (value < 0.0F)
        {
            return std::max(value - change, -max);
        }
        if (value > 0.0F)
        {
            return std::min(value + change, max);
        }
        return value;
    }

    float MathMirrorVert(float value, float border) { return value + 2 * (border - value); }

    Point GetCirclePosOutline(const Point& middle, const float radius, const float angle)
    {
        const auto direction = Point::FromRotation(angle);
        return {middle.x + (direction.x * radius), middle.y + (direction.y * radius)};
    }

    Point GetCirclePosRandom(const Point& mid, float radius)
    {
        const auto angle = GetRandomValue(0, 359);
        const Point dir = Point::FromRotation(angle);
        return mid + dir * (MathRandom(0, 1.0F) * radius);
    }

    Point GetClosestPointOnCircle(Point p, Point middle, float radius)
    {
        if (middle.euclidean(p) > radius)
        {
            return GetCirclePosOutline(middle, radius, middle.angle(p));
        }
        else
        {
            return p;
        }
    }

} // namespace magique
