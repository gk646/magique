#ifndef MAGEQUEST_STL_H
#define MAGEQUEST_STL_H

#include <cfloat>
#include <vector>

//===============================================
// STL (Standard Templates Libarary)
//===============================================
// ................................................................................
// These are the simple and public types/enums used and exposed by magique
// Note: Some enum use an explicit type to save memory when used in the ECS or networking
// ................................................................................

namespace magique
{
    // Returns the max element of the vector based on the return value of pred
    // Note: calls pred only ONCE for all elements (unlike std::max_element)
    template <typename T, typename Pred>
    T* max_element(std::vector<T>& vec, Pred pred);
    template <typename T, typename Pred>
    const T* max_element(const std::vector<T>& vec, Pred pred);

} // namespace magique

// IMPLEMENTATION

namespace magique
{
    template <typename T, typename Pred>
    T* max_element(std::vector<T>& vec, Pred pred)
    {
        float highest = FLT_MIN;
        T* ret = nullptr;
        for (auto& elem : vec)
        {
            const float val = pred(elem);
            if (val > highest)
            {
                ret = &elem;
                highest = val;
            }
        }
        return ret;
    }

    template <typename T, typename Pred>
    const T* max_element(const std::vector<T>& vec, Pred pred)
    {
        float highest = FLT_MIN;
        const T* ret = nullptr;
        for (const auto& elem : vec)
        {
            const float val = pred(elem);
            if (val > highest)
            {
                ret = &elem;
                highest = val;
            }
        }
        return ret;
    }



} // namespace magique

#endif // MAGEQUEST_STL_H
