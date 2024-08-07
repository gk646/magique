#ifndef INTERNALTYPES_H
#define INTERNALTYPES_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Public Internal Module
//-----------------------------------------------
// .....................................................................
// Internal but public simple types or methods. Dont modify them!.
// They have to be public due to templates (or other reasons) and are used internally
// .....................................................................

namespace magique
{
    // Array Iterator template
    template <typename U>
    class Iterator
    {
    public:
        using value_type = U;
        using difference_type = int64_t;
        using pointer = U*;
        using reference = U&;

        explicit Iterator(pointer ptr) : ptr_(ptr) {}

        reference operator*() const { return *ptr_; }

        pointer operator->() { return ptr_; }

        Iterator& operator++()
        {
            ++ptr_;
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++ptr_;
            return tmp;
        }

        Iterator& operator--()
        {
            --ptr_;
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --ptr_;
            return tmp;
        }

        Iterator& operator+=(difference_type offset)
        {
            ptr_ += offset;
            return *this;
        }

        Iterator operator+(difference_type offset) const { return Iterator(ptr_ + offset); }

        Iterator& operator-=(difference_type offset)
        {
            ptr_ -= offset;
            return *this;
        }

        Iterator operator-(difference_type offset) const { return Iterator(ptr_ - offset); }

        difference_type operator-(const Iterator& other) const { return ptr_ - other.ptr_; }

        reference operator[](difference_type index) const { return ptr_[index]; }

        bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }

        bool operator!=(const Iterator& other) const { return ptr_ != other.ptr_; }

        bool operator<(const Iterator& other) const { return ptr_ < other.ptr_; }

        bool operator>(const Iterator& other) const { return ptr_ > other.ptr_; }

        bool operator<=(const Iterator& other) const { return ptr_ <= other.ptr_; }

        bool operator>=(const Iterator& other) const { return ptr_ >= other.ptr_; }

    private:
        pointer ptr_;
    };

    template <typename T>
    static constexpr size_t SizeOf()
    {
        return sizeof(T);
    }

    // Stores information used in the GameSave
    struct GameSaveStorageCell final
    {
        char* data = nullptr;
        StorageID id{};
        int size = 0;
        int allocatedSize = 0;
        StorageType type = StorageType::EMPTY;
        void free();
        void assign(const char* ptr, int bytes);
        void grow(int newSize);
    };

    // Storest information for the game config
    struct GameConfigStorageCell final
    {
        union
        {
            int stringIndex = 0;
            Keybind keybind;
            char buffer[8];
        };
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

        int colors[6]{-1};                        // Color pool
        float minScale = 1, maxScale = 1;         // Scale
        float minInitVeloc = 1, maxInitVeloc = 1; // Initial velocity
        float dirX = 0, dirY = -1;                // Direction
        float spreadAngle = 0;                    // Spread angle around the direction
        float gravX = 0, gravY = 0;               // Gravity in x and y direction
        uint16_t minLife = 100, maxLife = 100;    // Total life time
        bool resolutionScaling = true;            // resolution scaling
        Shape shape = Shape::RECT;
        Shape emShape = Shape::CAPSULE;       // Default is point emission
        uint8_t r = 255, g = 0, b = 0, a = 1; // Color
    };


} // namespace magique

#endif //INTERNALTYPES_H