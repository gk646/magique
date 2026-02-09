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
    //================= FUNCTIONS =================//

    // Returns the value at the given step (0 to 1) on the scale between min and max - linear interpolation
    //      - step: value between 0 and 1 (inclusive) of where on the scale the value is
    float Lerp(float min, float max, float step);

    // Returns from 0 to 1 how close the given value is to max starting from min
    float LerpInverse(float min, float max, float value);

    // Returns the translated old value in the old scale to the corresponding value in the new scale - linear remapping
    float Lermp(float oldVal, float minO, float maxO, float minN, float maxN);

    float ExponentialDecay(float a, float b, float minRatio = 0.1f, float steepness = 1.0f);

    // Returns true if a roll with the given chance was true - GetRandomFloat()
    // Note: Chance must be between 0.0 and 1.0F - 0 is never true 1 is always true
    bool RollWithChance(float chance);

    // Picks a random element form the given pool
    template <typename T>
    const T* PickRandomElement(const std::vector<T>& pool);

    //================= NUMBERS =================//

    // True if the given "val" is within the specified range (included) - only uses 'operator<'
    template <typename T>
    bool IsValueInRange(const T& val, const T& min, const T& max);

    // Returns true if the given number is a power of two
    bool IsPowerOfTwo(unsigned int x);

    // Finds the closest power of two to the right of the given number
    unsigned int GetNextPowerOfTwo(unsigned int n);

    // Returns true of the given float is a whole number (e.g. 3.0F)
    bool IsWholeNumber(float num);

    // Returns true if the number is even
    bool IsEvenNumber(float num);

    // Applies the change to the given value such that it goes closer to 0 (or 0) but not oversteps it
    float TowardsZero(float value, float change);

    // Applies the change to the given value such that it goes towards the closest max (positive or negative) but not oversteps it
    float AwayFromZero(float value, float change, float max);

    // Mirrors the given value vertically
    int MirrorVertically(int value, int border);

    // Applies the change of degrees to the angle such that it stays within 0-360
    float AngleChange(float angle, float change);

    // Returns the shortest difference between two angles in degrees (always positive)
    float AngleDiff(float one, float two);

    // Returns the distance one has to go from one to get to two in degrees (signed ! e.g. with AngleChange())
    float AngleDistance(float one, float two);

    // Returns a new angle by modulating old towards target (the fastest way around) by at most the given maximum
    float AngleModulate(float old, float target, float max = 4.0F);

    //================= GEOMETRY =================//

    // Returns the point on the given circle that intersects the given angle (on the circle outline)
    //      - angle: 0-360 starting from the top clockwise
    Point GetCirclePosOutline(const Point& middle, float radius, float angle);

    // Returns a random position on the given circle area
    Point GetCirclePosRandom(const Point& mid, float radius);

    // Returns the direction vector of the given angle
    //      - angle: 0-360 starting from the top clockwise
    Point GetDirFromAngle(float angle);

    // Returns a normalized direction vector that points from the current to the target position
    // This is useful for moving the entity towards the next tile (pos.x += direction.x * movementSpeed)
    Point GetDirFromPoints(Point current, Point target);

    // Returns the angle that spans from the current to target point
    float GetAngleFromPoints(Point current, Point target);

    // Returns the angle in which the direction vector points - starting at 0 at 12'o clock
    float GetAngleFromDir(Point dir);

    // Returns the shortest possible distance to connect point p with rect r
    float GetShortestDistToRect(Point p, const Rectangle& r);

    // Returns p, if p is inside the circle defined by middle and radius
    // else returns the closest point on the circle using GetPointOnCircleFromAngle()
    Point GetClosestPointOnCircle(Point p, Point middle, float radius);

    // Returns the coordinates to center two (give only by dimensions) in the middle of one
    Point GetCenteredPos(const Rectangle& one, const Point& two);

} // namespace magique


//================= IMPLEMENTATION =================//


namespace magique
{

    template <typename T>
    const T* PickRandomElement(const std::vector<T>& pool)
    {
        if (pool.empty())
        {
            return nullptr;
        }
        return &pool[GetRandomValue(0, pool.size() - 1)];
    }

    template <typename T>
    bool IsValueInRange(const T& val, const T& min, const T& max)
    {
        return min <= val && val <= max;
    }

} // namespace magique

#endif //MAGIQUE_MATH_H
