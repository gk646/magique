#ifndef STL_UTIL_H
#define STL_UTIL_H


template <typename Container, typename T>
void UnorderedDelete(Container& container, const T& value)
{
    if (container.empty())
        return;

    if (container.size() == 1)
    {
        if (container.front() == value)
            container.clear();
        return;
    }

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

template <typename Container, typename T, typename Pred>
void UnorderedDelete(Container& container, const T& value, Pred pred)
{
    if (container.empty())
        return;

    if (container.size() == 1)
    {
        if (pred(container.front(), value))
            container.clear();
        return;
    }

    for (auto it = container.begin(); it != container.end(); ++it)
    {
        if (pred(*it, value))
        {
            *it = std::move(container.back());
            container.pop_back();
            return;
        }
    }
}

#endif //STL_UTIL_H