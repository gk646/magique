// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>

#include <magique/internal/InternalTypes.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

namespace magique::internal
{
    void GameSaveStorageCell::grow(const int newSize)
    {
        if (newSize > allocatedSize)
        {
            const int newAllocatedSize = newSize;
            auto* newData = new char[newAllocatedSize];
            if (data != nullptr)
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
        type = StorageType::EMPTY;
    }

    void GameSaveStorageCell::assign(const char* ptr, const int bytes, const StorageType newType)
    {
        grow(bytes);
        std::memcpy(data, ptr, bytes);
        size = bytes;
        type = newType;
    }

    void GameConfigStorageCell::assign(const char* data, const int size, const StorageType newType, const Keybind bind)
    {
        if (type == StorageType::STRING) // If it was a string delete it
        {
            delete[] string;
            string = nullptr;
        }

        if (newType == StorageType::STRING)
        {
            string = new char[size + 1];
            std::memcpy(string, data, size);
            string[size] = '\0';
        }
        else if (newType == StorageType::KEY_BIND)
        {
            keybind = bind;
        }
        else if (newType == StorageType::VALUE)
        {
            MAGIQUE_ASSERT(size <= 8, "Cannot save value bigger than 8 bytes!");
            std::memcpy(buffer, data, size);
        }
        else
        {
            LOG_ERROR("Passed wrong type!");
        }
        type = newType;
    }

} // namespace magique::internal