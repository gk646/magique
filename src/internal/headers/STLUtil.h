#ifndef STL_UTIL_H
#define STL_UTIL_H

template <typename Container, typename T>
void UnorderedDelete(Container& container, const T& value)
{
    if (container.empty())
        return;

    // Check if the container contains only one element
    if (container.size() == 1)
    {
        if (container.front() == value)
            container.clear();
        return;
    }

    // Iterate over the container to find the element
    for (auto it = container.begin(); it != container.end(); ++it)
    {
        if (*it == value)
        {
            *it = std::move(container.back());
            container.pop_back();
            return;
        }
    }
}


#endif //STL_UTIL_H