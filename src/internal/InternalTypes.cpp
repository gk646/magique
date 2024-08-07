#include <cstring>
#include <cmath>

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

    void GameSaveStorageCell::free()
    {
        delete[] data;
        data = nullptr;
        size = 0;
        allocatedSize = 0;
    }

    void GameSaveStorageCell::assign(const char* ptr, const int bytes)
    {
        grow(bytes);
        std::memcpy(data, ptr, bytes);
        size = bytes;
    }


} // namespace magique