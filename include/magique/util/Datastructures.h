#ifndef NODO_DATASTRUCTURES_H
#define NODO_DATASTRUCTURES_H

#include <ankerl/unordered_dense.h>
#include <magique/util/Strings.h>
#include <magique/util/Logging.h>

//===============================================
// Datastructures
//===============================================
// ................................................................................
// This is an advanced module that contains a selection of useful datastructures
// ................................................................................

namespace magique
{

    // Persisted as an array of objects with "key" and "value" values { "key" : {} , "value" : {}} in json
    template <typename K, typename V>
    using HashMap = ankerl::unordered_dense::map<K, V>;

    template <typename K, typename V, typename Hash, typename Equals>
    using HashMapEx = ankerl::unordered_dense::map<K, V, Hash, Equals>;

    template <typename K>
    using HashSet = ankerl::unordered_dense::set<K>;

    // Transparent lookup enabled - works with const char* and std::string_view
    template <typename Value>
    using StringHashMap = HashMapEx<std::string, Value, StringHashFunc, StringEqualsFunc>;

    // Like a bitflag but for enums
    // Uses the smallest possible
    // Enum MUST be a BitFlag!
    template <class E>
    struct EnumSet final
    {
        using StoreType = std::underlying_type_t<E>;

        EnumSet() = default;
        constexpr EnumSet(E data) : data_(static_cast<StoreType>(data)) {}
        EnumSet(const std::initializer_list<E>& data) : data_()
        {
            for (const auto flag : data)
            {
                setFlag(flag, true);
            }
        }

        bool operator==(const EnumSet& other) const { return data_ == other.data_; }
        EnumSet& operator=(E data)
        {
            assign(data);
            return *this;
        }
        EnumSet& operator-=(E data)
        {
            setFlag(data, false);
            return *this;
        }
        EnumSet& operator+=(E data)
        {
            setFlag(data, true);
            return *this;
        }

        E get() const noexcept { return static_cast<E>(data_); }

        bool isSet(E flag) const noexcept { return (data_ & static_cast<StoreType>(flag)) != 0; }

        void assign(E flag) { data_ = static_cast<StoreType>(flag); }

        bool empty() const noexcept { return data_ == 0; }

        void clear() noexcept { data_ = 0; }

        void setFlag(E flag, bool value) noexcept
        {
            if (value)
            {
                data_ |= static_cast<StoreType>(flag);
            }
            else
            {
                data_ &= ~static_cast<StoreType>(flag);
            }
        }

        void toggleFlag(E flag) noexcept { data_ ^= static_cast<StoreType>(flag); }

        bool all_of(const EnumSet& other) const noexcept { return all_of(other.get()); }
        bool all_of(E flag) const noexcept { return (data_ & static_cast<StoreType>(flag)) == flag; }

        bool any_of(const EnumSet& other) const noexcept { return any_of(other.get()); }
        bool any_of(E flag) const noexcept { return (data_ & static_cast<StoreType>(flag)) != 0; }

        // Compile time check(unfolding)
        template <E... Flags>
        constexpr bool any_of() const noexcept
        {
            E compositeFlag = (0 | ... | static_cast<StoreType>(Flags));
            return (data_ & compositeFlag) != 0;
        }

        // Compile time check(unfolding)
        template <E... Flags>
        constexpr bool all_of() const noexcept
        {
            E compositeFlag = (0 | ... | static_cast<StoreType>(Flags));
            return (data_ & compositeFlag) == compositeFlag;
        }

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

    // Statically sized vector (coming as inplace_vector in c++26)
    // Useful when you want to track the size but want an array as storage
    // Does not support complex types without a default constructor
    template <typename T, uint32_t capacity>
    struct StackVector final
    {
        StackVector() = default;
        StackVector(const std::initializer_list<T>& list)
        {
            for (auto& elem : list)
            {
                push_back(elem);
            }
        }

        void pop_back()
        {
            if (size_ > 0)
            {
                --size_;
            }
        }

        bool push_back(const T& elem)
        {
            if (size_ < capacity)
            {
                data[size_++] = elem;
                return true;
            }
            return false;
        }

        uint32_t size() const { return size_; }

        T& operator[](size_t index) { return data[index]; }
        const T& operator[](size_t index) const { return data[index]; }

        auto begin() const { return data.begin(); }
        auto end() const { return data.begin() + size_; }

        auto begin() { return data.begin(); }
        auto end() { return data.begin() + size_; }

    private:
        std::array<T, capacity> data;
        uint32_t size_ = 0;
    };

    // Useful if you want to map a type to a range of numbers but do not want to use a hashmap (so some sort of conversion)
    // Only instantiates up to the maximum needed range
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

        template <typename Key>
        T& operator[](Key key)
        {
            static_assert(std::is_integral_v<Key> || std::is_enum_v<Key>, "Key bust be integral");
            size_t index = static_cast<size_t>(key);
            return (*this)[index];
        }

        template <typename Key>
        const T& operator[](Key key) const
        {
            static_assert(std::is_integral_v<Key> || std::is_enum_v<Key>, "Key bust be integral");
            size_t index = static_cast<size_t>(key);
            return (*this)[index];
        }

        const T& operator[](size_t index) const { return data[index]; }

        T& operator[](size_t index)
        {
            if (data.size() < index + 1)
            {
                data.resize(index + 1);
            }
            return data[index];
        }
        void reserve(size_t size) { data.reserve(size); }

