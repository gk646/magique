#ifndef MAGIQUE_PUBLIC_DATASTRUCTURES_H
#define MAGIQUE_PUBLIC_DATASTRUCTURES_H

#include <vector>
#include <array>
#include <magique/core/Types.h>

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

    size_t width() const { return data.size(); }

    void clear() { data.clear(); }

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
    std::array<T, capacity> data;
    uint32_t size_ = 0;
};


// This is useful for dynamically size 2D grids
// The reduction is useful to sample the grid at a lower resolution
// E.g. for the fog of war for a map
// 500x500 tiles
// But full resolution is too big (you also need to save the progress)
// So first you have to sample from world to tile pos (worldPos / tileSize)
// And then another /2 to make the grid smaller
// So you a single grid cell is 16pixels instead of 8
// By making it a template parameter (and a power of two) you loose almost no speed!
template <typename T, int reduction = 1>
struct DynamicSimpleGrid final
{
    DynamicSimpleGrid() = default;
    DynamicSimpleGrid(int cols, int rows) : data(cols * rows), cols(cols), rows(rows) {}

    T& operator()(int x, int y)
    {
        x /= reduction;
        y /= reduction;
        return data[y * cols + x];
    }

    const T& operator()(int x, int y) const
    {
        x /= reduction;
        y /= reduction;
        int index = y * cols + x;
        return data[index];
    }

    const T& operator()(const magique::Point& point) const
    {
        const int x = static_cast<int>(point.x / reduction);
        const int y = static_cast<int>(point.y / reduction);
        return data[y * cols + x];
    }

    T& operator()(const magique::Point& point)
    {
        const int x = static_cast<int>(point.x / reduction);
        const int y = static_cast<int>(point.y / reduction);
        return data[y * cols + x];
    }

    int getCols() const { return cols; }
    int getRows() const { return rows; }

    const std::vector<T>& getData() const { return data; }
    std::vector<T>& getData() { return data; }

    void insert(int x, int y, int width, int height, const T& elem)
    {
        x /= reduction;
        y /= reduction;
        width /= reduction;
        height /= reduction;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                const int xCo = j + x;
                const int yCo = i + y;
                if (xCo >= 0 && yCo >= 0 && xCo < cols && yCo < rows)
                {
                    int idx = yCo * cols + xCo;
                    data[idx] = elem;
                }
            }
        }
    }

    bool insideGrid(int x, int y) const
    {
        x /= reduction;
        y /= reduction;
        return x >= 0 && y >= 0 && x < cols && y < rows;
    }

    bool insideGrid(const magique::Point& point) const
    {
        const int x = static_cast<int>(point.x / reduction);
        const int y = static_cast<int>(point.y / reduction);
        return x >= 0 && y >= 0 && x < cols && y < rows;
    }

    void resize(int nCols, int nRows, const T& elem = {})
    {
        cols = nCols;
        rows = nRows;
        data.resize(cols * nRows, elem);
    }

private:
    std::vector<T> data;
    int cols, rows;
};

#endif //MAGIQUE_PUBLIC_DATASTRUCTURES_H
