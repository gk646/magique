#include <cstring>

#include <magique/internal/InternalTypes.h>

namespace magique
{
    void GameSaveStorageCell::grow(const int newSize)
    {
        if (newSize > allocatedSize)
        {
            const int newAllocatedSize = newSize;
            auto* newData = new char[newAllocatedSize];
            if (data)
            {
                std::memcpy(newData, data, size);
                delete[] data;
            }
            data = newData;
            allocatedSize = newAllocatedSize;
        }
    }

    void GameSaveStorageCell::append(const char* ptr, const int bytes)
    {
        grow(size + bytes);
        std::memcpy(data + size, ptr, bytes);
        size += bytes;
    }

    void GameSaveStorageCell::assign(const char* ptr, int bytes)
    {
        grow(bytes);
        std::memcpy(data, ptr, bytes);
        size = bytes;
    }



} // namespace magique