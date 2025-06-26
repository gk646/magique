// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MATH_H
#define MAGIQUE_MATH_H

#include "magique/core/Types.h"

//===============================
// Math Module
//===============================
// ................................................................................
// This module contains helpful math functions related to game development
// ................................................................................

namespace magique
{
    // Clamps the given value - returns low if values is smaller than it, high if value is bigger than it, else val itself
    template <typename T>
    constexpr T clamp(T val, T low, T high);

    // Returns the value at the given step (0 to 1) on the scale between min and max - linear interpolation
    //      - step: value between 0 and 1 (inclusive) of where on the scale the value is
    float Lerp(float min, float max, float step);

    // Returns from 0 to 1 how close the given value is to max starting from min
    float LerpInverse(float min, float max, float value);

    // Returns the translated old value in the old scale to the corresponding value in the new scale - linear remapping
    float Lermp(float oldVal, float minO, float maxO, float minN, float maxN);

    // True if the given "val" is within the specified range (included) - only uses 'operator<'
    template <typename T>
    bool IsValueInRange(const T& val, const T& min, const T& max);

    // Returns true if the given number is a power of two
    bool IsPowerOfTwo(unsigned int x);

    // Finds the closest power of two to the right of the given number
    unsigned int GetNextPowerOfTwo(unsigned int n);

    // Returns true of the given float is a whole number (e.g. 3.0F)
    bool IsWholeNumber(float num);

    // Returns the point on the given circle that intersects the given angle
    //      - angle: 0-360 starting from the top clockwise
    Point GetPointOnCircleCircumferenceFromAngle(const Point& middle, float radius, float angle);

    // Returns the direction vector of the given angle
    //      - angle: 0-360 starting from the top clockwise
    Point GetDirectionFromAngle(float angle);

    // Returns a normalized direction vector that points from the current to the target position
    // This is useful for moving the entity towards the next tile (pos.x += direction.x * movementSpeed)
    Point GetDirectionVector(Point current, Point target);

    // Returns the angle that spans from the current to target point
    float GetAngleFromPoints(Point current, Point target);

} // namespace magique


//================= IMPLEMENTATION =================//

namespace magique
{
    template <typename T>
    bool IsValueInRange(const T& val, const T& min, const T& max)
    {
        return min <= val && val <= max;
    }

    template <typename T>
    constexpr T clamp(T val, T low, T high)
    {
        if (val < low)
        {
            return low;
        }
        if (val > high)
        {
            return high;
        }
        return val;
    }

} // namespace magique

#endif //MAGIQUE_MATH_H