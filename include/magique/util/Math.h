#ifndef MAGIQUE_MATH_H
#define MAGIQUE_MATH_H

//-------------------------------
// Math Module
//-------------------------------
// ................................................................................
// This module contains helpful math functions related to game development
// ................................................................................

namespace magique
{
    // Returns the value at the given step (0 to 1) on the scale between min and max - linear interpolation
    //      - step: value between 0 and 1 (inclusive) of where on the scale the value is
    float Lerp(float min, float max, float step);

    // Returns from 0 to 1 how close the given value is to max starting from min
    float LerpInverse(float min, float max, float value);

    // Returns the translated old value in the old scale to the corresponding value in the new scale - linear remapping
    float Lermp(float oldVal, float minO, float maxO, float minN, float maxN);

    // True if the given "val" is within the specified range (included) - only uses 'operator<'
    template <typename T>
    bool IsValueInRange(T val, T min, T max);

    // Returns true if the given number is a power of two
    bool IsPowerOfTwo(unsigned int x);

    // Finds the closest power of two to the right of the given number
    unsigned int GetNextPowerOfTwo(unsigned int n);

} // namespace magique


//----------------- IMPLEMENTATION -----------------//

namespace magique
{
    template <typename T>
    bool IsValueInRange(T val, T min, T max)
    {
        return min <= val && val <= max;
    }
} // namespace magique

#endif //MAGIQUE_MATH_H