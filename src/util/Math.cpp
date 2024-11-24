#include <magique/internal/Macros.h>
#include <magique/util/Math.h>

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
        return (num - static_cast<float>(static_cast<int>(num))) < EPSILON;
    }


} // namespace magique