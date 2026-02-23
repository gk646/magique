// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_INTERNAL_TYPES_H
#define MAGIQUE_INTERNAL_TYPES_H

#include <cstring>
#include <vector>
#include <magique/core/Types.h>

//===============================================
// Public Internal Module
//===============================================
// .....................................................................
// Internal but public simple types or methods. Don't modify or use them (unless you know what your doing).
// They have to be public due to templates (or other reasons) and are used internally
// .....................................................................

namespace magique::internal
{
    struct StorageCell final
    {
        StorageType type = StorageType::EMPTY;
        std::string name;
        std::string data;
    };

    struct StorageContainer
    {
        StorageContainer() = default;
        StorageContainer(const StorageContainer& other) = delete; // Involves potentially copying a lot of data
        StorageContainer&
        operator=(const StorageContainer& other) = delete; // Involves potentially copying a lot of data
        StorageContainer(StorageContainer&& other) noexcept = default;
        StorageContainer& operator=(StorageContainer&& other) noexcept = default;
        ~StorageContainer(); // Will clean itself up automatically

    protected:
        M_MAKE_PUB()
        static bool ToFile(StorageContainer& container, std::string_view path, std::string_view name, uint64_t key);
        static bool FromFile(StorageContainer& container, std::string_view path, std::string_view name, uint64_t key);

        // Erases the storage with the given id
        void eraseImpl(std::string_view slot);
        // Erases all storage slots
        void clearImpl() { cells.clear(); }

        StorageType getSlotTypeImpl(std::string_view slot)
        {
            const auto* cell = getCell(slot);
            if (cell == nullptr)
            {
                return StorageType::EMPTY;
            }
            return cell->type;
        }

        StorageCell* getCell(std::string_view slot)
        {
            for (auto& cell : cells)
            {
                if (cell.name == slot)
                {
                    return &cell;
                }
            }
            return nullptr;
        }

        StorageCell& getCellOrNew(std::string_view slot, StorageType type)
        {
            auto* cell = getCell(slot);
            if (cell == nullptr)
            {
                cell = &cells.emplace_back(type);
                cell->name = slot;
                return *cell;
            }
            cell->type = type;
            return *cell;
        }

        void assignDataImpl(std::string_view slot, const void* data, const int bytes, const StorageType type)
        {
            auto& cell = getCellOrNew(slot, type);
            cell.data.resize(bytes);
            std::memcpy(cell.data.data(), data, bytes);
        }

        std::vector<StorageCell> cells;
        bool isLoaded = false;
        bool isSaved = false;
    };

    struct EmitterData final
    {
        using ScaleFunction = float (*)(float s, float t);
        using ColorFunction = Color (*)(const Color& c, float t);
        ScaleFunction scaleFunc = nullptr;
        ColorFunction colorFunc = nullptr;
        void* tickFunc = nullptr;
        mutable Point emissionPos{0, 0}; // RECT: width/height  / CIRCLE: radius
        Point emissionDims{1, 1};        // RECT: width/height  / CIRCLE: radius
        Point emissionAnchor;
        Point particleDims;
        Color colors[MAGIQUE_PARTICLE_COLORPOOL_SIZE]{}; // Color pool
        float minScale = 1, maxScale = 1;              // Scale
        float minInitVeloc = 1, maxInitVeloc = 1;      // Initial velocity
        Point direction{};                             // Direction
        float rotation = 0;                            // Rotation around the anchor
        float volume = 1.0F;                           // How much of the body will be treated as spawnable area
        float spreadAngle = 0;                         // Spread angle around the direction
        Point gravity;                                 // Gravity in x and y direction
        float angularGravity = 0.0F;
        Point angularVelocity{0, 0};
        uint16_t minLife = 100, maxLife = 100; // Total lifetime
        uint8_t poolSize = 0;
        Shape shape = Shape::RECT;
        Shape emShape = Shape::TRIANGLE; // Default is point emission
        Color color;
    };

    enum class ShareCodePropertyType : uint8_t
    {
        INTEGER,
        FLOATING,
        STRING,
    };

    struct ShareCodeProperty final
    {
        char* name = nullptr; // Always terminated and allocated string
        int bits = 0;
        ShareCodePropertyType type = ShareCodePropertyType::INTEGER;
    };

