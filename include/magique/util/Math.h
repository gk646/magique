// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MATH_H
#define MAGIQUE_MATH_H

#include <vector>
#include <raylib/raylib.h>
#include <magique/core/Types.h>

//===============================
// Math Module
//===============================
// ........................ ........................................................
// This module contains helpful math functions related to game development
// ................................................................................

namespace magique
{
    // Returns a random float using raylib's GetRandomValue() - min and max included
    float MathRandom(float min  = 0.0F, float max = 1.0F);

    // Returns the value at the given step (0 to 1) on the scale between min and max - linear interpolation
    //      - step: value between 0 and 1 (inclusive) of where on the scale the value is
    float MathLerp(float min, float max, float step);

    // Returns from 0 to 1 how close the given value is to max starting from min
    float MathLerpInverse(float min, float max, float value);

    // Returns the translated old value in the old scale to the corresponding value in the new scale - linear remapping
    float MathLermp(float oldVal, float minO, float maxO, float minN, float maxN);

    // Exponential decay function
    float MathDecayExp(float a, float b, float minRatio = 0.1f, float steepness = 1.0f);

    // Returns true if a roll with the given chance was true - GetRandomFloat()
    // Note: Chance must be between 0.0 and 1.0F - 0 is never true 1 is always true
    bool MathRoll(float chance);

    // Picks a random element form the given pool
    template <typename T>
    const T* MathPickRandom(const std::vector<T>& pool);

    //================= NUMBERS =================//

    // True if the given "val" is within the specified range (included) - only uses 'operator<'
    template <typename T>
    bool MathInRange(const T& val, const T& min, const T& max);

    // Returns true if the given number is a power of two
    bool MathIsPowerOfTwo(unsigned int x);

    // Finds the closest power of two to the right of the given number
    unsigned int MathNextPowerOfTwo(unsigned int n);

    // Returns true of the given float is a whole number (e.g. 3.0F)
    bool MathIsWhole(float num);

    // Mirrors the given value vertically
    float MathMirrorVert(float value, float border);

    // Applies the change to the given value such that it goes closer to 0 (or 0) but not oversteps it
    float TowardsZero(float value, float change);

    // Applies the change to the given value such that it goes towards the closest max (positive or negative) but not oversteps it
    float AwayFromZero(float value, float change, float max);

    //================= GEOMETRY =================//

    // Returns the point on the given circle that intersects the given angle (on the circle outline)
    //      - angle: 0-360 starting from the top clockwise
    Point GetCirclePosOutline(const Point& middle, float radius, float angle);

    // Returns a random position on the given circle area
    Point GetCirclePosRandom(const Point& mid, float radius);

    // Returns p, if p is inside the circle defined by middle and radius
    // else returns the closest point on the circle using GetPointOnCircleFromAngle()
    Point GetClosestPointOnCircle(Point p, Point middle, float radius);

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique
{

    template <typename T>
    const T* MathPickRandom(const std::vector<T>& pool)
    {
        if (pool.empty())
        {
            return nullptr;
        }
        return &pool[GetRandomValue(0, pool.size() - 1)];
    }

    template <typename T>
    bool MathInRange(const T& val, const T& min, const T& max)
    {
        return (val >= min) && (val <= max);
    }

} // namespace magique

#endif // MAGIQUE_MATH_H
