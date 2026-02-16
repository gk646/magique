// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_INTERNAL_TYPES_H
#define MAGIQUE_INTERNAL_TYPES_H

#include <magique/core/Types.h>
#include <magique/internal/Macros.h>

//===============================================
// Public Internal Module
//===============================================
// .....................................................................
// Internal but public simple types or methods. Don't modify or use them (unless you know what your doing).
// They have to be public due to templates (or other reasons) and are used internally
// .....................................................................

namespace magique::internal
{
    template <typename T>
    static constexpr uint64_t SizeOf()
    {
        return sizeof(T);
    }

    // Stores information used in the GameSaveData
    struct GameSaveStorageCell final
    {
        char* data = nullptr;
        StorageID id;
        int size = 0;
        int allocatedSize = 0;
        StorageType type = StorageType::EMPTY;
        explicit GameSaveStorageCell(const StorageID id) : id(id) {}
        void free();
        void assign(const char* ptr, int bytes, StorageType type);
        void grow(int newSize);
    };

    // Stores information for the game config
    struct GameConfigStorageCell final
    {
        ConfigID id;
        StorageType type = StorageType::EMPTY;
        union
        {
            Keybind keybind;
            char* string = nullptr;
            char buffer[8];
        };
        explicit GameConfigStorageCell(const ConfigID id) : id(id) {}
        void assign(const char* data, int size, StorageType type, Keybind bind = Keybind{});
    };

    struct EmitterData final
    {
        using ScaleFunction = float (*)(float s, float t);
        using ColorFunction = Color (*)(const Color& c, float t);
        ScaleFunction scaleFunc = nullptr;
        ColorFunction colorFunc = nullptr;
        void* tickFunc = nullptr;

        //================= EMISSION SHAPE =================//
        mutable Point emissionPos{0, 0}; // RECT: width/height  / CIRCLE: radius
        Point emissionDims{1, 1};        // RECT: width/height  / CIRCLE: radius
        Point anchor;

        //================= PARTICLE =================//
        float p1 = 5.0F; // RECT: width  / CIRCLE: radius  / CAPSULE: radius
        float p2 = 5.0F; // RECT: height                   / CAPSULE: height

        int colors[MAGIQUE_PARTICLE_COLORPOOL_SIZE]{}; // Color pool
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
