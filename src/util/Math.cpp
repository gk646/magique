// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/internal/Macros.h>
#include <magique/util/Math.h>
#include <raylib/raylib.h>
#include <magique/util/Logging.h>
#include <magique/util/RayUtils.h>

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

    float ExponentialDecay(float a, float b, float minRatio, float steepness)
    {
        const float difference = std::abs(a - b);
        const float ratio = minRatio + (1.0f - minRatio) * std::exp(-steepness * difference);
        return ratio;
    }

    bool RollWithChance(const float chance)
    {
        const auto rand = GetRandomFloat(0.0F, 1.0F);
        return rand < clamp(chance, 0.0F, 1.0F);
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

    bool IsEvenNumber(const float num)
    {
        const auto integer = static_cast<int64_t>(std::floor(num));
        return integer % 2 == 0;
    }

    float TowardsZero(float value, float change)
    {
        if (value < 0.0F)
        {
            return minValue(value + change, 0.0F);
        }
        if (value > 0.0F)
        {
            return maxValue(value - change, 0.0F);
        }
        return value;
    }

    float AwayFromZero(float value, float change, float max)
    {
        if (value < 0.0F)
        {
            return maxValue(value - change, -max);
        }
        if (value > 0.0F)
        {
            return minValue(value + change, max);
        }
        return value;
    }

    int MirrorVertically(int value, int border) { return value + 2 * (border - value); }

    float AngleChange(float angle, float change)
    {
        float newAngle = angle + change;
        if (newAngle >= 360)
        {
            newAngle -= 360;
        }
        else if (newAngle < 0)
        {
            newAngle += 360;
        }
        return newAngle;
    }

    float AngleDiff(float one, float two)
    {
        float diff = std::abs(one - two);
        if (diff > 180.0F)
        {
            return std::abs(diff - 360.0F);
        }
        else
        {
            return diff;
        }
    }

    float AngleDistance(float one, float two)
    {
        float dist = two - one;
        if (dist < -180.0F)
        {
            return dist + 360.0F;
        }
        else if (dist > 180.0F)
        {
            return dist - 360.0F;
        }
        return dist;
    }

    float AngleModulate(float old, float target, float max)
    {
        auto dist = AngleDistance(old, target);
        if (dist > 0.0F)
        {
            return AngleChange(old, std::min(dist, max));
        }
        else if (dist < 0.0F)
        {
            return AngleChange(old, std::max(dist, -max));
        }
        return old;
    }

    Point GetCirclePosOutline(const Point& middle, const float radius, const float angle)
    {
        const auto direction = GetDirectionFromAngle(angle);
        return {middle.x + (direction.x * radius), middle.y + (direction.y * radius)};
    }

    Point GetCirclePosRandom(const Point& mid, float radius)
    {
        const auto angle = GetRandomValue(0, 359);
        const Point dir = GetDirectionFromAngle((float)angle);
        return mid + dir * (GetRandomFloat(0, 1.0F) * radius);
    }

    Point GetDirectionFromAngle(const float angle)
    {
        const float radians = (angle + 180) * DEG2RAD;
        return {-sinf(radians), cosf(radians)};
    }

    Point GetDirectionFromPoints(const Point current, const Point target)
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
            return 0.0F;
        }
        const float angle = std::atan2f(dy, dx);
        const float angleDegrees = angle * RAD2DEG;
        float gameAngleDegrees = 90.0F + angleDegrees;
        if (gameAngleDegrees < 0)
        {
            gameAngleDegrees += 360.0F;
        }
        return gameAngleDegrees;
    }

    float GetAngleFromDirection(Point dir)
    {
        const float angle = std::atan2f(dir.y, dir.x);
        const float angleDegrees = angle * RAD2DEG;
        float gameAngleDegrees = 90.0F + angleDegrees;
        if (gameAngleDegrees < 0)
        {
            gameAngleDegrees += 360.0F;
        }
        return gameAngleDegrees;
    }

    float GetShortestDistToRect(Point p, const Rectangle& r)
    {
        // Quite funny solution
        // You clamp the point coordinates to the rects side lengths
        const Point closest = {clamp(p.x, r.x, r.x + r.width), clamp(p.y, r.y, r.y + r.height)};
        return closest.euclidean(p);
    }

    Point GetClosestPointOnCircle(Point p, Point middle, float radius)
    {
        if (middle.euclidean(p) > radius)
        {
            return GetCirclePosOutline(middle, radius, GetAngleFromPoints(middle, p));
        }
        else
        {
            return p;
        }
    }

    Point GetCenteredPos(const Rectangle& one, const Point& two)
    {
        Point p;
        p.x = one.x + one.width / 2.0F - two.x / 2.0F;
        p.y = one.y + one.height / 2.0F - two.y / 2.0F;
        return p;
    }

} // namespace magique