    struct ShareCodePropertyData final
    {
        const char* name = nullptr;
        union
        {
            char* string = nullptr;
            float floating;
            int32_t integer;
        };
        int bits = 0;
        ShareCodePropertyType type = ShareCodePropertyType::INTEGER;
    };

    template <typename T, int rows, int columns>
    struct StaticMatrix final
    {
        using CellFunctor = T (*)(T, int row, int column);

        T data[rows * columns];
        T operator()(const int row, const int column) const
        {
            MAGIQUE_ASSERT(row < rows && column < columns, "Out of bounds");
            return data[row * columns + column];
        }
        T& operator()(const int row, const int column)
        {
            MAGIQUE_ASSERT(row < rows && column < columns, "Out of bounds");
            return data[row * columns + column];
        }
        T sumRow(const int row) const
        {
            T sum{};
            const auto* start = &data[row * columns];
            for (int i = 0; i < columns; ++i)
            {
                sum += start[i];
            }
            return sum;
        }
        T sumColumn(const int column) const
        {
            T sum{};
            for (int i = 0; i < rows; ++i)
            {
                sum += data[i * columns + column];
            }
            return sum;
        }
        T getRowMax(const int row)
        {
            const auto* start = &data[row * columns];
            T max = start[0];
            for (int i = 0; i < columns; ++i)
            {
                if (start[i] > max)
                    max = start[i];
            }
            return max;
        }
        T getColumnMax(const int column)
        {
            T max = data[0 * columns + column];
            for (int i = 0; i < rows; ++i)
            {
                const auto val = data[i * columns + column];
                if (val > max)
                    max = val;
            }
            return max;
        }
        int getRowMaxIndex(const int row)
        {
            const auto* start = &data[row * columns];
            T max = start[0];
            int index = 0;
            for (int i = 0; i < columns; ++i)
            {
                if (start[i] > max)
                {
                    max = start[i];
                    index = i;
                }
            }
            return index;
        }
        int getColumnMaxIndex(const int column)
        {
            T max = data[0 * columns + column];
            int index = 0;
            for (int i = 0; i < rows; ++i)
            {
                const auto val = data[i * columns + column];
                if (val > max)
                {
                    max = val;
                    index = i;
                }
            }
            return index;
        }

        void forEach(const CellFunctor functor)
        {
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < columns; ++j)
                {
                    data[i * columns + j] = functor(data[i * columns + j]);
                }
            }
        }
    };

    template <class StateEnum, class EventEnum>
    struct AgentMemory final
    {
        StateEnum state;
        EventEnum causingEvent;
        EventEnum nextEvent;
        float reward = 0;

        [[nodiscard]] int getNextEvent() const { return static_cast<int>(nextEvent); }
        [[nodiscard]] int getCausingEvent() const { return static_cast<int>(causingEvent); }
        [[nodiscard]] int getState() const { return static_cast<int>(state); }
    };

    // Note: You cannot remove elements
    template <typename T>
    struct CircularBuffer final
    {
        T* data;
        int capacity;
        int size = 0;

        explicit CircularBuffer(const int capacity) : data(new T[capacity]), capacity(capacity) {}
        ~CircularBuffer() { delete[] data; }
        void add(const T& value) { data[size++ % capacity] = value; }
        template <typename Functor>
        void forLastN(const int n, const Functor& functor)
        {
            const int elementsToIterate = n < size ? n : size;
            for (int i = 0; i < elementsToIterate; ++i)
            {
                int index = (size - 1 - i + capacity) % capacity;
                functor(data[index]);
            }
        }
    };

    struct SceneManagerMapping final
    {
        std::string name;
        UIObject* object = nullptr;
    };

    struct MessageCount
    {
        MessageType type;
        uint32_t count;
    };

    struct MultiplayerStatsData
    {
        std::array<MessageCount, UINT8_MAX> incoming;
        std::array<MessageCount, UINT8_MAX> outgoing;
        uint32_t bytesIn;
        uint32_t bytesOut;
    };


} // namespace magique::internal

#endif // MAGIQUE_INTERNAL_TYPES_H
