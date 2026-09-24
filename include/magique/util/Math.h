// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MATH_H
#define MAGIQUE_MATH_H

#include <cfloat>
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
    float MathRandom(float min = 0.0F, float max = 1.0F);

    // Returns a random number based on a normal distribution
    // avg is the expected value and stddev decides how big outliers can get
    // Note: This is interesting for games as it allows the general case to be more likely while still having rng
    float MathRandomGaussian(float avg, float stddev, float min = FLT_MIN, float max = FLT_MAX);

    // Returns from 0 to 1 how close the given value is to max starting from min
    float MathLerpInverse(float min, float max, float value);

    // Returns the translated old value in the old scale to the corresponding value in the new scale - linear remapping
    float MathLermp(float oldVal, float minO, float maxO, float minN, float maxN);

    // Exponential decay function
    float MathDecayExp(float a, float b, float minRatio = 0.1f, float steepness = 1.0f);

    // Returns true if a roll with the given chance was true
    // Note: Chance must be between 0.0 and 1.0F - 0 is never true 1 is always true
    bool MathRoll(float chance);

    // Returns the value of val + offset wraps it around such that it stays within 0 <= row < max
    int MathCirculate(int val, int offset, int max);

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

    // Returns true if the number is even
    bool MathIsEven(int num);

    // Mirrors the given value vertically
    template <typename T>
    T MathMirror(T value, T border);

    // Applies the change to the given value such that it goes closer to 0 (or 0) but not oversteps it
    float TowardsZero(float value, float change);

    // Applies the change to the given value such that it goes towards the closest max (positive or negative) but not oversteps it
    float AwayFromZero(float value, float change, float max);

} // namespace magique


// IMPLEMENTATION


namespace magique
{
    template <typename T>
    bool MathInRange(const T& val, const T& min, const T& max)
    {
        return (val >= min) && (val <= max);
    }

    template <typename T>
    T MathMirror(T value, T border)
    {
        return value + (2 * (border - value));
    }
} // namespace magique

#endif // MAGIQUE_MATH_H
