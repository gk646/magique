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


template <int size>
struct BitSet final
{
    static constexpr int BITS_PER_BYTE = 8;
    static constexpr int BYTE_COUNT = (size + BITS_PER_BYTE - 1) / BITS_PER_BYTE;

    uint8_t data[BYTE_COUNT] = {0};

    bool operator[](int bit) const
    {
        if (bit < 0 || bit >= size)
        {
            return false;
        }
        int byteIdx = bit / BITS_PER_BYTE;
        const int bitIdx = bit % BITS_PER_BYTE;
        return (data[byteIdx] & (1 << bitIdx)) != 0;
    }

    void set(int bit, bool val)
    {
        if (bit < 0 || bit >= size)
        {
            return;
        }
        int byteIdx = bit / BITS_PER_BYTE;
        int bitIdx = bit % BITS_PER_BYTE;
        data[byteIdx] |= ((val ? 1 : 0) << bitIdx);
    }

    void clear(int index)
    {
        if (index < 0 || index >= size)
        {
            return;
        }
        int byteIdx = index / BITS_PER_BYTE;
        int bitIdx = index % BITS_PER_BYTE;
        data[byteIdx] &= ~(1 << bitIdx);
    }

    void reset()
    {
        for (int i = 0; i < BYTE_COUNT; i++)
        {
            data[i] = 0;
        }
    }
};

template <class E>
struct EnumSet
{
    using StoreType = std::underlying_type_t<E>;

    EnumSet() = default;
    explicit EnumSet(E data) : data_(static_cast<StoreType>(data)) {}

    EnumSet& operator=(E data)
    {
        assign(data, true);
        return *this;
    }

    EnumSet& operator-=(E data)
    {
        unset(data);
        return *this;
    }

    EnumSet& operator+=(E data)
    {
        set(data);
        return *this;
    }

    void assign(E flag, const bool value)
    {
        if (value)
        {
            data_ = static_cast<StoreType>(flag);
        }
        else
        {
            data_ = 0;
        }
    }

    void set(E flag) noexcept { data_ |= static_cast<StoreType>(flag); }

    void unset(E flag) noexcept { data_ &= ~static_cast<StoreType>(flag); }

    void toggle(E flag) noexcept { data_ ^= static_cast<StoreType>(flag); }

    [[nodiscard]] bool isSet(E flag) const noexcept { return (data_ & static_cast<StoreType>(flag)) != 0; }

    void clear() noexcept { data_ = 0; }

    [[nodiscard]] bool any() const noexcept { return data_ != 0x0; }

    //Compile time check(unfolding)
    template <E... Flags>
    [[nodiscard]] constexpr bool any_of() const noexcept
    {
        E compositeFlag = (0 | ... | static_cast<StoreType>(Flags));
        return (data_ & compositeFlag) != 0;
    }

    //Compile time check(unfolding)
    template <E... Flags>
    [[nodiscard]] constexpr bool all_of() const noexcept
    {
        E compositeFlag = (0 | ... | static_cast<StoreType>(Flags));
        return (data_ & compositeFlag) == compositeFlag;
    }

    //Runtime check
    template <typename Iterable>
    [[nodiscard]] bool any_of(const Iterable& flags) const noexcept
    {
        for (auto flag : flags)
        {
            if ((data_ & static_cast<StoreType>(flag)) != 0)
            {
                return true;
            }
        }
        return false;
    }

    //Runtime check
    template <typename Iterable>
    [[nodiscard]] bool all_of(const Iterable& flags) const noexcept
    {
        for (auto flag : flags)
        {
            if ((data_ & static_cast<E>(flag)) == 0)
            {
                return false;
            }
        }
        return true;
    }

    E get() const noexcept { return static_cast<E>(data_); }

    void assign(const E& value) { data_ = static_cast<StoreType>(value); }

    struct EnumSetIterator
    {
        EnumSetIterator(StoreType data, StoreType newCurrent) : set(static_cast<E>(data)), current(newCurrent)
        {
            while (current != 0 && !set.isSet(static_cast<E>(current)))
            {
                current = current << 1;
            }
        }

        E operator*() const noexcept { return static_cast<E>(current); }

        EnumSetIterator& operator++() noexcept
        {
            current = current << 1;
            while (current != 0 && !set.isSet(static_cast<E>(current)))
            {
                current = current << 1;
            }
            return *this;
        }

        EnumSetIterator operator++(int) noexcept
        {
            EnumSetIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const EnumSetIterator& other) const noexcept { return current == other.current; }

        bool operator!=(const EnumSetIterator& other) const noexcept { return !(*this == other); }

    private:
        const EnumSet set;
        StoreType current;
    };

    EnumSetIterator begin() const noexcept { return EnumSetIterator(data_, 1); }

    EnumSetIterator end() const noexcept { return EnumSetIterator(data_, 0); }

private:
    StoreType data_ = 0;
};

#endif //MAGIQUE_PUBLIC_DATASTRUCTURES_H
