// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>

#include <magique/util/Logging.h>
#include <magique/persistence/container/GameConfig.h>

#include "internal/utils/EncryptionUtil.h"

static constexpr auto FILE_HEADER = "CONFIG";

namespace magique
{
    void GameConfig::SaveToFile(const GameConfig& config, const char* filePath, const uint64_t key)
    {
        int totalSize = 0;
        for (const auto& cell : config.storage)
        {
            totalSize += 1; // Type
            totalSize += 4; // ID
            if (cell.type == StorageType::STRING)
            {
                totalSize += static_cast<int>(strlen(cell.string)); // String
            }
            else
            {
                totalSize += 8; // Data
            }
        }

        // Generate data
        auto* data = new unsigned char[totalSize];

        int idx = 0;
        for (const auto& cell : config.storage)
        {
            auto type = static_cast<uint8_t>(cell.type); // only 5 types - but type int cause its in user space
            memcpy(&data[idx], &type, sizeof(uint8_t));
            ++idx;
            memcpy(&data[idx], &cell.id, sizeof(int));
            idx += 4;

            if (cell.type == StorageType::STRING)
            {
                const auto len = strlen(cell.string); // String
                memcpy(&data[idx], cell.string, len);
                idx += static_cast<int>(len);
            }
            else
            {
                memcpy(&data[idx], cell.buffer, 8);
                idx += 8;
            }
        }

        SymmetricEncrypt(reinterpret_cast<char*>(data), totalSize, key);

        FILE* file = fopen(filePath, "wb");
        if (file == nullptr)
        {
            LOG_ERROR("Failed to open file for writing: %s", filePath);
            delete[] data;
            return;
        }
        setvbuf(file, nullptr, _IONBF, 0);

        // Header
        fwrite(FILE_HEADER, strlen(FILE_HEADER), 1, file);
        // Data
        fwrite(data, totalSize, 1, file);

        fclose(file);
        delete[] data;

        LOG_INFO("Successfully saved GameConfig: %s | Entries: %d", filePath, static_cast<int>(config.storage.size()));
    }

    GameConfig GameConfig::LoadFromFile(const char* filePath, const uint64_t key)
    {
        GameConfig config;
        FILE* file = fopen(filePath, "rb");
        if (file == nullptr)
        {
            LOG_WARNING("Config does not exist. Will be created on shutdown: %s", filePath);
            return config;
        }
        auto constexpr headerSize = static_cast<int>(std::char_traits<char>::length(FILE_HEADER));
        setvbuf(file, nullptr, _IONBF, 0);

        fseek(file, 0, SEEK_END);
        const int totalSize = static_cast<int>(ftell(file)) - headerSize;
        fseek(file, 0, SEEK_SET);

        // Check header first
        char buffer[headerSize];
        fread(buffer, headerSize, 1, file);
        if (memcmp(FILE_HEADER, buffer, headerSize) != 0)
        {
            LOG_ERROR("Malformed gameconfig file: %s", filePath);
            fclose(file);
            return config;
        }

        auto* data = static_cast<char*>(malloc(totalSize));
        fread(data, totalSize, 1, file);

        // Decrypt
        SymmetricEncrypt(data, totalSize, key);

        int i = 0;
        while (i < totalSize)
        {
            uint8_t type = 0;
            std::memcpy(&type, &data[i], sizeof(uint8_t));
            ++i;

            int storageID = 0;
            std::memcpy(&storageID, &data[i], sizeof(int));
            i += 4;

            internal::GameConfigStorageCell cell{static_cast<ConfigID>(storageID)};

            if (type == static_cast<uint8_t>(StorageType::STRING))
            {
                const auto len = static_cast<int>(strlen(&data[i])); // thats funny
                cell.assign(&data[i], len, static_cast<StorageType>(type));
                i += len;
            }
            else
            {
                cell.assign(&data[i], 8, static_cast<StorageType>(type));
                i += 8;
            }
            config.storage.push_back(cell);
        }

        fclose(file);
        free(data);
        LOG_INFO("Successfully loaded GameConfig: %s | Entries: %d", filePath, static_cast<int>(config.storage.size()));
        config.loaded = true;
        return config;
    }

    internal::GameConfigStorageCell* GameConfig::getCell(const ConfigID id)
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

    void GameConfig::saveKeybind(const ConfigID id, const Keybind keybind)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            internal::GameConfigStorageCell newCell{id};
            newCell.assign(nullptr, 0, StorageType::KEY_BIND, keybind);
            storage.push_back(newCell);
        }
        else
        {
            cell->assign(nullptr, 0, StorageType::KEY_BIND, keybind);
        }
    }

    void GameConfig::saveString(const ConfigID id, const std::string& string)
    {
        auto* cell = getCell(id);
        if (cell == nullptr)
        {
            internal::GameConfigStorageCell newCell{id};
            newCell.assign(string.data(), static_cast<int>(string.size()), StorageType::STRING);
            storage.push_back(newCell);
        }
        else
        {
            cell->assign(string.data(), static_cast<int>(string.size()), StorageType::STRING);
        }
    }

    Keybind GameConfig::getKeybindOrElse(const ConfigID id, const Keybind defaultKeybind)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return defaultKeybind;
        }
        if (cell->type != StorageType::KEY_BIND)
        {
            return defaultKeybind;
        }
        return cell->keybind;
    }

    std::string GameConfig::getStringOrElse(const ConfigID id, const std::string& defaultKey)
    {
        const auto* cell = getCell(id);
        if (cell == nullptr)
        {
            return defaultKey;
        }
        if (cell->type != StorageType::STRING)
        {
            return defaultKey;
        }
        return {cell->string}; // is null terminated
    }

    void GameConfig::erase(const ConfigID id)
    {
        if (storage.empty())
            return;

        if (storage.size() == 1)
        {
            storage.front().assign(nullptr, 0, StorageType::KEY_BIND); // If was a string delete it
            storage.clear();
            return;
        }

        for (auto& cell : storage)
        {
            if (cell.id == id)
            {
                cell.assign(nullptr, 0, StorageType::KEY_BIND); // If was a string delete it
                cell = storage.back();
                storage.pop_back();
                return;
            }
        }
    }

    void GameConfig::clear()
    {
        for (auto& cell : storage)
        {
            if (cell.type == StorageType::STRING)
            {
                cell.assign(nullptr, 0, StorageType::KEY_BIND); // If was a string delete it
            }
        }
        storage.clear();
    }

} // namespace magique