        size_t width() const { return data.size(); }
        bool empty() const { return data.empty(); }
        void clear() { data.clear(); }

    private:
        std::vector<T> data;
    };

    // This is useful for dynamically size 2D grids where for each cell you store data
    // E.g. for the fog of war for a map
    // The reduction is useful to sample the grid at a lower resolution
    //
    // First you sample from world to tile pos (worldPos / tileSize)
    // And then another /2 to make the grid smaller (reduction)
    // So you a single grid cell is 16pixels instead of 8
    // By making it a template parameter (and a power of two) you loose almost no speed!
    // All functions take the pixel value
    template <typename T, int reduction = 2>
    struct DynamicGridContainer final
    {
        DynamicGridContainer() = default;
        DynamicGridContainer(int cols, int rows) : data(cols * rows), cols(cols), rows(rows) {}

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

        const T& operator()(const Point& point) const
        {
            const int x = static_cast<int>(point.x / reduction);
            const int y = static_cast<int>(point.y / reduction);
            return data[y * cols + x];
        }

        T& operator()(const Point& point)
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

        bool insideGrid(const Point& point) const
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
        int cols = 0;
        int rows = 0;
    };

    // A faster version of std::priority_queue
    template <typename T, typename Compare = std::greater<T>, typename Allocator = std::allocator<T>>
    struct PriorityQueue final
    {
        explicit PriorityQueue(size_t len = 32) { data_.reserve(len); }

        size_t size() const { return data_.size(); }

        // Adds an element to the priority queue
        void push(const T& e) noexcept
        {
            data_.push_back(e);
            sift_up(data_.size() - 1);
        }

        // Construct a new T element at the end of the list
        template <typename... Args>
        void emplace(Args&&... args) noexcept
        {
            data_.emplace_back(std::forward<Args>(args)...);
            sift_up(data_.size() - 1);
        }

        // Removes the highest priority element from the priority queue
        void pop() noexcept
        {
            MAGIQUE_ASSERT(!data_.empty(), "no such element");
            std::swap(data_[0], data_.back());
            data_.pop_back();
            sift_down(0);
        }

        // Returns the highest priority element of the queue
        T& top() noexcept { return data_.front(); }
        const T& top() const noexcept { return data_.front(); }

        // returns true if the queue is empty
        bool empty() { return data_.empty(); }

        // Clears the queue of all elements
        void clear() { data_.clear(); }

        auto begin() { return data_.begin(); }
        auto end() { return data_.end(); }

    private:
        void sift_up(size_t index) noexcept
        {
            auto parent = (index - 1) / 2;
            while (index != 0 && !comp(data_[index], data_[parent]))
            {
                std::swap(data_[index], data_[parent]);
                index = parent;
                parent = (index - 1) / 2;
            }
        }

        void sift_down(size_t index) noexcept
        {
            auto left = (2 * index) + 1;
            auto right = (2 * index) + 2;
            const auto size = data_.size();
            while ((left < size && comp(data_[index], data_[left])) ||
                   (right < size && comp(data_[index], data_[right])))
            {
                auto smallest = (right >= size || comp(data_[right], data_[left])) ? left : right;
                std::swap(data_[index], data_[smallest]);
                index = smallest;
                left = (2 * index) + 1;
                right = (2 * index) + 2;
            }
        }

        std::vector<T, Allocator> data_;
        Compare comp;
    };


    // Useful to map a enum to a value with direct indexing
    // Just a wrapped std::array<>
    // Supports loading from JSON
    // e.g. keybinds EnumArray<PlayerAction, magique::Keybind>
    // If no explicit size is given uses Enum::COUNT (should be the last defined value)
    // Persisted as an array of objects with "key" and "value" values { "key" : {} , "value" : {}} in json
    template <class Key, typename Value, int manual_size = 0>
    struct EnumArray final
    {
        struct ValueHolder final // This is persisted and loaded from JSON
        {
            Key key;
            Value value;
            bool operator==(const ValueHolder& other) const = default;
        };

        EnumArray() { initKeys(); };

        constexpr EnumArray(const std::initializer_list<ValueHolder>& init) : EnumArray()
        {
            for (const auto& value : init)
            {
                const auto keyInt = static_cast<size_t>(value.key);
                if (keyInt < 0 || keyInt >= size())
                {
                    LOG_ERROR("Invalid key");
                    continue;
                }
                data[keyInt] = ValueHolder{value.key, value.value};
            }
            initKeys();
        }

        const Value& operator[](Key key) const { return data[static_cast<size_t>(key)].value; }
        Value& operator[](Key key) { return data[static_cast<size_t>(key)].value; }

        auto begin() { return data.begin(); }
        auto end() { return data.end(); }
        auto begin() const { return data.begin(); }
        auto end() const { return data.end(); }

        size_t size() const { return data.size(); }

        bool operator==(const EnumArray& other) const = default;

    private:
        void initKeys()
        {
            for (size_t i = 0; i < data.size(); i++)
            {
                data[i].key = (Key)i;
            }
        }
        static_assert(std::is_integral_v<Key> || std::is_enum_v<Key>, "Key has to be integral");
        std::array<ValueHolder, manual_size == 0 ? (int)Key::COUNT : manual_size> data;
    };

    // To prevent false sharing when accessed in multithread context
    template <typename T>
    struct AlignedVec final
    {
        alignas(64) std::vector<T> vec;
    };

} // namespace magique

#endif // NODO_DATASTRUCTURES_H
