#ifndef MAGIQUE_PUBLIC_DATASTRUCTURES_H
#define MAGIQUE_PUBLIC_DATASTRUCTURES_H

#include <vector>
#include <array>

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

    T& operator[](size_t index)
    {
        if (data.size() < index + 1)
        {
            data.resize(index + 1);
        }
        return data[index];
    }

    const T& operator[](size_t index) const { return data[index]; }

    void reserve(size_t size) { data.reserve(size); }

    size_t width() { return data.size(); }

private:
    std::vector<T> data;
};

template <typename T, uint32_t capacity>
struct StackVector final
{
    void pop_back()
    {
        if (size_ > 0)
        {
            --size_;
        }
    }

    void push_back(const T& elem)
    {
        if (size_ < capacity)
        {
            data[size_++] = elem;
        }
    }

    uint32_t size() const { return size_; }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }

private:
    static_assert(std::is_trivially_constructible_v<T>, "Must be trivial");
    static_assert(std::is_trivially_destructible_v<T>, "Must be trivial");
    std::array<T, capacity> data;
    uint32_t size_ = 0;
};

#endif //MAGIQUE_PUBLIC_DATASTRUCTURES_H