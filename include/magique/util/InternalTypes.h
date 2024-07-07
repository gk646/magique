#ifndef INTERNALTYPES_H
#define INTERNALTYPES_H

#include <iterator>
#include <magique/core/Types.h>

//-----------------------------------------------
// Public Internal Types
//-----------------------------------------------
// .....................................................................
// Dont modify these types. The have to be public due to templates (or other reasons) but are used internally
// .....................................................................

namespace magique
{
    // Array Iterator template
    template <typename U>
    class Iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = U;
        using difference_type = std::ptrdiff_t;
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
            void* string = nullptr;
        };
    };

} // namespace magique

#endif //INTERNALTYPES_H