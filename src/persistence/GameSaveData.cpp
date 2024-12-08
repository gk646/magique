// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS

#include <magique/persistence/container/GameSaveData.h>
#include <magique/util/Logging.h>

#include "internal/utils/EncryptionUtil.h"

using StorageCell = magique::internal::GameSaveStorageCell;

namespace magique
{
    constexpr auto* FILE_HEADER = "MAGIQUE_SAVE_FILE";

    GameSaveData::GameSaveData(GameSaveData&& other) noexcept : storage(std::move(other.storage)), isPersisted(other.isPersisted) {}

    GameSaveData& GameSaveData::operator=(GameSaveData&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        for (auto& cell : storage)
        {
            cell.free();
        }

        storage = std::move(other.storage);
        isPersisted = other.isPersisted;

        return *this;
    }

    GameSaveData::~GameSaveData()
    {
        if (!isPersisted && !storage.empty())
        {
            LOG_WARNING("GameSaveData is deleted without being saved!");
        }

        for (auto& cell : storage)
        {
            cell.free();
        }
    }

    StorageType GameSaveData::getStorageInfo(const StorageID id)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return StorageType::EMPTY;
        }
        return cell->type;
    }

    //----------------- PERSISTENCE -----------------//

    bool SaveToDisk(GameSaveData& save, const char* filePath, const uint64_t encryptionKey)
    {
        int totalSize = 0;
        for (const auto& cell : save.storage)
        {
            totalSize += 1;         // Type
            totalSize += 4;         // ID
            totalSize += 4;         // Size
            totalSize += cell.size; // Data
        }

        // Generate data
        auto* data = new unsigned char[totalSize];

        int idx = 0;
        for (const auto& cell : save.storage)
        {
            auto type = static_cast<uint8_t>(cell.type); // only 5 types - but type int cause its in user space
            std::memcpy(&data[idx], &type, sizeof(uint8_t));
            ++idx;
            std::memcpy(&data[idx], &cell.id, sizeof(int));
            idx += 4;
            std::memcpy(&data[idx], &cell.size, sizeof(int));
            idx += 4;
            std::memcpy(&data[idx], cell.data, cell.size);
            idx += cell.size;
        }

        SymmetricEncrypt((char*)data, totalSize, encryptionKey);

        FILE* file = fopen(filePath, "wb");
        if (file == nullptr)
        {
            LOG_ERROR("Failed to open file for writing: %s", filePath);
            delete[] data;
            return false;
        }
        setvbuf(file, nullptr, _IONBF, 0);

        // Header
        fwrite(FILE_HEADER, strlen(FILE_HEADER), 1, file);
        // Data
        fwrite(data, totalSize, 1, file);

        fclose(file);
        save.isPersisted = true;
        delete[] data;

        LOG_INFO("Successfully saved gamesave: %s | Size: %.2fkb", filePath, (float)totalSize / 1000.0F);
        return true;
    }

    bool LoadFromDisk(GameSaveData& save, const char* filePath, const uint64_t encryptionKey)
    {
        MAGIQUE_ASSERT(save.isPersisted == false, "Can only load from empty save!");
        MAGIQUE_ASSERT(save.storage.empty(), "Can only load from empty save!");

        save.isPersisted = true; // The loaded gamesave is not expected to be saved (?)
        FILE* file = fopen(filePath, "rb");
        if (file == nullptr)
        {
            LOG_WARNING("File does not exist. Will be created once you save: %s", filePath);
            return false;
        }
        auto constexpr headerSize = static_cast<int>(std::char_traits<char>::length(FILE_HEADER));

        fseek(file, 0, SEEK_END);
        const int totalSize = static_cast<int>(ftell(file)) - headerSize;
        fseek(file, 0, SEEK_SET);

        // Check header first
        char buffer[headerSize];
        fread(buffer, headerSize, 1, file);
        if (memcmp(FILE_HEADER, buffer, headerSize) != 0)
        {
            LOG_ERROR("Malformed gamesave file: %s", filePath);
            fclose(file);
            return false;
        }

        auto* data = new char[totalSize];
        fread(data, totalSize, 1, file);

        // Decrypt
        SymmetricEncrypt(data, totalSize, encryptionKey);

        int i = 0;
        while (i < totalSize)
        {
            uint8_t type = 0;
            std::memcpy(&type, &data[i], sizeof(uint8_t));
            ++i;

            int storageID = 0;
            std::memcpy(&storageID, &data[i], sizeof(int));
            i += 4;

            int size = 0;
            std::memcpy(&size, &data[i], sizeof(int));
            i += 4;

            // Sanity check
            if (size > 1'000'000 || size < 0) // Bigger than 1mb
            {
                delete[] data;
                LOG_ERROR("Failed to parse game save. Invalid entry!");
                return {};
            }

            StorageCell cell{static_cast<StorageID>(storageID)};
            cell.assign(&data[i], size, static_cast<StorageType>(type));
            save.storage.push_back(cell);
            i += size;
        }

        fclose(file);
        delete[] data;
        LOG_INFO("Successfully loaded gamesave: %s | Size: %.2fkb", filePath, static_cast<float>(totalSize) / 1000.0F);
        return true;
    }

    //----------------- SAVING -----------------//

    void GameSaveData::saveString(const StorageID id, const std::string& string)
    {
        assignDataImpl(id, string.data(), static_cast<int>(string.size()), StorageType::STRING);
    }

    void GameSaveData::saveData(const StorageID id, const void* data, const int bytes)
    {
        assignDataImpl(id, data, bytes, StorageType::DATA);
    }

    //----------------- GETTING -----------------//

    std::string GameSaveData::getStringOrElse(const StorageID id, const std::string& defaultVal)
    {
        const auto* const cell = getCell(id);
        if (cell == nullptr) [[unlikely]]
        {
            LOG_WARNING("Storage with given id does not exist!");
            return defaultVal;
        }
        if (cell->type != StorageType::STRING)
        {
            LOG_ERROR("This storage does not save a string! %d", id);
            return defaultVal;
        }
        return {cell->data, static_cast<size_t>(cell->size)};
    }

    //----------------- PRIVATE -----------------//

    StorageCell* GameSaveData::getCell(const StorageID id)
    {
        for (auto& cell : storage)
        {
            if (cell.id == id)
            {
                return &cell;
            }
        }
        return nullptr;
    }

    void GameSaveData::assignDataImpl(const StorageID id, const void* data, const int bytes, const StorageType type)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            StorageCell newCell{id};
            newCell.assign(static_cast<const char*>(data), bytes, type);
            storage.push_back(newCell);
        }
        else
        {
            cell->assign(static_cast<const char*>(data), bytes, type);
        }
    }

} // namespace magique