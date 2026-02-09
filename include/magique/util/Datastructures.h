#ifndef NODO_DATASTRUCTURES_H
#define NODO_DATASTRUCTURES_H

#include <span>
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

        explicit EnumSet(E data) : data_(static_cast<StoreType>(data)) {}

        explicit EnumSet(const std::span<E>& data) : data_()
        {
            for (const auto flag : data)
            {
                set(flag);
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
            unset(data);
            return *this;
        }

        EnumSet& operator+=(E data)
        {
            set(data);
            return *this;
        }

        E get() const noexcept { return static_cast<E>(data_); }

        bool isSet(E flag) const noexcept { return (data_ & static_cast<StoreType>(flag)) != 0; }

        void assign(E flag) { data_ = static_cast<StoreType>(flag); }

        bool empty() const noexcept { return data_ == 0; }

        void clear() noexcept { data_ = 0; }

        void set(E flag) noexcept { data_ |= static_cast<StoreType>(flag); }

        void unset(E flag) noexcept { data_ &= ~static_cast<StoreType>(flag); }

        void toggle(E flag) noexcept { data_ ^= static_cast<StoreType>(flag); }

        // Compile time check(unfolding)
        template <E... Flags>
        [[nodiscard]] constexpr bool any_of() const noexcept
        {
            E compositeFlag = (0 | ... | static_cast<StoreType>(Flags));
            return (data_ & compositeFlag) != 0;
        }

        // Compile time check(unfolding)
        template <E... Flags>
        [[nodiscard]] constexpr bool all_of() const noexcept
        {
            E compositeFlag = (0 | ... | static_cast<StoreType>(Flags));
            return (data_ & compositeFlag) == compositeFlag;
        }

        // Runtime check
        [[nodiscard]] bool any_of(const std::span<E>& flags) const noexcept
        {
            for (auto flag : flags)
            {
                if (isSet(flag))
                {
                    return true;
                }
            }
            return false;
        }

        // Runtime check
        [[nodiscard]] bool all_of(const std::span<E>& flags) const noexcept
        {
            for (const auto flag : flags)
            {
                if (!isSet(flag))
                {
                    return false;
                }
            }
            return true;
        }
        [[nodiscard]] bool all_of(E flag) const noexcept { return (data_ & static_cast<StoreType>(flag)) == flag; }

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
    // Only instantiate up to the maximum needed range
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

        [[nodiscard]] bool insideGrid(int x, int y) const
        {
            x /= reduction;
            y /= reduction;
            return x >= 0 && y >= 0 && x < cols && y < rows;
        }

        [[nodiscard]] bool insideGrid(const Point& point) const
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

    // Useful when saving data that is uniquely identified by a enum value (or integral)
    // But it's not sure if all values exists (if every value is set use EnumValueHolder)
    // e.g.
    template <typename Key, typename Value>
    struct EnumVector final
    {
        struct ValueHolder final
        {
            Key key;
            Value value;
        };

        using T = ValueHolder;

        EnumVector() = default;
        EnumVector(const std::vector<T>& data) : data_(data) {}
        EnumVector(std::vector<T>&& data) : data_(std::move(data)) {}

        std::vector<T>& data() { return data_; }
        const std::vector<T>& data() const { return data_; }

        // Returns first matching value from the given container of keys otherwise "elseVal"
        template <typename Iterable>
        Value getOrElse(const Iterable& container, Value elseVal = {}) const
        {
            for (const auto& key : container)
            {
                auto* value = getImpl(key);
                if (value != nullptr)
                {
                    return value;
                }
            }
            return elseVal;
        }

        Value getOrElse(const Key& key, Value elseVal = {}) const
        {
            auto* value = getImpl(key);
            if (value != nullptr)
            {
                return *value;
            }
            return elseVal;
        }

        Value& operator[](const Key& key)
        {
            auto* value = getImpl(key);
            if (value != nullptr) [[likely]]
            {
                return *value;
            }
            LOG_FATAL("No such value: %d", (int)key);
            return data_.front().value;
        }

        const Value& operator[](const Key& key) const
        {
            auto* value = getImpl(key);
            if (value != nullptr) [[likely]]
            {
                return *value;
            }
            LOG_FATAL("No such value: %d", (int)key);
            return data_.front().value;
        }

        bool hasKey(const Key& key) const { return getImpl(key) != nullptr; }

        void add(Key key, const Value& value)
        {
            if (data_.size() <= (int)key)
            {
                data_.resize((int)key + 1);
            }
            auto& elem = data_[static_cast<int>(key)];
            elem.value = value;
            elem.key = key;
        }

        void add(Key key, Value&& value)
        {
            if (data_.size() <= (int)key)
            {
                data_.resize((int)key + 1);
            }
            auto& elem = data_[static_cast<int>(key)];
            elem.value = std::move(value);
            elem.key = key;
        }

    private:
        Value* getImpl(const Key& key) const
        {
            for (auto& entry : data_)
            {
                if (entry.key == key)
                    return &entry.value;
            }
            return nullptr;
        }

        std::vector<T> data_;
        friend glz::meta<EnumVector>;
        static_assert(std::is_integral_v<Key> || std::is_enum_v<Key>, "Key has to be integral");
    };

    // Useful to map a enum to a value with direct indexing
    // Just a wrapped std::array<>
    // Supports loading from JSON
    // e.g. keybinds EnumArray<PlayerAction, magique::Keybind>
    // If no explicit size is given uses Enum::COUNT (should be the last defined value)
    template <class Key, typename Value, int max_size = 0>
    struct EnumArray final
    {
        static constexpr int size = max_size == 0 ? (int)Key::COUNT : max_size;

        struct ValueHolder final
        {
            Key key;
            Value value;
        };

        EnumArray() = default;

        constexpr EnumArray(const std::initializer_list<ValueHolder>& init)
        {
            for (const auto& value : init)
            {
                int keyInt = static_cast<int>(value.key);
                if (keyInt < 0 || keyInt >= size)
                {
                    LOG_ERROR("Invalid key");
                    continue;
                }
                data_[keyInt] = value.value;
            }
        }

        explicit EnumArray(const EnumVector<Key, Value>& init)
        {
            for (auto& [key, val] : init.data())
            {
                (*this)[key] = std::move(val);
            }
            if (init.size() < size)
            {
                LOG_WARNING("Not all values assigned");
            }
        }

        const Value& operator[](Key key) const { return data_[static_cast<int>(key)]; }

        Value& operator[](Key key) { return data_[static_cast<int>(key)]; }

        std::array<Value, size>& data() { return data_; }
        const std::array<Value, size>& data() const { return data_; }

    private:
        static_assert(std::is_integral_v<Key> || std::is_enum_v<Key>, "Key has to be integral");
        std::array<Value, size> data_;
    };

    // To prevent false sharing when accessed in multithread context
    template <typename T>
    struct AlignedVec final
    {
        alignas(64) std::vector<T> vec;
    };

} // namespace magique

#endif // NODO_DATASTRUCTURES_H
