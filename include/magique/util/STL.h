#ifndef MAGEQUEST_STL_H
#define MAGEQUEST_STL_H

#include <cfloat>
#include <string_view>
#include <vector>

namespace magique
{

    // Uses a static std::string to assign and returns the pointer to it
    // Note: Only valid until this method is called again
    const char* ToCStr(const std::string_view& view);

    // Returns the max element of the vector based on the return value of pred
    // Note: calls pred only ONCE for all elements (unlike std::max_element)
    template <typename T, typename Pred>
    T* MaxElement(std::vector<T>& vec, Pred pred);
    template <typename T, typename Pred>
    const T* MaxElement(const std::vector<T>& vec, Pred pred);

} // namespace magique


// IMPLEMENTATION


namespace magique
{

    template <typename T, typename Pred>
    T* MaxElement(std::vector<T>& vec, Pred pred)
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
    const T* MaxElement(const std::vector<T>& vec, Pred pred)
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

#endif //MAGEQUEST_STL_H
