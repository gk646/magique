#ifndef MAGIQUE_INTERNALTYPES_H
#define MAGIQUE_INTERNALTYPES_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Public Internal Module
//-----------------------------------------------
// .....................................................................
// Internal but public simple types or methods. Dont modify or use them.
// They have to be public due to templates (or other reasons) and are used internally
// .....................................................................

namespace magique::internal
{
    template <typename T>
    static constexpr size_t SizeOf()
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

        //----------------- EMISSION SHAPE -----------------//
        float emX = 0, emY = 0;
        float emp1 = 0; // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float emp2 = 0; // RECT: height                   / CAPSULE: heigth  / TRIANGLE: offsetY

        //----------------- PARTICLE -----------------//
        float p1 = 5.0F; // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float p2 = 5.0F; // RECT: height                   / CAPSULE: heigth  / TRIANGLE: offsetY
        float p3 = 0.0F; //                                                   / TRIANGLE: offsetX2
        float p4 = 0.0F; //                                                   / TRIANGLE: offsetY2

        int colors[MAGIQUE_PARTICLE_COLORPOOL_SIZE]{-1}; // Color pool
        float minScale = 1, maxScale = 1;                // Scale
        float minInitVeloc = 1, maxInitVeloc = 1;        // Initial velocity
        float dirX = 0, dirY = -1;                       // Direction
        float spreadAngle = 0;                           // Spread angle around the direction
        float gravX = 0, gravY = 0;                      // Gravity in x and y direction
        uint16_t minLife = 100, maxLife = 100;           // Total life time
        bool resolutionScaling = true;                   // resolution scaling
        Shape shape = Shape::RECT;
        Shape emShape = Shape::CAPSULE;       // Default is point emission
        uint8_t r = 255, g = 0, b = 0, a = 1; // Color
    };

} // namespace magique

#endif //MAGIQUE_INTERNALTYPES_H