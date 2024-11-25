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

    // Stores information used in the GameSave
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
        void assign(const char* data, int size, StorageType type, Keybind bind = Keybind{0});
    };

    struct EmitterData final
    {
        using ScaleFunction = float (*)(float s, float t);
        using ColorFunction = Color (*)(const Color& c, float t);
        ScaleFunction scaleFunc = nullptr;
        ColorFunction colorFunc = nullptr;
        void* tickFunc = nullptr;

        //================= EMISSION SHAPE =================//
        float emX = 0, emY = 0;
        float emp1 = 0; // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float emp2 = 0; // RECT: height                   / CAPSULE: height  / TRIANGLE: offsetY

        //================= PARTICLE =================//
        float p1 = 5.0F; // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float p2 = 5.0F; // RECT: height                   / CAPSULE: height  / TRIANGLE: offsetY
        float p3 = 0.0F; //                                                   / TRIANGLE: offsetX2
        float p4 = 0.0F; //                                                   / TRIANGLE: offsetY2

        int colors[MAGIQUE_PARTICLE_COLORPOOL_SIZE]{-1}; // Color pool
        float minScale = 1, maxScale = 1;                // Scale
        float minInitVeloc = 1, maxInitVeloc = 1;        // Initial velocity
        float dirX = 0, dirY = -1;                       // Direction
        float spreadAngle = 0;                           // Spread angle around the direction
        float gravX = 0, gravY = 0;                      // Gravity in x and y direction
        uint16_t minLife = 100, maxLife = 100;           // Total lifetime
        bool resolutionScaling = true;                   // resolution scaling
        Shape shape = Shape::RECT;
        Shape emShape = Shape::CAPSULE;       // Default is point emission
        uint8_t r = 255, g = 0, b = 0, a = 1; // Color
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

    struct ThreatTableEntry final
    {
        entt::entity target;
        float threat;
    };

    struct UIContainerMapping final
    {
        char name[MAGIQUE_MAX_NAMES_LENGTH]{};
        int index = 0;
    };

    struct WindowManagerMapping final
    {
        char name[MAGIQUE_MAX_NAMES_LENGTH]{};
        Window* window = nullptr;
    };

    struct ParameterData final
    {
        ParameterType types[3]{}; // Allowed types
        char* name = nullptr;     // Name
        bool optional = false;    // If param is optional
        bool variadic = false;    // Stands for a variable amount of parameters
        union
        {
            float number;
            bool boolean;
            char* string;
        };
    };


} // namespace magique::internal

#endif //MAGIQUE_INTERNAL_TYPES_H