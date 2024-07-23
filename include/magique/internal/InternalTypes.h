#ifndef INTERNALTYPES_H
#define INTERNALTYPES_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Public Internal Module
//-----------------------------------------------
// .....................................................................
// Dont modify these types or methods.
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
        StorageID id{};
        char* data = nullptr;
        int size = 0;
        int allocatedSize = 0;
        bool operator==(const GameSaveStorageCell& o) const { return id == o.id; }
        bool operator<(const GameSaveStorageCell& o) const { return id < o.id; }
        void assign(const char* ptr, int bytes);
        void append(const char* ptr, int bytes);
        void grow(int newSize);
    };

    struct GameConfigStorageCell final
    {
        union
        {
            Keybind keybind;
            Setting setting;
            void* string;
            unsigned char buffer[8]{};
        };
    };

    struct EmitterData final
    {
        using ScaleFunction = float (*)(float s, float t);
        using ColorFunction = float (*)(Color& c, float t);
        bool resolutionScaling = true;
        uint8_t r = 0, g = 0, b = 0, a = 1;
        Shape shape = RECT;
        Shape emissionShape = RECT;
        uint16_t amount = 1;
        uint16_t lifeTime = 1000;
        uint16_t width = 10, height = 10;
        uint16_t emWidth = 1, emHeight = 1;
        uint16_t x = 1, y = 1;
        float minScale = 1;
        float maxScale = 1;
        float minInitVeloc = 1;
        float maxInitVeloc = 1;
        float dirX = 0, dirY = 0;
        float spreadAngle = 0;
        ScaleFunction scaleFunc = nullptr;
        ColorFunction colorFunc = nullptr;
    };

    struct ParticleData final
    {
        float x, y;         // Position
        float vx, vy;       // Velocity
        uint16_t age;       // Current age
        uint16_t lifetime;  // Total lifetime
        float size;         // Current size
        uint8_t r, g, b, a; // Current color
    };


} // namespace magique

#endif //INTERNALTYPES_H