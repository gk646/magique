// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/internal/Macros.h>
#include <magique/util/Math.h>
#include <raylib/raylib.h>
#include <magique/util/Logging.h>

#include "internal/utils/CollisionPrimitives.h"

namespace magique
{
    float Lerp(const float min, const float max, const float step) { return min + step * (max - min); }

    float LerpInverse(const float min, const float max, const float value)
    {
        MAGIQUE_ASSERT(value <= max && value >= min, "'value' needs to be within the given bounds");
        return (value - min) / (max - min);
    }

    float Lermp(const float oldV, const float minO, const float maxO, const float minN, const float maxN)
    {
        return (oldV - minO / maxO - minO) * (maxN - minN) + minN;
    }

    bool IsPowerOfTwo(const unsigned int x) { return x != 0 && (x & (x - 1)) == 0; }

    unsigned int GetNextPowerOfTwo(unsigned int n)
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

    bool IsWholeNumber(const float num)
    {
        constexpr float EPSILON = 0.00001F;
        const float fractionalPart = num - static_cast<float>(static_cast<int>(num));
        return (fractionalPart < EPSILON) || (1.0F - fractionalPart < EPSILON);
    }

    Point GetPointOnCircleCircumferenceFromAngle(const Point& middle, const float radius, const float angle)
    {
        const auto direction = GetDirectionFromAngle(angle);
        return {middle.x + direction.x * radius, middle.y + direction.y * radius};
    }

    Point GetDirectionFromAngle(const float angle)
    {
        const float radians = (angle + 180) * DEG2RAD;
        return {-sinf(radians), cosf(radians)};
    }

    Point GetDirectionVector(const Point current, const Point target)
    {
        auto diff = target - current;
        return diff.normalize();
    }

    float GetAngleFromPoints(const Point current, const Point target)
    {
        const float dx = target.x - current.x;
        const float dy = target.y - current.y;
        if (dx == 0 && dy == 0)
        {
            return 0.0f;
        }
        const float angle = std::atan2f(dy, dx);
        const float angleDegrees = angle * RAD2DEG;
        float gameAngleDegrees = 90.0F + angleDegrees;
        if (gameAngleDegrees < 0)
        {
            gameAngleDegrees += 360.0f;
        }
        return gameAngleDegrees;
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

} // namespace magique