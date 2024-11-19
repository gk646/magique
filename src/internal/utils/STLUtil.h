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

template <typename T>
T minValue(const T& t1, const T& t2)
{
    if (t1 < t2)
        return t1;
    return t2;
}

template <typename T>
T maxValue(const T& t1, const T& t2)
{
    if (t1 < t2)
        return t2;
    return t1;
}

//  quickSort
// Based on public-domain C implementation by Darel Rex Finley.

template <typename T, typename Compare>
bool QuickSort(T* arr, const int elements, Compare comp)
{
    constexpr int MAX_LEVELS = 500;

    int beg[MAX_LEVELS];
    int end[MAX_LEVELS];
    int i = 0;
    int L;
    int R;

    beg[0] = 0;
    end[0] = elements;

    while (i >= 0)
    {
        L = beg[i];
        R = end[i] - 1;
        if (L < R)
        {
            T piv = arr[L];
            if (i == MAX_LEVELS - 1)
                return false;
            while (L < R)
            {
                while (!comp(arr[R], piv) && L < R)
                    R--;
                if (L < R)
                    arr[L++] = arr[R];
                while (!comp(piv, arr[L]) && L < R)
                    L++;
                if (L < R)
                    arr[R--] = arr[L];
            }
            arr[L] = piv;
            beg[i + 1] = L + 1;
            end[i + 1] = end[i];
            end[i++] = L;
        }
        else
        {
            i--;
        }
    }
    return true;
}

#endif //STL_UTIL_H