#ifndef MAGIQUE_PUBLIC_DATASTRUCTURES_H
#define MAGIQUE_PUBLIC_DATASTRUCTURES_H

#include <vector>

template <typename T>
struct SparseRangeVector final
{
    void set(size_t index, const T& value)
    {
        if (data.size() < index + 1)
        {
            data.resize(index + 1);
        }
        data[index] = value;
    }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    void reserve(size_t size) { data.reserve(size); }

    size_t width() { return data.size(); }

private:
    std::vector<T> data;
};

#endif //MAGIQUE_PUBLIC_DATASTRUCTURES_H