#ifndef MAGEQUEST_STL_H
#define MAGEQUEST_STL_H

#include <algorithm>
#include <cfloat>
#include <span>
#include <vector>
#include <raylib/raylib.h>

//===============================================
// STL (Standard Templates Library)
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

    // Randomly picks n unique elements from the given range and returns them
    // Note: vec MUST not contain duplicates
    template <typename T, std::size_t Extent = std::dynamic_extent>
    std::vector<std::remove_cv_t<T>> pick_unique_rand(std::span<T, Extent> vec, size_t n);

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

    template <typename T, std::size_t Extent>
    std::vector<std::remove_cv_t<T>> pick_unique_rand(std::span<T, Extent> vec, size_t n)
    {
        using ValueType = std::remove_cv_t<T>;
        std::vector<ValueType> ret;
        ret.reserve(n);

        if (n == 0) [[unlikely]]
            return ret;

        if (vec.size() <= n)
            return std::vector<ValueType>(vec.begin(), vec.end());

        while (ret.size() < n)
        {
            auto index = GetRandomValue(0, vec.size() - 1);
            if (std::ranges::contains(ret, vec[index]))
                continue;
            ret.push_back(vec[index]);
        }

        return ret;
    }

} // namespace magique

#endif // MAGEQUEST_STL_H
